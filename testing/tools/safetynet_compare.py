#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Compares the performance of two versions of the pdfium code.

--machine-readable outputs a machine readable version of the conclusions in
the json format:
{
  "params": {
    "threshold": 0.02
  },
  "summary": {
    "number_cases": 123,
    "number_failed_to_measure": 1,
    "number_regressions": 2,
    "number_improvements": 1
  },
  "comparison_by_case": {
    "testing/resources/new_test.pdf": [null, 1000, null, "failure"],
    "testing/resources/test1.pdf": [100, 120, 0.2, "regression"],
    "testing/resources/test2.pdf": [100, 2000, 19.0, "regression"],
    "testing/resources/test3.pdf": [1000, 1005, 0.005, "small_change"],
    "testing/resources/test4.pdf": [1000, 1000, 0.0, "no_change"],
    "testing/resources/test5.pdf": [1000, 600, -0.4, "improvement"]
  },
}
"""

import functools
import json
import multiprocessing
import optparse
import os
import re
import subprocess
import sys
from collections import Counter

from githelper import GitHelper


FORMAT_RED = '\033[01;31m{0}\033[00m'
FORMAT_GREEN = '\033[01;32m{0}\033[00m'
FORMAT_BLUE = '\033[01;36m{0}\033[00m'
FORMAT_NORMAL = '{0}'

RATING_FAILURE = 'failure'
RATING_REGRESSION = 'regression'
RATING_IMPROVEMENT = 'improvement'
RATING_NO_CHANGE = 'no_change'
RATING_SMALL_CHANGE = 'small_change'

RATINGS = [
  RATING_FAILURE,
  RATING_REGRESSION,
  RATING_IMPROVEMENT,
  RATING_NO_CHANGE,
  RATING_SMALL_CHANGE
]

RATING_TO_COLOR = {
  RATING_FAILURE: FORMAT_RED,
  RATING_REGRESSION: FORMAT_RED,
  RATING_IMPROVEMENT: FORMAT_BLUE,
  RATING_NO_CHANGE: FORMAT_GREEN,
  RATING_SMALL_CHANGE: FORMAT_NORMAL,
}


def printerr(s):
  print >> sys.stderr, s


def RunSingleTestCaseParallel(this, run_label, test_case):
  result = this._RunSingleTestCase(run_label, test_case);
  return (test_case, result)


class CompareRun(object):
  def __init__(self, options, input_paths):
    self.git = GitHelper()
    self.options = options
    self._InitPaths(input_paths)

  def _InitPaths(self,input_paths):
    self.safe_measure_script_path = os.path.join(self.options.build_dir,
                                                'safetynet_measure_current.py')

    input_file_re = re.compile('^.+[.]pdf$')
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
    self._FreezeMeasureScript()

    if self.options.branch_after:
      before, after = \
          self._ProfileTwoOtherBranches(self.options.branch_before,
                                        self.options.branch_after)
    elif self.options.branch_before:
      before, after = \
          self._ProfileCurrentAndOtherBranch(self.options.branch_before)
    else:
      before, after = self._ProfileLocalChangesAndCurrentBranch()

    conclusions = self._DrawConclusions(before, after)
    if self.options.machine_readable:
      self._PrintConclusionsMachine(conclusions)
    else:
      self._PrintConclusionsHuman(conclusions)

    self._CleanUp(conclusions)

  def _FreezeMeasureScript(self):
    subprocess.check_output(['cp', 'testing/tools/safetynet_measure.py',
                             self.safe_measure_script_path])

  def _ProfileTwoOtherBranches(self, before_branch, after_branch):
    branch_to_restore = self.git.GetCurrentBranchName()

    self._StashLocalChanges();

    self._CheckoutBranch(after_branch)
    self._BuildCurrentBranch()
    after = self._MeasureCurrentBranch('after')

    self._CheckoutBranch(before_branch)
    self._BuildCurrentBranch()
    before = self._MeasureCurrentBranch('before')

    self._CheckoutBranch(branch_to_restore)
    self._RestoreLocalChanges()

    return before, after

  def _ProfileCurrentAndOtherBranch(self, other_branch):
    branch_to_restore = self.git.GetCurrentBranchName()

    self._BuildCurrentBranch()
    after = self._MeasureCurrentBranch('after')

    self._StashLocalChanges();

    self._CheckoutBranch(other_branch)
    self._BuildCurrentBranch()
    before = self._MeasureCurrentBranch('before')

    self._CheckoutBranch(branch_to_restore)
    self._RestoreLocalChanges()

    return before, after

  def _ProfileLocalChangesAndCurrentBranch(self):
    self._BuildCurrentBranch()
    after = self._MeasureCurrentBranch('after')

    pushed = self._StashLocalChanges();
    if not pushed:
      printerr(FORMAT_RED.format('Warning: No local changes to compare'))

    self._BuildCurrentBranch()
    before = self._MeasureCurrentBranch('before')

    self._RestoreLocalChanges()

    return before, after

  def _CheckoutBranch(self, branch):
    printerr("Checking out branch '%s'" % branch)
    self.git.Checkout(branch)

  def _StashLocalChanges(self):
    printerr('Stashing local changes...')
    return self.git.StashPush()
    printerr('Done.')

  def _RestoreLocalChanges(self):
    printerr('Restoring local changes...')
    self.git.StashPopAll()
    printerr('Done.')

  def _BuildCurrentBranch(self):
    printerr('Building...')
    subprocess.check_output(['ninja', '-C', self.options.build_dir,
                             'pdfium_test'])
    printerr('Done.')

  def _MeasureCurrentBranch(self, run_label):
    printerr('Measuring...')
    if self.options.num_workers > 1 and len(self.test_cases) > 1:
      results = self._RunAsync(run_label)
    else:
      results = self._RunSync(run_label)
    printerr('Done.')

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

    if self.options.interesting_section:
      command.append('--interesting-section')

    if self.options.profiler:
      command.append('--profiler=%s' % self.options.profiler)

    profile_file_path = self._GetProfileFilePath(run_label, test_case)
    if profile_file_path:
      command.append('--output-path=%s' % profile_file_path)

    output = subprocess.check_output(command)

    # Get the time number as output, making sure it's just a number
    output = output.strip()
    if re.match('^[0-9]+$', output):
      return int(output)
    else:
      return None

  def _GetProfileFilePath(self, run_label, test_case):
    if self.options.output_dir:
      output_filename = ('callgrind.out.%s.%s'
                         % (test_case.replace('/', '_'),
                            run_label))
      return os.path.join(self.options.output_dir, output_filename)
    else:
      return None

  def _DrawConclusions(self, times_before_branch, times_after_branch):
    conclusions = ComparisonResults(self.options.threshold_significant)

    for test_case in sorted(self.test_cases):
      before = times_before_branch.get(test_case)
      after = times_after_branch.get(test_case)
      conclusions.ProcessCase(test_case, before, after)

    return conclusions

  def _PrintConclusionsMachine(self, conclusions):
    output_dict = conclusions.GetOutputDict()
    json_output = json.dumps(output_dict)
    print json_output

  def _PrintConclusionsHuman(self, conclusions):
    # Print header
    print '=' * 80
    print '{0:>11s} {1:>15s}  {2}' .format(
        '% Change',
        'Time after',
        'Test case')
    print '-' * 80

    for _, case_result in sorted(conclusions.GetCaseResults().iteritems()):
      if case_result.rating == RATING_FAILURE:
        print ('Failed to measure time for %s' % test_case)
        continue

      color = RATING_TO_COLOR[case_result.rating]

      print '{0} {1:15,d}  {2}' .format(
          color.format('{:+11.4%}'.format(case_result.ratio)),
          case_result.after,
          case_result.case_name)

    totals = conclusions.GetSummary()
    print '=' * 80
    print 'Test cases run: %d' % totals.GetTotal()
    color = FORMAT_RED if totals.GetCount(RATING_FAILURE) else FORMAT_GREEN
    print ('Failed to measure: %s'
           % color.format(totals.GetCount(RATING_FAILURE)))
    color = FORMAT_RED if totals.GetCount(RATING_REGRESSION) else FORMAT_GREEN
    print ('Regressions: %s'
           % color.format(totals.GetCount(RATING_REGRESSION)))
    color = FORMAT_BLUE if totals.GetCount(RATING_IMPROVEMENT) else FORMAT_GREEN
    print ('Improvements: %s'
           % color.format(totals.GetCount(RATING_IMPROVEMENT)))

  def _CleanUp(self, conclusions):
    if self.options.profiler != 'callgrind':
      return

    for case_result in conclusions.GetCaseResults().values():
      if case_result.rating not in [RATING_REGRESSION, RATING_IMPROVEMENT]:
        os.remove(self._GetProfileFilePath('before', case_result.case_name))
        os.remove(self._GetProfileFilePath('after', case_result.case_name))


class ComparisonResults(object):

  def __init__(self, threshold_significant):
    self.threshold_significant = threshold_significant
    # This makes the threshold a factor rather than a linear amount.
    # threshold_significant 0.1 -> 90.9% to 110% is not significant
    # threshold_significant 0.25 -> 80% to 125% is not significant
    # threshold_significant 1 -> 50% to 200% is not significant
    # threshold_significant 4 -> 20% to 500% is not significant
    self.threshold_significant_negative = \
        (1 / (1 + threshold_significant)) - 1

    self.params = {'threshold': threshold_significant}
    self.summary = ComparisonSummary()
    self.case_results = {}

  def ProcessCase(self, case_name, before, after):
    # Switch 0 to None to simplify processing
    if not before:
      before = None
    if not after:
      after = None

    if not before or not after:
      ratio = None
      rating = RATING_FAILURE
    else:
      ratio = (float(after) / before) - 1.0
      if ratio > self.threshold_significant:
        rating = RATING_REGRESSION
      elif ratio > 0:
        rating = RATING_SMALL_CHANGE
      elif ratio < self.threshold_significant_negative:
        rating = RATING_IMPROVEMENT
      elif ratio < 0:
        rating = RATING_SMALL_CHANGE
      else:
        rating = RATING_NO_CHANGE

    case_result = CaseResult(case_name, before, after, ratio, rating)

    self.summary.ProcessCaseResult(case_result)
    self.case_results[case_name] = case_result

  def GetSummary(self):
    return self.summary

  def GetCaseResults(self):
    return self.case_results

  def GetOutputDict(self):
    output_dict = {}
    output_dict['params'] = {'threshold': self.threshold_significant}
    output_dict['summary'] = self.summary.GetOutputDict()
    output_dict['comparison_by_case'] = \
        {cr.case_name: cr.GetOutputTuple()
         for cr in self.GetCaseResults().values()}
    return output_dict


class ComparisonSummary(object):
  def __init__(self):
    self.rating_counter = Counter()

  def ProcessCaseResult(self, case_result):
    self.rating_counter[case_result.rating] += 1

  def GetTotal(self):
    return sum(self.rating_counter.values())

  def GetCount(self, rating):
    return self.rating_counter[rating]

  def GetOutputDict(self):
    result = {'total': self.GetTotal()}
    for rating in RATINGS:
      result[rating] = self.GetCount(rating)
    return result


class CaseResult(object):
  def __init__(self, case_name, before, after, ratio, rating):
    self.case_name = case_name
    self.before = before
    self.after = after
    self.ratio = ratio
    self.rating = rating

  def GetOutputTuple(self):
    return (self.before, self.after, self.ratio, self.rating)


def main():
  parser = optparse.OptionParser()
  parser.add_option('--branch-before',
                    help='git branch to use as "before" for comparison. '
                         'Omitting this will use the current branch '
                         'without uncommitted changes as the baseline.')
  parser.add_option('--branch-after',
                    help='git branch to use as "after" for comparison. '
                         'Omitting this will use the current branch '
                         'with uncommitted changes.')
  parser.add_option('--build-dir', default=os.path.join('out', 'Release'),
                    help='relative path from the base source directory')
  parser.add_option('--profiler', default='callgrind',
                    help='what profiler to use, if not the default. Supports '
                         'callgrind and perfstat for now.')
  parser.add_option('--interesting-section', action='store_true',
                    help='whether to measure just the interesting section or '
                         'the whole test harness. Limiting to only the '
                         'interesting section does not work on Release since '
                         'the delimiters are optimized out')
  parser.add_option('--num-workers', default=multiprocessing.cpu_count(),
                    type='int', help='run NUM_WORKERS jobs in parallel')
  parser.add_option('--output-dir',
                    help='directory to write the profile data output files')
  parser.add_option('--threshold-significant', default=0.02, type='float',
                    help='variations in performance above this factor are '
                         'considered significant')
  parser.add_option('--machine-readable', action='store_true',
                    help='whether to get output for machines. Specs are in '
                         'this script\'s module docstring. Default is '
                         'human-readable.')
  options, input_paths = parser.parse_args()

  if len(input_paths) < 1:
    printerr('At least one test case must be specified')
    return 1

  git = GitHelper()

  if options.branch_after and not options.branch_before:
      printerr('--branch-after requires --branch-before to be specified.')
      return 1

  if options.branch_after and not git.BranchExists(options.branch_after):
      printerr('Branch "%s" does not exist' % options.branch_after)
      return 1

  if options.branch_before and not git.BranchExists(options.branch_before):
      printerr('Branch "%s" does not exist' % options.branch_before)
      return 1

  if options.output_dir:
    options.output_dir = os.path.expanduser(options.output_dir)
    if not os.path.isdir(options.output_dir):
      printerr('"%s" is not a directory' % options.output_dir)
      return 1

  if options.threshold_significant <= 0.0:
    printerr('--threshold-significant should receive a positive float')
    return 1

  run = CompareRun(options, input_paths)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
