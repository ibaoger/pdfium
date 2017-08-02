import functools
import glob
import multiprocessing
import os
import re
import subprocess
import webbrowser

from common import DirectoryFinder


def GenerateOneDiffParallel(image_comparison, image):
  return image_comparison.GenerateOneDiff(image)


class ImageComparison(object):

  def __init__(self, build_dir, test_cases, output_path, two_labels,
               num_workers, threshold_fraction):
    self.build_dir = build_dir
    self.test_cases = test_cases
    self.output_path = output_path
    self.two_labels = two_labels
    self.num_workers = num_workers
    self.threshold = threshold_fraction * 100
    print self.threshold

  def Run(self):
    print 'ImageComparison.Run()'

    if len(self.two_labels) != 2:
      print 'two_labels must be a tuple of length 2'
      return 1

    finder = DirectoryFinder(self.build_dir)
    self.img_diff_bin = finder.ExecutablePath('pdfium_diff')

    html_path = os.path.join(self.output_path, 'compare.html')

    self.diff_path = os.path.join(self.output_path, 'diff')
    if not os.path.exists(self.diff_path):
      os.makedirs(self.diff_path)

    # pdf_filenames = [os.path.split(tc)[1] for tc in self.test_cases]

    self.image_locations = ImageLocations(self.output_path, self.diff_path, self.two_labels)

    difference = self._GenerateDiffs()

    small_changes = []

    with open(html_path, 'w') as f:
      f.write('<html><body>')
      f.write('<table>')
      for image in self.image_locations.Images():
        diff = difference[image]
        if diff is None:
          print 'Failed to compare image %s' % image
        elif diff > self.threshold:
          self._WriteImageRows(f, image, diff)
        else:
          small_changes.append((image, diff))
      self._WriteSmallChanges(f, small_changes)
      f.write('</table>')
      f.write('</body></html>')

    webbrowser.open(html_path)
    return 0

  def _WriteImageRows(self, f, image, diff):
    f.write('<tr><td colspan="2">')
    f.write('%s (%.4f%% changed)' % (image, diff))
    f.write('</td></tr>')

    f.write('<tr>')
    self._WritePageCompareTd(f,
        self._GetRelativePath(self.image_locations.Left(image)),
        self._GetRelativePath(self.image_locations.Right(image)))
    self._WritePageTd(f,
        self._GetRelativePath(self.image_locations.Diff(image)))
    f.write('</tr>')

  def _GenerateDiffs(self):
    difference = {}
    pool = multiprocessing.Pool(self.num_workers)
    worker_func = functools.partial(GenerateOneDiffParallel, self)

    try:
      # The timeout is a workaround for http://bugs.python.org/issue8296
      # which prevents KeyboardInterrupt from working.
      one_year_in_seconds = 3600 * 24 * 365
      worker_results = (pool.map_async(worker_func,
                                       self.image_locations.Images())
                        .get(one_year_in_seconds))
      for worker_result in worker_results:
        image, result = worker_result
        difference[image] = result
    except KeyboardInterrupt:
      pool.terminate()
      sys.exit(1)
    else:
      pool.close()

    pool.join()

    return difference

  def GenerateOneDiff(self, image):
    try:
      diff_output = subprocess.check_output(
          [self.img_diff_bin,
           self.image_locations.Left(image),
           self.image_locations.Right(image)])
    except subprocess.CalledProcessError as e:
      percentage_change = float(re.findall(r'\d+\.\d+', e.output)[0])
    else:
      # print diff_output
      return image, 0

    try:
      diff_output = subprocess.check_output(
          [self.img_diff_bin,
           '--diff',
           self.image_locations.Left(image),
           self.image_locations.Right(image),
           self.image_locations.Diff(image)])
    except subprocess.CalledProcessError as e:
      return image, percentage_change
    else:
      print 'Warning: Should have failed the previous diff.'
      return image, 0

  def _GetRelativePath(self, absolute_path):
    return os.path.relpath(absolute_path, start=self.output_path)

  def _WritePageTd(self, f, image_path):
    f.write('<td>')
    # f.write('<img src="%s" style="width: 33vw;">' % image_path)
    f.write('<img src="%s">' % image_path)
    f.write('</td>')

  def _WritePageCompareTd(self, f, normal_image_path, hover_image_path):
    f.write('<td>')
    f.write('<img src="%s" '
            'onmouseover="this.src=\'%s\';" '
            'onmouseout="this.src=\'%s\';">' % (normal_image_path,
                                                hover_image_path,
                                                normal_image_path))
    f.write('</td>')


  def _WriteSmallChanges(self, f, small_changes):
    for image, change in small_changes:
      f.write('<tr><td colspan="2">')
      if not change:
        f.write('No change for: %s' % image)
      else:
        f.write('Small change of %.4f%% for: %s' % (change, image))
      f.write('</td></tr>')


class ImageLocations(object):

  def __init__(self, output_path, diff_path, two_labels):
    self.output_path = output_path
    self.diff_path = diff_path
    self.two_labels = two_labels

    self.left = self._FindImages(self.two_labels[0])
    self.right = self._FindImages(self.two_labels[1])

    self.images = list(self.left.viewkeys() & self.right.viewkeys())

    # Sort by pdf filename, then page number
    def key_fn(s):
      pieces = s.rsplit('.', 2)
      return (pieces[0], int(pieces[1]))

    self.images.sort(key=key_fn)
    self.diff = {image: os.path.join(self.diff_path, image)
                 for image in self.images}

  def _FindImages(self, label):
    image_path_matcher = os.path.join(self.output_path, label, '*.*.png')
    image_paths = glob.glob(image_path_matcher)

    image_dict = {os.path.split(image_path)[1]: image_path
                  for image_path in image_paths}

    return image_dict

  def Images(self):
    return self.images

  def Left(self, test_case):
    return self.left[test_case]

  def Right(self, test_case):
    return self.right[test_case]

  def Diff(self, test_case):
    return self.diff[test_case]
