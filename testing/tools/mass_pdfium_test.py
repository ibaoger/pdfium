#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Compares the performance of two versions of the pdfium code."""

import argparse
import functools
import json
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
import tempfile

from common import GetBooleanGnArg
from common import PrintErr
from common import RunCommandPropagateErr


PDFIUM_TEST = 'pdfium_test'


def RunSingleTestCaseParallel(this, build_dir, test_case):
  result = this.RunSingleTestCase(build_dir, test_case)
  return (test_case, result)


class MassRun(object):
  """A comparison between two branches of pdfium."""

  def __init__(self, args):
    self.args = args
    self._InitPaths()

  def _InitPaths(self):
    input_file_re = re.compile('^.+[.]pdf$')
    self.test_cases = []
    for input_path in self.args.input_paths:
      if os.path.isfile(input_path):
        self.test_cases.append(input_path)
      elif os.path.isdir(input_path):
        for file_dir, _, filename_list in os.walk(input_path):
          for input_filename in filename_list:
            if input_file_re.match(input_filename):
              file_path = os.path.join(file_dir, input_filename)
              if os.path.isfile(file_path):
                self.test_cases.append(file_path)

  def Run(self):
    """Runs comparison by checking out branches, building and measuring them.

    Returns:
      Exit code for the script.
    """
    self._BuildCurrentBranch(self.args.build_dir)
    after = self._MeasureCurrentBranch(self.args.build_dir)
    acc = 0
    for k, v in after.iteritems():
      acc += v
      print '%3d %s' % (v, k)
    print 'Cases: %d' % len(after)
    print 'Average: %f' % (acc / float(len(after)))

    return 0

  def _BuildCurrentBranch(self, build_dir):
    """Synchronizes and builds the current version of pdfium.

    Args:
      build_dir: String with path to build directory
    """
    # PrintErr('Syncing...')
    # RunCommandPropagateErr(['gclient', 'sync'], exit_status_on_error=1)
    # PrintErr('Done.')

    PrintErr('Building...')
    cmd = ['ninja', '-C', build_dir, 'pdfium_test']
    if GetBooleanGnArg('use_goma', build_dir):
      cmd.extend(['-j', '250'])
    RunCommandPropagateErr(cmd, stdout_has_errors=True, exit_status_on_error=1)
    PrintErr('Done.')

  def _MeasureCurrentBranch(self, build_dir):
    PrintErr('Measuring...')
    if self.args.num_workers > 1 and len(self.test_cases) > 1:
      results = self._RunAsync(build_dir)
    else:
      results = self._RunSync(build_dir)
    PrintErr('Done.')

    return results

  def _RunSync(self, build_dir):
    """Profiles the test cases synchronously.

    Args:
      build_dir: String with path to build directory

    Returns:
      A dict mapping each test case name to the the profiling values for that
      test case.
    """
    results = {}

    for test_case in self.test_cases:
      result = self.RunSingleTestCase(build_dir, test_case)
      if result is not None:
        results[test_case] = result

    return results

  def _RunAsync(self, build_dir):
    """Profiles the test cases asynchronously.

    Uses as many workers as configured by --num-workers.

    Args:
      build_dir: String with path to build directory

    Returns:
      A dict mapping each test case name to the the profiling values for that
      test case.
    """
    results = {}
    pool = multiprocessing.Pool(self.args.num_workers)
    worker_func = functools.partial(
        RunSingleTestCaseParallel, self, build_dir)

    try:
      # The timeout is a workaround for http://bugs.python.org/issue8296
      # which prevents KeyboardInterrupt from working.
      one_year_in_seconds = 3600 * 24 * 365
      worker_results = (pool.map_async(worker_func, self.test_cases)
                        .get(one_year_in_seconds))
      for worker_result in worker_results:
        test_case, result = worker_result
        if result is not None:
          results[test_case] = result
    except KeyboardInterrupt:
      pool.terminate()
      sys.exit(1)
    else:
      pool.close()

    pool.join()

    return results

  def RunSingleTestCase(self, build_dir, test_case):
    """Profiles a single test case.

    Args:
      build_dir: String with path to build directory
      test_case: Path to the test case.

    Returns:
      The measured profiling value for that test case.
    """
    print '    %s' % test_case

    pdfium_test_path = os.path.join(build_dir, PDFIUM_TEST)
    command = [pdfium_test_path, test_case]

    output = RunCommandPropagateErr(command)

    if output is None:
      return None

    # Get the time number as output, making sure it's just a number
    output = output.strip()

    if re.match('^[0-9]+$', output):
      return int(output)

    return None


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('input_paths', nargs='+',
                      help='pdf files or directories to search for pdf files '
                           'to run as test cases')
  parser.add_argument('--build-dir', default=os.path.join('out', 'Release'),
                      help='relative path from the base source directory '
                           'to the build directory')
  parser.add_argument('--cache-dir', default=None,
                      help='directory with a new or preexisting cache for '
                           'downloads. Default is to not use a cache.')
  parser.add_argument('--num-workers', default=multiprocessing.cpu_count(),
                      type=int, help='run NUM_WORKERS jobs in parallel')

  args = parser.parse_args()

  # Always start at the pdfium src dir, which is assumed to be two level above
  # this script.
  pdfium_src_dir = os.path.join(
      os.path.dirname(__file__),
      os.path.pardir,
      os.path.pardir)
  os.chdir(pdfium_src_dir)

  run = MassRun(args)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
