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
import subprocess
import sys

from githelper import GitHelper
from safetynet_conclusions import ComparisonConclusions
from safetynet_conclusions import PrintConclusionsDictHumanReadable
from safetynet_conclusions import RATING_IMPROVEMENT
from safetynet_conclusions import RATING_REGRESSION


def PrintErr(s):
  """Prints s to stderr."""
  print >> sys.stderr, s


def RunSingleTestCaseParallel(this, run_label, test_case):
  result = this.RunSingleTestCase(run_label, test_case)
  return (test_case, result)


class CompareRun(object):
  """A comparison between two branches of pdfium."""

  def __init__(self, args):
    self.git = GitHelper()
    self.args = args
    self._InitPaths()

  def _InitPaths(self):
    self.safe_measure_script_path = os.path.join(self.args.build_dir,
                                                 'safetynet_measure_current.py')

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
    self._FreezeMeasureScript()

    if self.args.branch_after:
      before, after = self._ProfileTwoOtherBranches(
          self.args.branch_before,
          self.args.branch_after)

    elif self.args.branch_before:
      before, after = self._ProfileCurrentAndOtherBranch(
          self.args.branch_before)

    else:
      before, after = self._ProfileLocalChangesAndCurrentBranch()

    conclusions = self._DrawConclusions(before, after)
    conclusions_dict = conclusions.GetOutputDict()

    if self.args.machine_readable:
      print json.dumps(conclusions_dict)
    else:
      PrintConclusionsDictHumanReadable(conclusions_dict, colored=True)

    self._CleanUp(conclusions)

    return 0

  def _FreezeMeasureScript(self):
    """Freezes a version of the measuring script.

    This is needed to make sure we are comparing the pdfium library changes and
    not script changes that may happen between the two branches.
    """
    subprocess.check_output(['cp', 'testing/tools/safetynet_measure.py',
                             self.safe_measure_script_path])

  def _ProfileTwoOtherBranches(self, before_branch, after_branch):
    """Profiles two branches that are not the current branch.

    after_branch does not need to descend from before_branch, they will be
    measured the same way

    Args:
      before_branch: One branch to profile.
      after_branch: Other branch to profile.

    Returns:
      A tuple (before, after), where each of before and after is a dict
      mapping a test case name to the the profiling values for that test case
      in the given branch.
    """
    branch_to_restore = self.git.GetCurrentBranchName()

    self._StashLocalChanges()

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
    """Profiles the current branch (with uncommitted changes) and another one.

    The current branch does not need to descend from other_branch.

    Args:
      other_branch: Other branch to profile that is not the current.

    Returns:
      A tuple (before, after), where each of before and after is a dict
      mapping a test case name to the the profiling values for that test case
      in the given branch. The current branch is considered to be "after" and
      the other branch is considered to be "before".
    """
    branch_to_restore = self.git.GetCurrentBranchName()

    self._BuildCurrentBranch()
    after = self._MeasureCurrentBranch('after')

    self._StashLocalChanges()

    self._CheckoutBranch(other_branch)
    self._BuildCurrentBranch()
    before = self._MeasureCurrentBranch('before')

    self._CheckoutBranch(branch_to_restore)
    self._RestoreLocalChanges()

    return before, after

  def _ProfileLocalChangesAndCurrentBranch(self):
    """Profiles the current branch with and without uncommitted changes.

    Returns:
      A tuple (before, after), where each of before and after is a dict
      mapping a test case name to the the profiling values for that test case
      using the given version. The current branch without uncommitted changes is
      considered to be "before" and with uncommitted changes is considered to be
      "after".
    """
    self._BuildCurrentBranch()
    after = self._MeasureCurrentBranch('after')

    pushed = self._StashLocalChanges()
    if not pushed:
      PrintErr('Warning: No local changes to compare')

    self._BuildCurrentBranch()
    before = self._MeasureCurrentBranch('before')

    self._RestoreLocalChanges()

    return before, after

  def _CheckoutBranch(self, branch):
    PrintErr("Checking out branch '%s'" % branch)
    self.git.Checkout(branch)

  def _StashLocalChanges(self):
    PrintErr('Stashing local changes')
    return self.git.StashPush()

  def _RestoreLocalChanges(self):
    PrintErr('Restoring local changes')
    self.git.StashPopAll()

  def _BuildCurrentBranch(self):
    PrintErr('Building...')
    subprocess.check_output(['ninja', '-C', self.args.build_dir,
                             'pdfium_test'])
    PrintErr('Done.')

  def _MeasureCurrentBranch(self, run_label):
    PrintErr('Measuring...')
    if self.args.num_workers > 1 and len(self.test_cases) > 1:
      results = self._RunAsync(run_label)
    else:
      results = self._RunSync(run_label)
    PrintErr('Done.')

    return results

  def _RunSync(self, run_label):
    """Profiles the test cases synchronously.

    Args:
      run_label: String to differentiate this version of the code in output
          files from other versions.

    Returns:
      A dict mapping each test case name to the the profiling values for that
      test case.
    """
    results = {}

    for test_case in self.test_cases:
      result = self.RunSingleTestCase(run_label, test_case)
      if result is not None:
        results[test_case] = result

    return results

  def _RunAsync(self, run_label):
    """Profiles the test cases asynchronously.

    Uses as many workers as configured by --num-workers.

    Args:
      run_label: String to differentiate this version of the code in output
          files from other versions.

    Returns:
      A dict mapping each test case name to the the profiling values for that
      test case.
    """
    results = {}
    pool = multiprocessing.Pool(self.args.num_workers)
    worker_func = functools.partial(RunSingleTestCaseParallel, self, run_label)

    worker_results = pool.imap(worker_func, self.test_cases)
    for worker_result in worker_results:
      test_case, result = worker_result
      if result is not None:
        results[test_case] = result

    pool.close()
    pool.join()

    return results

  def RunSingleTestCase(self, run_label, test_case):
    """Profiles a single test case.

    Args:
      run_label: String to differentiate this version of the code in output
          files from other versions.
      test_case: Path to the test case.

    Returns:
      The measured profiling value for that test case.
    """
    command = [self.safe_measure_script_path, test_case,
               '--build-dir=%s' % self.args.build_dir]

    if self.args.interesting_section:
      command.append('--interesting-section')

    if self.args.profiler:
      command.append('--profiler=%s' % self.args.profiler)

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
    if self.args.output_dir:
      output_filename = ('callgrind.out.%s.%s'
                         % (test_case.replace('/', '_'),
                            run_label))
      return os.path.join(self.args.output_dir, output_filename)
    else:
      return None

  def _DrawConclusions(self, times_before_branch, times_after_branch):
    """Draws conclusions comparing results of test runs in two branches.

    Args:
      times_before_branch: A dict mapping each test case name to the the
          profiling values for that test case in the branch to be considered
          as the baseline.
      times_after_branch: A dict mapping each test case name to the the
          profiling values for that test case in the branch to be considered
          as the new version.

    Returns:
      ComparisonConclusions with all test cases processed.
    """
    conclusions = ComparisonConclusions(self.args.threshold_significant)

    for test_case in sorted(self.test_cases):
      before = times_before_branch.get(test_case)
      after = times_after_branch.get(test_case)
      conclusions.ProcessCase(test_case, before, after)

    return conclusions

  def _CleanUp(self, conclusions):
    """Removes profile output files for uninteresting cases.

    Cases without significant regressions or improvements and considered
    uninteresting.

    Args:
      conclusions: A ComparisonConclusions,
    """
    if self.args.profiler != 'callgrind':
      return

    for case_result in conclusions.GetCaseResults().values():
      if case_result.rating not in [RATING_REGRESSION, RATING_IMPROVEMENT]:
        os.remove(self._GetProfileFilePath('before', case_result.case_name))
        os.remove(self._GetProfileFilePath('after', case_result.case_name))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('input_paths', nargs='+',
                      help='pdf files or directories to search for pdf files '
                           'to run as test cases')
  parser.add_argument('--branch-before',
                      help='git branch to use as "before" for comparison. '
                           'Omitting this will use the current branch '
                           'without uncommitted changes as the baseline.')
  parser.add_argument('--branch-after',
                      help='git branch to use as "after" for comparison. '
                           'Omitting this will use the current branch '
                           'with uncommitted changes.')
  parser.add_argument('--build-dir', default=os.path.join('out', 'Release'),
                      help='relative path from the base source directory')
  parser.add_argument('--profiler', default='callgrind',
                      help='what profiler to use, if not the default. Supports '
                           'callgrind and perfstat for now.')
  parser.add_argument('--interesting-section', action='store_true',
                      help='whether to measure just the interesting section or '
                           'the whole test harness. Limiting to only the '
                           'interesting section does not work on Release since '
                           'the delimiters are optimized out')
  parser.add_argument('--num-workers', default=multiprocessing.cpu_count(),
                      type=int, help='run NUM_WORKERS jobs in parallel')
  parser.add_argument('--output-dir',
                      help='directory to write the profile data output files')
  parser.add_argument('--threshold-significant', default=0.02, type=float,
                      help='variations in performance above this factor are '
                           'considered significant')
  parser.add_argument('--machine-readable', action='store_true',
                      help='whether to get output for machines. If enabled the '
                           'output will be a json with the format specified in '
                           'ComparisonConclusions.GetOutputDict(). Default is '
                           'human-readable.')
  args = parser.parse_args()

  git = GitHelper()

  if args.branch_after and not args.branch_before:
    PrintErr('--branch-after requires --branch-before to be specified.')
    return 1

  if args.branch_after and not git.BranchExists(args.branch_after):
    PrintErr('Branch "%s" does not exist' % args.branch_after)
    return 1

  if args.branch_before and not git.BranchExists(args.branch_before):
    PrintErr('Branch "%s" does not exist' % args.branch_before)
    return 1

  if args.output_dir:
    args.output_dir = os.path.expanduser(args.output_dir)
    if not os.path.isdir(args.output_dir):
      PrintErr('"%s" is not a directory' % args.output_dir)
      return 1

  if args.threshold_significant <= 0.0:
    PrintErr('--threshold-significant should receive a positive float')
    return 1

  run = CompareRun(args)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
