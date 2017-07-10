#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys


CALLGRIND_PROFILER = 'callgrind'
PERFSTAT_PROFILER = 'perfstat'


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
    if self.options.profiler == CALLGRIND_PROFILER:
      time = self._RunCallgrind()
    elif self.options.profiler == PERFSTAT_PROFILER:
      time = self._RunPerfStat()
    else:
      print 'profiler=%s not supported, aborting' % self.options.profiler
      return 1

    if time is None:
      return 1

    print time
    return 0

  def _RunCallgrind(self):
    # Whether to turn instrument the whole run or to use the callgrind macro
    # delimiters in pdfium_test.
    instrument_at_start = ('yes' if self.options.measure_full else 'no')
    cmd_to_run = ['valgrind', '--tool=callgrind',
                  '--instr-atstart=%s' % instrument_at_start,
                  '--callgrind-out-file=/dev/null', # Don't output callgrind.out
                  self.pdfium_test_path, '--send-events', self.pdf_path]
    output = subprocess.check_output(cmd_to_run, stderr=subprocess.STDOUT)

    # Match the line with the instruction count, eg.
    # '==98765== Collected : 12345'
    return self._ExtractIrCount('\\bCollected\\b.*\\b(\\d+)', output)

  def _RunPerfStat(self):
    cmd_to_run = ['perf', 'stat',
                  '--no-big-num', # Do not add thousands separators
                  '-einstructions', # Print only instruction count
                  self.pdfium_test_path, '--send-events', self.pdf_path]
    output = subprocess.check_output(cmd_to_run, stderr=subprocess.STDOUT)

    # Match the line with the instruction count, eg.
    # '        12345      instructions'
    return self._ExtractIrCount('\\b(\\d+)\\b.*\\binstructions\\b', output)

  def _ExtractIrCount(self, regex, output):
    matcher = re.compile(regex)
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
  parser.add_option('--profiler', default=CALLGRIND_PROFILER,
                    help='what profiler to use. Supports callgrind and '
                         'perfstat for now.')
  parser.add_option('--measure-full', default=False,
                    help='whether to measure the whole test harness or '
                         'just the interesting section')
  options, pdf_path = parser.parse_args()

  if len(pdf_path) != 1:
    print 'Exactly one test case must be specified'
    return 1

  run = PerformanceRun(options, pdf_path[0])
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
