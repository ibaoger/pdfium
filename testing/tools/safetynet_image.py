import glob
import os
import subprocess
import webbrowser

from common import DirectoryFinder


class ImageComparison(object):

  def __init__(self, build_dir, test_cases, output_path, two_labels):
    self.build_dir = build_dir
    self.test_cases = test_cases
    self.output_path = output_path
    self.two_labels = two_labels

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

    with open(html_path, 'w') as f:
      f.write('<html><body>')
      f.write('<table>')
      for test_case in self.test_cases:
        self._WriteImageRows(f, test_case)
      self._WriteOmittedIdentical(f)
      f.write('</table>')
      f.write('</body></html>')

    webbrowser.open(html_path)
    return 0

  def _WriteImageRows(self, f, test_case):
    _, pdf_filename = os.path.split(test_case)

    f.write('<tr><td colspan="3">')
    f.write('%s (%s)' % (test_case, pdf_filename))
    f.write('</td></tr>')

    image_paths_per_label = [self._FindImages(label, pdf_filename)
                             for label in self.two_labels]

    for image_left, image_right in zip(*image_paths_per_label):
      output_image = os.path.join(self.diff_path, os.path.split(image_left)[1])
      try:
        diff_output = subprocess.check_output(
            [self.img_diff_bin, '--diff', image_left, image_right,
             output_image])
        different = False
        # print diff_output
      except subprocess.CalledProcessError as e:
        different = True
        # print e

      if different:
        f.write('<tr>')
        self._WritePageTd(f, image_left)
        self._WritePageTd(f, image_right)
        self._WritePageTd(f, output_image)
        f.write('</tr>')

  def _WritePageTd(self, f, image_path):
    f.write('<td>')
    f.write('<img src="%s">' % image_path)
    f.write('</td>')

  def _FindImages(self, label, pdf_filename):
    image_path_matcher = os.path.join(
        self.output_path, label, '%s.*.png' % pdf_filename)
    image_paths = glob.glob(image_path_matcher)
    # Sort by page number
    image_paths.sort(key=lambda s: int(s.split('.')[-2]))
    return image_paths

  def _WriteOmittedIdentical(self, f):
    pass
