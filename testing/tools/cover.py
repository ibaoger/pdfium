#!/usr/bin/env python

# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Converts .gcov files into nicer HTML format.

llvm-cov-3.5 gcov -abcf -stats -u
"""

import optparse
import os
import re
import subprocess
import sys

class CovBranch:
  def __init__(self, num, count):
    self.branch_number = num
    self.taken_count = count


class CovLine:
  def __init__(self, line_number, count, text):
    self.count = count
    self.line_number = line_number
    self.text = text
    self.branch_info = []

  def add_branch_info(self, branch):
    self.branch_info.append(branch)


class CovFunc:
  def __init__(self, name, count, percent_executed):
    self.name = name
    self.count = count
    self.percent_executed = int(percent_executed[0:-1])
    self.line_number = 0


class CovFile:
  def __init__(self, filename):
    self.filename = filename
    self.functions = []
    self.lines = []
    self.info = {}

  def add_line(self, line):
    self.lines.append(line)

  def add_function(self, func):
    self.functions.append(func)

  def add_info(self, key, value):
    self.info[key] = value

  def coverage(self):
    count = len(self.functions)
    percent = 0
    for func in self.functions:
      percent += func.percent_executed

    return percent / count

  def outname(self):
    return os.path.basename(self.filename) + ".html"


def demangle(name):
  return subprocess.check_output(['c++filt', name], universal_newlines=True).strip()


def process_gcov_file(file):
  # Strip the .gcov from the filename
  cov = CovFile(file[0:-5])
  cur_line = None
  cur_func = None

  func_re = re.compile('^function ')
  branch_re = re.compile('^branch ')
  block_re = re.compile('^[0-9]+-block')
  with open(file, 'rb') as infile:
    for line in infile:
      if func_re.match(line):
        parts = map(str.strip, line.split(' '))
        name = demangle(parts[1])
        count = parts[3]
        percent_executed = parts[8]
        cur_func = CovFunc(name, count, percent_executed)
        cov.add_function(cur_func)

      elif branch_re.match(line):
        parts = map(str.strip, line.split(' '))
        branch = CovBranch(parts[1], parts[3])

        if cur_line == None:
          print "ERORR: Branch before line? (" + line + ")"
          return None

        cur_line.add_branch_info(branch)

      else:
        parts = line.split(':')

        line = parts[1].strip()
        if line == '0':
          cov.add_info(parts[2].strip(), parts[3].strip())
          continue

        count = 0
        count_data = parts[0].strip()
        if count_data != '#####' and count_data != '$$$$$' and count_data != '-':
          count = count_data

        # Skip function block information for now
        if block_re.match(line):
          continue

        if cur_func != None:
          cur_func.line_number = line
          cur_func = None

        cur_line = CovLine(line, count, ':'.join(parts[2:]))
        cov.add_line(cur_line)

  return cov

def main():
  parser = optparse.OptionParser()
  parser.add_option('--output-dir', default='')
  parser.add_option('--data-dir', default='')
  options, args = parser.parse_args()

  if options.data_dir == "":
    print "Must provide --data-dir option."
    return 1
  if options.output_dir == "":
    print "Must provide --output-dir option."
    return 1

  if not os.path.exists(options.output_dir):
    os.makedirs(options.output_dir)

  files = []
  gcov_re = re.compile('^[a-zA-Z0-9_.]+[.]gcov$')
  for file_dir, _, filename_list in os.walk(options.data_dir):
    for input_filename in filename_list:
      if gcov_re.match(input_filename):
        input_path = os.path.join(file_dir, input_filename)
        if os.path.isfile(input_path):
          files.append(input_path)



  info = []
  for file in sorted(files):
    print "Processing %s" % file
    d = process_gcov_file(file)
    if d == None:
      print("Error processing " + file)
    info.append(d)

  out_index = os.path.join(options.output_dir, "index.html")
  with open(out_index, "wb") as index:
    html = """<!doctype html>
<head>
<title>PDFium Coverage</title>
<style>
table { border-collapse: collapse; border-spacing: 0; }
th, tr, td { padding: 0; margin: 0; border: 0;}
tr:nth-child(even) { background-color: #eee; }
pre { margin: 0; }
.empty { background-color: #ffeded; }
</style>
</head>
<body>
"""
    index.write(html)
    index.write("<h1>TOC</h1><ul>")

    for i in info:
      l = "<li><a href='" + i.outname() + "'>" + i.filename + "</a> &mdash; "
      l += str(i.coverage()) + "% covered</li>"
      index.write(l)

      out_file = os.path.join(options.output_dir, i.outname())
      with open(out_file, "wb") as outfile:
        outfile.write(html)

        outfile.write("<h1>" + i.outname()[0:-5] + "</h1>\n")
        outfile.write("<ol>\n")
        for func in i.functions:
          outfile.write("<li><a href='#line-" + str(func.line_number) + "'>")
          outfile.write(func.name + "</a> called " + func.count + " times. ")
          outfile.write(str(func.percent_executed) + "% covered.</li>\n")
        outfile.write("</ol>\n")

        outfile.write("<h2>Details</h2>\n")
        outfile.write("<table>\n")
        outfile.write("<tr><th>Count</th><th>Line</th></tr>")
        for line in i.lines:
          outfile.write("<tr><td>" + str(line.count) + "</td><td>")
          outfile.write("<a name='line-" + str(line.line_number) + "'>")
          outfile.write("<pre")
          if line.count == 0:
            outfile.write(" class='empty'")

          outfile.write("><code>" + line.text + "</code></pre></a></td></tr>\n")
        outfile.write("</table>\n")
        outfile.write("</body></html>")
    index.write("</ul></body></html>")

  return 0

if __name__ == '__main__':
  sys.exit(main())
