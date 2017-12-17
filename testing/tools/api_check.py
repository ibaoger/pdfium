#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import sys

def IsValidFunctionName(function, filename):
  if function.startswith('FPDF'):
    return True
  if function == 'FSDK_SetUnSpObjProcessHandler' and filename == 'fpdf_ext.h':
    return True
  if function.startswith('FORM_') and filename == 'fpdf_formfill.h':
    return True
  return False


def FindFunction(function_snippet, filename):
  function_split = function_snippet.split('(')
  assert len(function_split) == 2
  function = function_split[0]
  assert IsValidFunctionName(function, filename)
  return function


def GetExportsFromHeader(dirname, filename):
  contents = open(os.path.join(dirname, filename)).readlines()
  look_for_function_name = False
  functions = []
  for line in contents:
    if look_for_function_name:
      look_for_function_name = False
      split_line = line.rstrip().split(' ')
      functions.append(FindFunction(split_line[0], filename))
      continue

    if not line.startswith('FPDF_EXPORT '):
      continue

    # Format should be: FPDF_EXPORT return_type FPDF_CALLCONV
    split_line = line.rstrip().split(' ')
    callconv_index = split_line.index('FPDF_CALLCONV')
    assert callconv_index >= 2
    if callconv_index + 1 == len(split_line):
      look_for_function_name = True
      continue

    functions.append(FindFunction(split_line[callconv_index + 1], filename))
  return functions


def GetFunctionsFromPublicHeaders(src_path):
  public_path = os.path.join(src_path, 'public')
  functions = []
  for filename in os.listdir(public_path):
    if filename.endswith('.h'):
      functions.extend(GetExportsFromHeader(public_path, filename))
  return functions


def GetFunctionsFromTest(api_test_path):
  chk_regex = re.compile('^    CHK\((.*)\);\n$')
  contents = open(api_test_path).readlines()
  functions = []
  for line in contents:
    match = chk_regex.match(line)
    if match:
      functions.append(match.groups()[0])
  return functions


def FindDuplicates(functions):
  return set([f for f in functions if functions.count(f) > 1])


def CheckAndPrintFailure(failure_list, failure_message):
  if not failure_list:
    return True

  print '%s:' % failure_message
  for f in sorted(failure_list):
    print f
  return False


def main():
  script_abspath = os.path.abspath(__file__)
  src_path = os.path.dirname(os.path.dirname(os.path.dirname(script_abspath)))
  public_functions = GetFunctionsFromPublicHeaders(src_path)

  api_test_path = os.path.join(src_path, 'fpdfsdk', 'fpdfview_c_api_test.c')
  test_functions = GetFunctionsFromTest(api_test_path)

  result = True
  duplicate_public_functions = FindDuplicates(public_functions)
  check = CheckAndPrintFailure(duplicate_public_functions,
                               'Found duplicate functions in public headers')
  result = result and check

  duplicate_test_functions = FindDuplicates(test_functions)
  check = CheckAndPrintFailure(duplicate_test_functions,
                               'Found duplicate functions in API test')
  result = result and check

  public_functions_set = set(public_functions)
  test_functions_set = set(test_functions)
  not_tested = public_functions_set.difference(test_functions_set)
  check = CheckAndPrintFailure(not_tested, 'Functions not tested')
  result = result and check
  non_existent = test_functions_set.difference(public_functions_set)
  check = CheckAndPrintFailure(non_existent, 'Tested functions do not exist')
  result = result and check

  return 0 if result else 1


if __name__ == '__main__':
  sys.exit(main())
