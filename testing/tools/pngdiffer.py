#!/usr/bin/env python
# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import sys

import common

class PNGDiffer():
  def __init__(self, finder):
    self.pdfium_diff_path = finder.ExecutablePath('pdfium_diff')
    self.os_name = finder.os_name

  def GetActualFiles(self, input_filename, source_dir, working_dir):
    actual_paths = []
    paths = TemplatePaths(input_filename, source_dir, working_dir)

    i = 0
    while True:
      actual_path = paths.GetActualPath(i)
      expected_path = paths.GetExpectedPath(i)
      platform_expected_path = paths.GetPlatformExpectedPath(self.os_name, i)
      if os.path.exists(platform_expected_path):
        expected_path = platform_expected_path
      elif not os.path.exists(expected_path):
        break
      actual_paths.append(actual_path)
      i += 1
    return actual_paths

  def HasDifferences(self, input_filename, source_dir, working_dir):
    paths = TemplatePaths(input_filename, source_dir, working_dir)

    i = 0
    while True:
      actual_path = paths.GetActualPath(i)
      expected_path = paths.GetExpectedPath(i)
      # PDFium tests should be platform independent. Platform based results are
      # used to capture platform dependent implementations.
      platform_expected_path = paths.GetPlatformExpectedPath(self.os_name, i)
      if (not os.path.exists(expected_path) and
          not os.path.exists(platform_expected_path)):
        if i == 0:
          print "WARNING: no expected results files for " + input_filename
        break
      print "Checking " + actual_path
      sys.stdout.flush()
      if os.path.exists(expected_path):
        error = common.RunCommand(
            [self.pdfium_diff_path, expected_path, actual_path])
      else:
        error = 1;
      if error:
        # When failed, we check against platform based results.
        if os.path.exists(platform_expected_path):
          error = common.RunCommand(
              [self.pdfium_diff_path, platform_expected_path, actual_path])
        if error:
          print "FAILURE: " + input_filename + "; " + str(error)
          return True
      i += 1
    return False

  def Regenerate(self, input_filename, source_dir, working_dir, platform_only):
    paths = TemplatePaths(input_filename, source_dir, working_dir)

    page = 0
    while True:
      # Loop through the generated page images. Stop when there is a page
      # missing a png, which means the document ended.
      actual_path = paths.GetActualPath(page)
      if not os.path.isfile(actual_path):
        break

      platform_expected_path = paths.GetPlatformExpectedPath(self.os_name, page)

      # If there is a platform expected png, we will overwrite it. Otherwise,
      # overwrite the generic png in "all" mode, or do nothing in "platform"
      # mode.
      if os.path.exists(platform_expected_path):
        expected_path = platform_expected_path
      elif not platform_only:
        expected_path = paths.GetExpectedPath(page)
      else:
        expected_path = None

      if expected_path is not None and os.path.exists(expected_path):
        shutil.copyfile(actual_path, expected_path)

      page += 1


ACTUAL_TEMPLATE = '.pdf.%d.png'
EXPECTED_TEMPLATE = '_expected' + ACTUAL_TEMPLATE
PLATFORM_EXPECTED_TEMPLATE = '_expected_%s' + ACTUAL_TEMPLATE


class TemplatePaths(object):

  def __init__(self, input_filename, source_dir, working_dir):
    input_root, _ = os.path.splitext(input_filename)
    self.actual_path_template = os.path.join(working_dir,
                                             input_root + ACTUAL_TEMPLATE)
    self.expected_path = os.path.join(
        source_dir, input_root + EXPECTED_TEMPLATE)
    self.platform_expected_path = os.path.join(
        source_dir, input_root + PLATFORM_EXPECTED_TEMPLATE)

  def GetActualPath(self, page):
    return self.actual_path_template % page

  def GetExpectedPath(self, page):
    return self.expected_path % page

  def GetPlatformExpectedPath(self, platform, page):
    return self.platform_expected_path % (platform, page)
