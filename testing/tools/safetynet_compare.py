#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import subprocess
import sys


FORMAT_RED = '\033[01;31m{0}\033[00m'
FORMAT_GREEN = '\033[01;32m{0}\033[00m'
FORMAT_BLUE = '\033[01;36m{0}\033[00m'
FORMAT_NORMAL = '{0}'


class CompareRun(object):
  def __init__(self, options, pdf_paths):
    self.options = options
    self.pdf_paths = pdf_paths
    self._InitPaths()

  def _InitPaths(self):
    self.safe_measure_script_path = os.path.join(self.options.build_dir,
                                                'safetynet_measure_current.py')

  def Run(self):
    current_branch_name = subprocess.check_output(
        ['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    subprocess.check_call(['cp', 'testing/tools/safetynet_measure.py',
                           self.safe_measure_script_path])

    # Measure branch #1
    times_current_branch = self._BuildAndMeasureCurrentBranch()

    # Checkout branch #2
    output = subprocess.check_output(['git', 'stash'])
    do_stash_pop = ('No local changes to save' not in output)
    subprocess.check_call(['git', 'checkout', self.options.branch_to_compare])

    # Measure branch #2
    times_other_branch = self._BuildAndMeasureCurrentBranch()

    # Checkout branch #1 again and restore changes
    subprocess.check_call(['git', 'checkout', current_branch_name])
    if do_stash_pop:
      subprocess.check_call(['git', 'stash', 'pop'])

    self._PrintConclusions(times_current_branch, times_other_branch)

    return 0

  def _BuildAndMeasureCurrentBranch(self):
    subprocess.check_call(['ninja', '-C', self.options.build_dir,
                           'pdfium_test'])

    results = {}

    for pdf_path in self.pdf_paths:
      command = [self.safe_measure_script_path, pdf_path,
                '--build-dir=%s' % self.options.build_dir]
      if self.options.measure_full:
        command.append('--measure-full=yes')
      if self.options.profiler:
        command.append('--profiler=%s' % self.options.profiler)

      output = subprocess.check_output(command)

      # Get the time number as output, making sure it's just a number
      output = output.strip()
      if re.match('^[0-9]+$', output):
        results[pdf_path] = int(output)

    return results

  def _PrintConclusions(self, times_current_branch, times_other_branch):
    for pdf_path in self.pdf_paths:
      time_current_branch = times_current_branch.get(pdf_path)
      time_other_branch = times_other_branch.get(pdf_path)
      if not time_current_branch:
        print ('Failed to measure time in current branch %s for %s'
               % (current_branch_name, pdf_path))
        continue
      if not time_other_branch:
        print ('Failed to measure time in branch-to-compare %s for %s'
               % (self.options.branch_to_compare, pdf_path))
        continue

      ratio = (float(time_current_branch) / time_other_branch) - 1.0

      if ratio > 0.02:
        color = FORMAT_RED
      elif ratio > 0:
        color = FORMAT_NORMAL
      elif ratio < -0.02:
        color = FORMAT_BLUE
      elif ratio < 0:
        color = FORMAT_NORMAL
      else:
        color = FORMAT_GREEN
      print 'Change: %s for %s' % (color.format('{:+.4%}'.format(ratio)),
                                   os.path.basename(pdf_path))


def main():
  parser = optparse.OptionParser()
  parser.add_option('--branch-to-compare',
                    help='git branch to use as baseline for comparison')
  parser.add_option('--build-dir', default=os.path.join('out', 'Debug'),
                    help='relative path from the base source directory')
  parser.add_option('--profiler',
                    help='what profiler to use, if not the default. Supports '
                         'callgrind and perfstat for now.')
  parser.add_option('--measure-full', default=False,
                    help='whether to measure the whole test harness or '
                         'just the interesting section')
  options, pdf_paths = parser.parse_args()

  if len(pdf_paths) < 1:
    print 'At least one test case must be specified'
    return 1

  if not options.branch_to_compare:
    print 'Specify the git branch to compare (name or hash)'
    return 1

  try:
    subprocess.check_call(['git', 'rev-parse', '--verify',
                           options.branch_to_compare])
  except subprocess.CalledProcessError:
    print 'Branch %s does not exist' % options.branch_to_compare
    return 1

  run = CompareRun(options, pdf_paths)
  return run.Run()


if __name__ == '__main__':
  sys.exit(main())
