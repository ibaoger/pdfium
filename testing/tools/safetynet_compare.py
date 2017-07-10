#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import functools
import multiprocessing
import optparse
import os
import re
import subprocess
import sys


FORMAT_RED = '\033[01;31m{0}\033[00m'
FORMAT_GREEN = '\033[01;32m{0}\033[00m'
FORMAT_BLUE = '\033[01;36m{0}\033[00m'
FORMAT_NORMAL = '{0}'


def RunSingleTestCaseParallel(this, run_label, test_case):
  result = this._RunSingleTestCase(run_label, test_case);
  return (test_case, result)


class CompareRun(object):
  def __init__(self, options, input_paths):
    self.options = options
    self._InitPaths(input_paths)

  def _InitPaths(self,input_paths):
    self.safe_measure_script_path = os.path.join(self.options.build_dir,
                                                'safetynet_measure_current.py')

    input_file_re = re.compile('^.+[.](in|pdf)$')
    self.test_cases = []
    for input_path in input_paths:
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
    if self.options.branch_to_compare:
      current_branch_name = subprocess.check_output(
          ['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()

    subprocess.check_call(['cp', 'testing/tools/safetynet_measure.py',
                           self.safe_measure_script_path])

    # Measure branch #1
    self._BuildCurrentBranch()
    times_current_branch = self._MeasureCurrentBranch('current')

    # Checkout branch #2
    output = subprocess.check_output(['git', 'stash', '--include-untracked'])
    do_stash_pop = ('No local changes to save' not in output)

    if not do_stash_pop and not self.options.branch_to_compare:
      print FORMAT_RED.format('No changes are being compared')

    if self.options.branch_to_compare:
      subprocess.check_call(['git', 'checkout', self.options.branch_to_compare])

    # Measure branch #2
    self._BuildCurrentBranch()
    times_other_branch = self._MeasureCurrentBranch('other')

    # Checkout branch #1 again and restore changes
    if self.options.branch_to_compare:
      subprocess.check_call(['git', 'checkout', current_branch_name])
    if do_stash_pop:
      subprocess.check_call(['git', 'stash', 'pop'])

    self._PrintConclusions(times_current_branch, times_other_branch)

    return 0

  def _BuildCurrentBranch(self):
    subprocess.check_call(['ninja', '-C', self.options.build_dir,
                           'pdfium_test'])

  def _MeasureCurrentBranch(self, run_label):
    if self.options.num_workers > 1 and len(self.test_cases) > 1:
      results = self._RunAsync(run_label)
    else:
      results = self._RunSync(run_label)

    return results

  def _RunSync(self, run_label):
    results = {}

    for test_case in self.test_cases:
      result = self._RunSingleTestCase(run_label, test_case)
      if result is not None:
        results[test_case] = result

    return results

  def _RunAsync(self, run_label):
    results = {}
    pool = multiprocessing.Pool(self.options.num_workers)
    worker_func = functools.partial(RunSingleTestCaseParallel, self, run_label)

    worker_results = pool.imap(worker_func, self.test_cases)
    for worker_result in worker_results:
      test_case, result = worker_result
      if result is not None:
        results[test_case] = result

    pool.close()
    pool.join()

    return results

  def _RunSingleTestCase(self, run_label, test_case):
    command = [self.safe_measure_script_path, test_case,
              '--build-dir=%s' % self.options.build_dir]
    if self.options.measure_full:
      command.append('--measure-full')
    if self.options.profiler:
      command.append('--profiler=%s' % self.options.profiler)
    if self.options.output_dir:
      output_filename = ('callgrind.out.%s.%s'
                         % (test_case.replace('/', '_'),
                            run_label))
      output_path = os.path.join(self.options.output_dir, output_filename)
      command.append('--output-path=%s' % output_path)

    output = subprocess.check_output(command)

    # Get the time number as output, making sure it's just a number
    output = output.strip()
    if re.match('^[0-9]+$', output):
      return int(output)
    else:
      return None

  def _PrintConclusions(self, times_current_branch, times_other_branch):
    count_failed_to_measure = 0
    count_significant_regressions = 0
    count_significant_improvements = 0

    for test_case in sorted(self.test_cases):
      time_current_branch = times_current_branch.get(test_case)
      time_other_branch = times_other_branch.get(test_case)
      if not time_current_branch:
        print ('Failed to measure time in current branch for %s'
               % test_case)
        count_failed_to_measure += 1
        continue
      if not time_other_branch:
        print ('Failed to measure time in branch-to-compare for %s'
               % test_case)
        count_failed_to_measure += 1
        continue

      ratio = (float(time_current_branch) / time_other_branch) - 1.0

      if ratio > 0.02:
        color = FORMAT_RED
        count_significant_regressions += 1
      elif ratio > 0:
        color = FORMAT_NORMAL
      elif ratio < -0.02:
        color = FORMAT_BLUE
        count_significant_improvements += 1
      elif ratio < 0:
        color = FORMAT_NORMAL
      else:
        color = FORMAT_GREEN

      print 'Change: {0} {1:15,d} - {2}' .format(
          color.format('{:+11.4%}'.format(ratio)),
          time_current_branch,
          test_case)

    print '=' * 80
    print 'Test cases run: %d' % len(self.test_cases)
    color = FORMAT_RED if count_failed_to_measure else FORMAT_GREEN
    print 'Failed to measure: %s' % color.format(count_failed_to_measure)
    color = FORMAT_RED if count_significant_regressions else FORMAT_GREEN
    print 'Regressions: %s' % color.format(count_significant_regressions)
    color = FORMAT_BLUE if count_significant_improvements else FORMAT_GREEN
    print 'Improvements: %s' % color.format(count_significant_improvements)


def main():
  parser = optparse.OptionParser()
  parser.add_option('--branch-to-compare',
                    help='git branch to use as baseline for comparison. '
                         'Omitting this will try comparing the current branch '
                         'with and without uncommitted changes.')
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  parser.add_option('--profiler',
                    help='what profiler to use, if not the default. Supports '
                         'callgrind and perfstat for now.')
  parser.add_option('--measure-full', action='store_true',
                    help='whether to measure the whole test harness or '
                         'just the interesting section')
  parser.add_option('--num-workers', default=multiprocessing.cpu_count(),
                    type='int', help='run NUM_WORKERS jobs in parallel')
  parser.add_option('--output-dir',
                    help='directory to write the profile data output files')
  options, input_paths = parser.parse_args()

  if len(input_paths) < 1:
    print 'At least one test case must be specified'
    return 1

  if options.branch_to_compare:
    try:
      subprocess.check_call(['git', 'rev-parse', '--verify',
                             options.branch_to_compare])
    except subprocess.CalledProcessError:
      print 'Branch "%s" does not exist' % options.branch_to_compare
      return 1

  options.output_dir = os.path.expanduser(options.output_dir)
  if options.output_dir:
    if not os.path.isdir(options.output_dir):
      print '"%s" is not a directory' % options.output_dir
      return 1

  run = CompareRun(options, input_paths)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
