#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess

class GitHelper(object):
  """Issues git commands."""

  def __init__(self):
    self.stashed = 0

  def Checkout(self, branch):
    subprocess.check_output(['git', 'checkout', branch])

  def Pull(self):
    subprocess.check_output(['git', 'pull'])

  def StashPush(self):
    output = subprocess.check_output(['git', 'stash', '--include-untracked'])
    if 'No local changes to save' in output:
      return False

    self.stashed += 1
    return True

  def StashPopAll(self):
    while self.stashed > 0:
      subprocess.check_output(['git', 'stash', 'pop'])
      self.stashed -= 1

  def GetCurrentBranchName(self):
    return subprocess.check_output(
        ['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()

  def GetCurrentBranchHash(self):
    return subprocess.check_output(
        ['git', 'rev-parse', 'HEAD']).strip()

  def IsCurrentBranchClean(self):
    output = subprocess.check_output(['git', 'status', '--porcelain'])
    return not output

  def BranchExists(self, branch_name):
    try:
      subprocess.check_output(['git', 'rev-parse', '--verify',
                               branch_name])
      return True
    except subprocess.CalledProcessError:
      return False
