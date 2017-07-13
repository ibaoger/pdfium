#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import datetime
import json
import os
import subprocess
import sys

from githelper import GitHelper
from safetynet_conclusions import PrintConclusionsDictHumanReadable


class JobContext(object):

  def __init__(self, args):
    self.run_name = datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')
    self.results_dir = args.results_dir
    self.last_revision_covered_file = os.path.join(self.results_dir,
                                                   'last_revision_covered')
    self.run_output_dir = os.path.join(self.results_dir,
                                       'profiles_%s' % self.run_name)
    self.run_output_log_file = os.path.join(self.results_dir,
                                           '%s.log' % self.run_name)


class JobRun(object):

  def __init__(self, args, context):
    self.git = GitHelper()
    self.args = args
    self.context = context

  def Run(self):
    if not self.args.testing and not self.git.IsCurrentBranchClean():
      print 'Current branch is not clean, aborting'
      return 1

    if not self.args.testing:
      self.git.Pull()

    # Make sure results dir exists
    if not os.path.exists(self.context.results_dir):
      os.makedirs(self.context.results_dir)

    if not os.path.exists(self.context.last_revision_covered_file):
      return self._InitialRun()
    else:
      with open(self.context.last_revision_covered_file) as f:
        last_revision_covered = f.read().strip()
      return self._IncrementalRun(last_revision_covered)

  def _InitialRun(self):
    current = self.git.GetCurrentBranchHash()

    print 'Initial run, current is %s' % current

    with open(self.context.last_revision_covered_file, 'w') as f:
      f.write(current + '\n')

    print 'All set up, next runs will be incremental and perform comparisons'
    return 0

  def _IncrementalRun(self, last_revision_covered):
    current = self.git.GetCurrentBranchHash()

    print ('Incremental run, current is %s, last is %s'
           % (current, last_revision_covered))

    if current == last_revision_covered:
      print 'No changes seen, finishing job'
      return 0

    # Run compare
    if not os.path.exists(self.context.run_output_dir):
      os.makedirs(self.context.run_output_dir)
    cmd = ['testing/tools/safetynet_compare.py',
           'testing/corpus/fx/shading',
           '--machine-readable',
           '--branch-before=%s' % last_revision_covered,
           '--output-dir=%s' % self.context.run_output_dir]
    json_output = subprocess.check_output(cmd, stderr=sys.stderr)

    output_info = json.loads(json_output)
    PrintConclusionsDictHumanReadable(output_info)
    if (output_info['summary']['improvement']
        or output_info['summary']['regression']):
      print 'Something detected.'
    else:
      print 'Nothing detected.'

    return 0


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('results_dir',
                      help='where to write the job results')
  parser.add_argument('--testing', action='store_true',
                      help='whether to skip the pull. Use '
                           'for script debugging only')
  parser.add_argument('--output-to-log', action='store_true',
                      help='whether to write output to a log file')
  args = parser.parse_args()

  job_context = JobContext(args)

  if args.output_to_log:
    log_file = open(job_context.run_output_log_file, 'w')
    sys.stdout = log_file
    sys.stderr = log_file

  run = JobRun(args, job_context)
  result = run.Run()

  if args.output_to_log:
    log_file.close()

  return result


if __name__ == '__main__':
  sys.exit(main())
