#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys

class PerformanceRun(object):
  def __init__(self, options, pdf_path):
    self.options = options
    self.pdf_path = pdf_path
    self._InitPaths()

  def _InitPaths(self):
    self.pdfium_test_path =  os.path.join(self.options.build_dir, 'pdfium_test')
    if not os.path.exists(self.pdfium_test_path):
      print "FAILURE: Can't find test executable '%s'" % self.pdfium_test_path
      print 'Use --build-dir to specify its location.'
      return 1

  def Run(self):
    cmd_to_run = ['valgrind', '--tool=callgrind', '--instr-atstart=no',
                  '--callgrind-out-file=/dev/null',
                  self.pdfium_test_path, '--send-events', self.pdf_path]
    output = subprocess.check_output(cmd_to_run, stderr=subprocess.STDOUT)
    ir_count = self._ExtractIrCount(output)
    if ir_count is None:
      return 1

    print ir_count
    return 0

  def _ExtractIrCount(self, output):
    # Match the line with the instruction count, eg.
    # '==98765== Collected : 12345'
    matcher = re.compile('\\bCollected\\b.*\\b(\\d+)')
    for output_line in output.split('\n'):
      matched = matcher.search(output_line)
      if matched:
        # Group 1 is the instruction number, eg. 12345
        return int(matched.group(1))
    return None


def main():
  parser = optparse.OptionParser()
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  options, pdf_path = parser.parse_args()

  if len(pdf_path) != 1:
    print 'Exactly one test case must be specified'
    return 1

  run = PerformanceRun(options, pdf_path[0])
  return run.Run()

if __name__ == '__main__':
  sys.exit(main())
