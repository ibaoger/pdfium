# SafetyNet - Performance regression detection for PDFium

[TOC]

This guide explains how to use SafetyNet to detect performance regressions
in PDFium.

## Running a comparison

safetynet_compare.py is a script that compares the performance between two
versions of pdfium. This can be used to verify if a given change has caused
or will cause any positive or negative changes in performance for a set of test
cases.

The profilers used are exclusive to Linux, so for now this can only be run on
Linux.

### Usage

Run the safetynet_compare.py script in testing/tools.

Pass one or more paths with test cases. Each path can be either a .pdf file or
a directory containing .pdf files - other files in those directories are
ignored.

The following comparison modes are supported:

Compare uncommitted changes against clean branch:
```shell
$ testing/tools/safetynet_compare.py path/to/pdfs
```

Compare current branch with another branch or commit:
```shell
$ testing/tools/safetynet_compare.py path/to/pdfs --branch-before another_branch
$ testing/tools/safetynet_compare.py path/to/pdfs --branch-before 1a3c5e7
```

Compare two other branches or commits:
```shell
$ testing/tools/safetynet_compare.py path/to/pdfs --branch-after another_branch --branch-before yet_another_branch
$ testing/tools/safetynet_compare.py path/to/pdfs --branch-after 1a3c5e7 --branch-before 0b2d4f6
$ testing/tools/safetynet_compare.py path/to/pdfs --branch-after another_branch --branch-before 0b2d4f6
```

Compare two build flag configurations:
```shell
$ gn args out/BuildConfig1
$ gn args out/BuildConfig2
$ testing/tools/safetynet_compare.py path/to/pdfs --build-dir out/BuildConfig2 --build-dir-before out/BuildConfig1
```

### Profilers

safetynet_compare.py must use either of these profilers:

#### perf stat

Only works on Linux.
Make sure you have perf by typing in the terminal:
```shell
$ perf
```

This is a fast profiler, but uses sampling so it's slightly inaccurate.
Expect variations of up to 1%, which is below the cutoff to consider a
change significant.

Use this when running over large test sets to get good enough results.

#### callgrind

Only works on Linux.
Make sure valgrind is installed:
```shell
$ valgrind
```

This is a slow and accurate profiler. Expect variations of around 100
instructions. However, this takes about 50 times longer to run than perf stat.

Use this when looking for small variations (< 1%).

One advantage is that callgrind generates callgrind.out files (pass --output-dir
to safetynet_compare.py) with the profiling information that can be analyzed to
find the cause of a regression. KCachegrind is a good visualizer for these
files.

## Common Options
* --build-dir: this allows testing various configs, but defaults to out/Release.
It is a relative path from the src directory to the build directory.
* --profiler: callgrind or perfstat. Defaults to callgrind.
* --output-dir: where to place the profiling output files. These are
callgrind.out.[test_case] files for callgrind, perfstat does not produce them.
By default they are not written.
* --case-order: sort test case results according to this metric. Can be "after",
"before", "ratio" and "rating". If not specified, sort by path.
* --this-repo: use the repository where the script is instead of checking out a
temporary one. This is faster and does not require downloads, but although it
restores the state of the local repo, if the script is killed or crashes the
changes can remain stashed and you may be on another branch.

## Other Options
* --build-dir-before: if comparing different build dirs (say, to test what a
flag flip does), specify the build dir for the “before” branch here and the
build dir for the “after” branch with --build-dir.
* --interesting-section: only the interesting section should be measured instead
of all the execution of the test harness. This only works in debug, since in
release the delimiters are stripped out. This does not work to compare branches
that don’t have the callgrind delimiters, as it would otherwise be unfair to
compare a whole run vs the interesting section of another run.
* --machine-readable: print out a json that’s easier to read by code with the
results instead of a terminal-formatted output.
* --num-workers: how many workers to use to parallelize. Defaults to # of CPUs
in the machine
* --threshold-significant: highlight differences that exceed this value.
Defaults to 0.02.
* --tmp-dir: directory in which temporary repos will be cloned. Defaults to
/tmp.


## Setup a nightly job

TODO: Complete with safetynet_job.py setup and usage
