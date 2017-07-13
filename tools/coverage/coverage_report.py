#!/usr/bin/env python

# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generates a coverage report for given binaries using llvm-gcov & lcov.

Requires llvm-cov 3.5 or later.
Requires lcov 1.11 or later.
Requires that 'use_coverage = true' is set in args.gn.
"""

import argparse
import pprint
import os
import re
import subprocess
import sys


class CoverageExecutor(object):

  def __init__(self, parser, args):
    self.dry_run = args['dry_run']
    self.verbose = args['verbose']

    self.coverage_files = set()
    self.source_directory = args['source_directory']
    if not os.path.isdir(self.source_directory):
      parser.error("'{}' needs to be a directory".format(self.source_directory))

    self.build_directory = args['build_directory']
    if not os.path.isdir(self.build_directory):
      parser.error("'{}' needs to be a directory".format(self.build_directory))

    self.coverage_tests = self.calculate_coverage_tests(args)
    if not len(self.coverage_tests):
      parser.error(
          'No valid tests in set to be run. This is likely due to bad command '
          'line arguments'
      )

    if not self.boolean_gn_arg('use_coverage'):
      parser.error(
          "'use_coverage' does not appear to be set to true for build, but is "
          "needed"
      )

    self.use_goma = self.boolean_gn_arg('use_goma')

    self.output_directory = args['output_directory']
    if not os.path.exists(self.output_directory):
      if not self.dry_run:
        os.mkdir(self.output_directory)
    elif not os.path.isdir(self.output_directory):
      parser.error(
          "'{}' exists, but is not a directory".format(self.output_directory))
    self.coverage_totals_path = os.path.join(self.output_directory,
                                             'pdfium_totals.info')

    llvm_cov = self.determine_proper_llvm_cov()
    if not llvm_cov:
      print 'Unable to find appropriate llvm-cov to use'
      exit(1)
    self.lcov_env = os.environ
    self.lcov_env['LLVM_COV_BIN'] = llvm_cov

    self.lcov = self.determine_proper_lcov()
    if not self.lcov:
      print 'Unable to find appropriate lcov to use'
      exit(1)

  def boolean_gn_arg(self, arg):
    """Extract the value of a boolean flag in args.gn"""
    gn_args_output = self.checkout_output([
        'gn', 'args', self.build_directory, '--list={}'.format(arg), '--short'
    ])
    arg_match_output = re.match('{} = (.*)'.format(arg),
                                gn_args_output).group(1)
    if self.verbose:
      print "Found '{}' for value of {}".format(arg_match_output, arg)
    return arg_match_output == 'true'

  def checkout_output(
      self,
      args,
      dry_run=False,
      env=None,):
    """Wrapper for the check_output command that allows specifying if this is a dry run or not."""
    if dry_run:
      print "Would have run '{}'".format(' '.join(args))
      return ''

    if env:
      output = subprocess.check_output(args, env=env)
    else:
      output = subprocess.check_output(args)

    if self.verbose:
      print "check_output({}) returned '{}'".format(args, output)
    return output

  def call(self, args, dry_run=False, env=None):
    """Wrapper for the call command that allows specifying if this is a dry run or not."""
    if dry_run:
      print "Would have run '{}'".format(' '.join(args))
      return 0

    if env:
      output = subprocess.call(args, env=env)
    else:
      output = subprocess.call(args)

    if self.verbose:
      print "call({}) returned '{}'".format(args, output)
    return output

  def call_lcov(self, args, dry_run=False, needs_directory=True):
    """Wrapper to call lcov that adds appropriate arguments as needed."""
    lcov_args = [
        self.lcov, '--config-file',
        os.path.join(self.source_directory, 'tools', 'coverage', 'lcovrc'),
        '--gcov-tool',
        os.path.join(self.source_directory, 'tools', 'coverage', 'llvm-gcov')
    ]
    if needs_directory:
      lcov_args.extend(['--directory', self.source_directory])
    if not self.verbose:
      lcov_args.extend(['--quiet'])
    lcov_args.extend(args)
    return self.call(lcov_args, dry_run=dry_run, env=self.lcov_env)

  def calculate_coverage_tests(self, args):
    """Based on the arguments passed in, determine which tests are going to be run."""
    testing_tools_directory = os.path.join(self.source_directory, 'testing',
                                           'tools')
    # Values are tuples of the path to test binary and a boolean indicating if this binary uses test_runner.py
    coverage_tests = {
        'pdfium_unittests': (os.path.join(self.build_directory,
                                          'pdfium_unittests'), False),
        'pdfium_embeddertests': (os.path.join(self.build_directory,
                                              'pdfium_embeddertests'), False),
        'corpus_tests': (os.path.join(testing_tools_directory,
                                      'run_corpus_tests.py'), True),
        'javascript_tests': (os.path.join(testing_tools_directory,
                                          'run_javascript_tests.py'), True),
        'pixel_tests': (os.path.join(testing_tools_directory,
                                     'run_pixel_tests.py'), True),
    }

    tests_to_remove = []
    if len(args['tests']):
      for test_name in coverage_tests.keys():
        if test_name not in args['tests']:
          tests_to_remove.append(test_name)
    elif not args['slow']:
      slow_tests = ['corpus_tests', 'javascript_tests', 'pixel_tests']
      tests_to_remove.extend(slow_tests)

    for test_name in tests_to_remove:
      del coverage_tests[test_name]

    return coverage_tests

  def determine_proper_llvm_cov(self):
    """Find the newest version of llvm_cov that will work with the script."""
    llvm_cov_binary = None
    llvm_cov_major_version = 0
    llvm_cov_minor_version = 0
    # When calling Bash builtins like this the command and arguments must be passed in as single string instead of
    # as separate list members.
    potential_binaries = self.checkout_output(
        ['bash', '-c', 'compgen -abck llvm-cov']).splitlines()
    for binary in potential_binaries:
      if self.verbose:
        print 'Testing llvm-cov binary, {}'.format(binary)
      # Older versions of llvm-cov return 1 on success for the --version command, which causes Python to think
      # it exited with a failure status. Conveniently these versions also don't support the gcov command, so they
      # are not usable anyway.
      try:
        version_output = self.checkout_output([binary,
                                               '--version']).splitlines()
      except subprocess.CalledProcessError:
        if self.verbose:
          print ('--version return status 1, so this must be an older version '
                 'of llvm-cov, ignoring')
        continue

      # Technically the version is in x.y.z format, but only the x.y versions matter for determining
      # compatibility.
      parsed_major_version = 0
      parsed_minor_version = 0
      for line in version_output:
        matcher = re.match('.*LLVM version ([0-9]+)\.([0-9]+).*', line)
        if matcher:
          parsed_major_version = int(matcher.group(1))
          parsed_minor_version = int(matcher.group(2))
          break

      if self.verbose:
        print 'Found version to be {}.{}'.format(parsed_major_version,
                                                 parsed_minor_version)
      if parsed_major_version > llvm_cov_major_version or (
          parsed_major_version == llvm_cov_major_version and
          parsed_minor_version > llvm_cov_minor_version):
        if self.verbose:
          print 'Found new highest version number'
        llvm_cov_binary = binary
        llvm_cov_major_version = parsed_major_version
        llvm_cov_minor_version = parsed_minor_version

    # gcov command was implemented in 3.5, which is used for generating data for lcov
    if llvm_cov_major_version < 3 or (llvm_cov_major_version == 3 and
                                      llvm_cov_minor_version < 5):
      if self.verbose:
        print ('Highest version of llvm-cov discovered is {}.{}, which lower '
               'then the minimum 3.5').format(
            llvm_cov_major_version, llvm_cov_minor_version)
      llvm_cov_binary = None

    return llvm_cov_binary

  def determine_proper_lcov(self):
    """Find the newest version of lcov that will work with the script."""
    lcov_binary = None
    lcov_major_version = 0
    lcov_minor_version = 0
    # When calling Bash builtins like this the command and arguments must be passed in as single string instead of
    # as separate list members.
    potential_binaries = self.checkout_output(
        ['bash', '-c', 'compgen -abck lcov']).splitlines()
    for binary in potential_binaries:
      if self.verbose:
        print 'Testing lcov binary, {}'.format(binary)

      # Skipping binraries that fail on --version, since lcov should always succeed.
      try:
        version_output = self.checkout_output([binary,
                                               '--version']).splitlines()
      except subprocess.CalledProcessError:
        if self.verbose:
          print '--version return status 1, so ignoring'
        continue

      parsed_major_version = 0
      parsed_minor_version = 0
      for line in version_output:
        matcher = re.match('.*LCOV version ([0-9]+)\.([0-9]+).*', line)
        if matcher:
          parsed_major_version = int(matcher.group(1))
          parsed_minor_version = int(matcher.group(2))
          break

      if self.verbose:
        print 'Found version to be {}.{}'.format(parsed_major_version,
                                                 parsed_minor_version)
      if parsed_major_version > lcov_major_version or (
          parsed_major_version == lcov_major_version and
          parsed_minor_version > lcov_minor_version):
        if self.verbose:
          print 'Found new highest version number'
        lcov_binary = binary
        lcov_major_version = parsed_major_version
        lcov_minor_version = parsed_minor_version

    # lcov 1.10 and earlier pass in problematic flags to the gcov tool being used
    if lcov_major_version < 1 or (lcov_major_version == 1 and
                                  lcov_minor_version < 11):
      if self.verbose:
        print ('Highest version of lcov discovered is {}.{}, which lower then '
               'the minimum 1.11').format(
            lcov_major_version, lcov_minor_version)
      lcov_binary = None

    return lcov_binary

  def build_binaries(self):
    """Build all the binaries that are going to be needed for coverage generation."""
    call_args = ['ninja']
    if self.use_goma:
      call_args.extend(['-j', '1000'])
    call_args.extend(['-C', self.build_directory])
    return self.call(call_args, dry_run=self.dry_run) == 0

  def generate_coverage(self, name, test_spec):
    """Generate the coverage data for a test with the given name specifications."""
    if self.verbose:
      print "Generating coverage for test '{}', using data '{}'".format(
          name, test_spec)
    binary = test_spec[0]
    use_test_runner = test_spec[1]
    if not os.path.exists(binary):
      print ('Unable to generate coverage for {}, since it appears to not exist'
             ' @ {}').format(
          name, binary)
      return False

    if self.call_lcov(['--zerocounters'], dry_run=self.dry_run):
      print 'Unable to clear counters for {}'.format(name)
      return False

    binary_args = [binary]
    if use_test_runner:
      # Test runner tests multi-thread in the wrapper script, not the test binary, so need -j 1, otherwise
      # multiple processes will be witting to the code coverage files, invalidating results.
      # TODO(pdfium:811): Rewrite how test runner tests work, so that they can be run in multi-threaded mode.
      binary_args.extend(['-j', '1', '--build-dir', self.build_directory])
    if self.call(binary_args, dry_run=self.dry_run) and self.verbose:
      print 'Running {} appears to have failed, which might affect results'.format(
          binary)

    output_raw_path = os.path.join(self.output_directory,
                                   '{}_raw.info'.format(name))
    if self.call_lcov(
        ['--capture', '--test-name', binary, '--output-file', output_raw_path],
        dry_run=self.dry_run):
      print 'Unable to capture coverage data for {}'.format(name)
      return False

    output_filtered_path = os.path.join(self.output_directory,
                                        '{}_filtered.info'.format(name))
    output_filters = [
        '/usr/include/*', '*third_party*', '*testing*', '*_unittest.cpp',
        '*_embeddertest.cpp'
    ]
    if self.call_lcov(
        ['--remove', output_raw_path] + output_filters +
        ['--output-file', output_filtered_path],
        dry_run=self.dry_run,
        needs_directory=False):
      print 'Unable to filter coverage data for {}'.format(name)
      return False

    self.coverage_files.add(output_filtered_path)
    return True

  def merge_coverage(self):
    """Merge all of the coverage data sets into one for report generation."""
    merge_args = []
    for coverage_file in self.coverage_files:
      merge_args.extend(['--add-tracefile', coverage_file])

    merge_args.extend(['--output-file', self.coverage_totals_path])
    return self.call_lcov(
        merge_args, dry_run=self.dry_run, needs_directory=False) == 0

  def generate_report(self):
    """Produce HTML coverage report based on combined coverage data set"""
    return 0 == self.call(
        [
            'genhtml', '--config-file',
            os.path.join(self.source_directory, 'tools', 'coverage',
                         'lcovrc'), '--legend', '--demangle-cpp',
            '--show-details', '--prefix', self.source_directory,
            '--ignore-errors', 'source', self.coverage_totals_path,
            '--output-directory', self.output_directory
        ],
        dry_run=self.dry_run) == 0

  def run(self):
    """Setup environment, execute the tests and generate coverage report"""
    if not self.build_binaries():
      print 'Failed to successfully build binaries'
      return False

    for name in self.coverage_tests.keys():
      if not self.generate_coverage(name, self.coverage_tests[name]):
        print 'Failed to successfully generate coverage data'
        return False

    if not self.merge_coverage():
      print 'Failed to successfully merge generated coverage data'
      return False

    if not self.generate_report():
      print 'Failed to successfully generated coverage report'
      return False

    return True


def main():
  parser = argparse.ArgumentParser()
  parser.formatter_class = argparse.RawDescriptionHelpFormatter
  parser.description = ('Generates a coverage report for given binaries using '
                        'llvm-cov & lcov.\n\nRequires llvm-cov 3.5 or '
                        'later.\nRequires lcov 1.11 or later.\n\nBy default '
                        "runs 'pdfium_unittests' and 'pdfium_embeddertests'. If"
                        ' --slow is passed in then all tests will be run. If '
                        'any of the tests are specified on the command line, '
                        'then only those will be run.')
  parser.add_argument(
      '-s',
      '--source_directory',
      nargs=1,
      help='Location of PDFium source directory, defaults to CWD',
      default=os.getcwd())
  build_default = os.path.join('out', 'Coverage')
  parser.add_argument(
      '-b',
      '--build_directory',
      nargs=1,
      help=
      "Location of PDFium build directory with coverage enabled, defaults to '{}' under CWD".
      format(build_default),
      default=os.path.join(os.getcwd(), build_default))
  output_default = 'coverage_report'
  parser.add_argument(
      '-o',
      '--output_directory',
      nargs=1,
      help=
      "Location to write out coverage report to, defaults to '{}' under CWD ".
      format(output_default),
      default=os.path.join(os.getcwd(), output_default))
  parser.add_argument(
      '-n',
      '--dry-run',
      help='Output commands instead of executing them',
      action='store_true')
  parser.add_argument(
      '-v',
      '--verbose',
      help='Output additional diagnostic information',
      action='store_true')
  parser.add_argument(
      '--slow',
      help=
      'Run all tests, even those known to take a long time. Ignored if specific tests are passed in.',
      action='store_true')
  parser.add_argument(
      'tests',
      help=
      "Tests to be run, defaults to all. Valid entries are 'pdfium_unittests', 'pdfium_embeddertests', "
      "'corpus_tests', 'javascript_tests', and 'pixel_tests'",
      nargs='*')

  args = vars(parser.parse_args())
  if args['verbose']:
    pprint.pprint(args)

  executor = CoverageExecutor(parser, args)
  if executor.run():
    return 0
  else:
    return 1


if __name__ == '__main__':
  sys.exit(main())
