#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys

import common


FORMAT_RED = '\033[01;31m{0}\033[00m'
FORMAT_GREEN = '\033[01;32m{0}\033[00m'
FORMAT_BLUE = '\033[01;36m{0}\033[00m'


class CompareRun(object):
  def __init__(self, options, pdf_path):
    self.options = options
    self.pdf_path = pdf_path
    self._InitPaths()

  def _InitPaths(self):
    finder = common.DirectoryFinder(self.options.build_dir)

    self.pdfium_test_path = finder.ExecutablePath('pdfium_test')
    if not os.path.exists(self.pdfium_test_path):
      print "FAILURE: Can't find test executable '%s'" % self.pdfium_test_path
      print 'Use --build-dir to specify its location.'
      return 1

  def Run(self):
    subprocess.check_call(['cp', 'testing/tools/safetynet_measure.py', self.options.build_dir + '/safetynet_measure_current.py'])

    # Measure branch #1
    time_current_branch = self._BuildAndMeasureCurrentBranch()
    if time_current_branch is None:
      print 'Failed to measure time in current branch'
      return 1

    # Checkout branch #2
    output = subprocess.check_output(['git', 'stash'])
    do_stash_pop = ('No local changes to save' not in output)
    subprocess.check_call(['git', 'checkout', 'safetynet_harness3'])

    # Measure branch #2
    time_other_branch = self._BuildAndMeasureCurrentBranch()
    if time_other_branch is None:
      print 'Failed to measure time in other branch'
      return 1

    # Checkout branch #1 again
    subprocess.check_call(['git', 'checkout', 'safetynet_harness4'])
    if do_stash_pop:
      subprocess.check_call(['git', 'stash', 'pop'])

    # Compare and print
    ratio = (float(time_current_branch) / time_other_branch) - 1.0
    print 'Test case: %s' % os.path.basename(self.pdf_path)
    print 'Before: %d' % time_other_branch
    print 'After: %d' % time_current_branch
    if ratio > 0:
      color = FORMAT_RED
    elif ratio < 0:
      color = FORMAT_BLUE
    else:
      color = FORMAT_GREEN
    print 'Change: %s' % color.format('{:+.4%}'.format(ratio))

    return 0

  def _BuildAndMeasureCurrentBranch(self):
    subprocess.check_call(['ninja', '-C', 'out/Debug', 'pdfium_all'])
    output = subprocess.check_output([self.options.build_dir + '/safetynet_measure_current.py', self.pdf_path])
    output = output.strip()
    if re.match('^[0-9]+$', output):
      return int(output)
    else:
      return None


def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  options, pdf_path = parser.parse_args()

  if len(pdf_path) != 1:
    print 'Exactly one test case must be specified'
    return 1

  run = CompareRun(options, pdf_path[0])
  return run.Run()

if __name__ == '__main__':
  sys.exit(main())
