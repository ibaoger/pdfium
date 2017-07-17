#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Measures performance for rendering a single test case with pdfium.

The output is a number that is a metric which depends on the profiler specified.
"""

import argparse
import os
import re
import subprocess
import sys


CALLGRIND_PROFILER = 'callgrind'
PERFSTAT_PROFILER = 'perfstat'


class PerformanceRun(object):
  """A single measurement of a test case."""

  def __init__(self, args):
    self.args = args
    self.pdfium_test_path = os.path.join(self.args.build_dir, 'pdfium_test')

  def _CheckTools(self):
    """Returns whether the tool file paths are sane."""
    if not os.path.exists(self.pdfium_test_path):
      print "FAILURE: Can't find test executable '%s'" % self.pdfium_test_path
      print 'Use --build-dir to specify its location.'
      return False
    if not os.access(self.pdfium_test_path, os.X_OK):
      print ("FAILURE: Test executable '%s' lacks execution permissions"
             % self.pdfium_test_path)
      return False
    return True

  def Run(self):
    """Runs test harness and measures performance with the given profiler.

    Returns:
      Exit code for the script.
    """
    if not self._CheckTools():
      return 1

    if self.args.profiler == CALLGRIND_PROFILER:
      time = self._RunCallgrind()
    elif self.args.profiler == PERFSTAT_PROFILER:
      time = self._RunPerfStat()
    else:
      print 'profiler=%s not supported, aborting' % self.args.profiler
      return 1

    if time is None:
      return 1

    print time
    return 0

  def _RunCallgrind(self):
    """Runs test harness and measures performance with callgrind.

    Returns:
      int with the result of the measurement, in instructions or time.
    """
    # Whether to turn instrument the whole run or to use the callgrind macro
    # delimiters in pdfium_test.
    instrument_at_start = ('no' if self.args.interesting_section else 'yes')
    output_path = self.args.output_path or '/dev/null'

    valgrind_cmd = (['valgrind', '--tool=callgrind',
                     '--instr-atstart=%s' % instrument_at_start,
                     '--callgrind-out-file=%s' % output_path]
                    + self._BuildTestHarnessCommand())
    output = subprocess.check_output(valgrind_cmd, stderr=subprocess.STDOUT)

    # Match the line with the instruction count, eg.
    # '==98765== Collected : 12345'
    return self._ExtractIrCount(r'\bCollected\b *: *\b(\d+)', output)

  def _RunPerfStat(self):
    """Runs test harness and measures performance with perf stat.

    Returns:
      int with the result of the measurement, in instructions or time.
    """
    # --no-big-num: do not add thousands separators
    # -einstructions: print only instruction count
    cmd_to_run = (['perf', 'stat', '--no-big-num', '-einstructions']
                  + self._BuildTestHarnessCommand())
    output = subprocess.check_output(cmd_to_run, stderr=subprocess.STDOUT)

    # Match the line with the instruction count, eg.
    # '        12345      instructions'
    return self._ExtractIrCount(r'\b(\d+)\b.*\binstructions\b', output)

  def _BuildTestHarnessCommand(self):
    """Builds command to run the test harness."""
    cmd = [self.pdfium_test_path, '--send-events']
    if self.args.interesting_section:
      cmd.append('--callgrind-delim')
    cmd.append(self.args.pdf_path)
    return cmd

  def _ExtractIrCount(self, regex, output):
    """Extracts a number from the output with a regex."""
    matched = re.search(regex, output)

    if not matched:
      return None

    # Group 1 is the instruction number, eg. 12345
    return int(matched.group(1))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('pdf_path',
                      help='test case to measure load and rendering time')
  parser.add_argument('--build-dir', default=os.path.join('out', 'Release'),
                      help='relative path from the base source directory')
  parser.add_argument('--profiler', default=CALLGRIND_PROFILER,
                      help='which profiler to use. Supports callgrind and '
                           'perfstat for now.')
  parser.add_argument('--interesting-section', action='store_true',
                      help='whether to measure just the interesting section or '
                           'the whole test harness. The interesting section is '
                           'pdfium reading a pdf from memory and rendering '
                           'it, which omits loading the time to load the file, '
                           'initialize the library, terminate it, etc. '
                           'Limiting to only the interesting section does not '
                           'work on Release since the delimiters are optimized '
                           'out')
  parser.add_argument('--output-path',
                      help='where to write the profile data output file')
  args = parser.parse_args()

  run = PerformanceRun(args)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
