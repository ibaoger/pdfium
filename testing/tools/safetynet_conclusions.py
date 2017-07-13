#!/usr/bin/env python
# Copyright 2017 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from collections import Counter


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


class ComparisonConclusions(object):

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
    """
    Returns a dict that can be serialized with the format below.

    {
      "params": {
        "threshold": 0.02
      },
      "summary": {
        "total": 123,
        "failure": 1,
        "regression": 2,
        "improvement": 1,
        "no_change": 100,
        "small_change": 19
      },
      "comparison_by_case": {
        "testing/resources/new_test.pdf": {
          "before": null,
          "after": 1000,
          "ratio": null,
          "rating": "failure"
        },
        "testing/resources/test1.pdf": {
          "before": 100,
          "after": 120,
          "ratio": 0.2,
          "rating": "regression"
        },
        "testing/resources/test2.pdf": {
          "before": 100,
          "after": 2000,
          "ratio": 19.0,
          "rating": "regression"
        },
        "testing/resources/test3.pdf": {
          "before": 1000,
          "after": 1005,
          "ratio": 0.005,
          "rating": "small_change"
        },
        "testing/resources/test4.pdf": {
          "before": 1000,
          "after": 1000,
          "ratio": 0.0,
          "rating": "no_change"
        },
        "testing/resources/test5.pdf": {
          "before": 1000,
          "after": 600,
          "ratio": -0.4,
          "rating": "improvement"
        }
      }
    }
    """
    output_dict = {}
    output_dict['params'] = {'threshold': self.threshold_significant}
    output_dict['summary'] = self.summary.GetOutputDict()
    output_dict['comparison_by_case'] = \
        {cr.case_name: cr.GetOutputDict()
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

  def GetOutputDict(self):
    return {'before': self.before,
            'after': self.after,
            'ratio': self.ratio,
            'rating': self.rating}


def PrintConclusionsDictHumanReadable(conclusions_dict):
  # Print header
  print '=' * 80
  print '{0:>11s} {1:>15s}  {2}' .format(
      '% Change',
      'Time after',
      'Test case')
  print '-' * 80

  # Print cases
  for case_name, case_dict in sorted(conclusions_dict['comparison_by_case']
                                    .iteritems()):
    if case_dict['rating'] == RATING_FAILURE:
      print ('Failed to measure time for %s' % test_case)
      continue

    color = RATING_TO_COLOR[case_dict['rating']]

    print '{0} {1:15,d}  {2}' .format(
        color.format('{:+11.4%}'.format(case_dict['ratio'])),
        case_dict['after'],
        case_name)

  # Print totals
  totals = conclusions_dict['summary']
  print '=' * 80
  print 'Test cases run: %d' % totals['total']
  color = FORMAT_RED if totals[RATING_FAILURE] else FORMAT_GREEN
  print ('Failed to measure: %s'
         % color.format(totals[RATING_FAILURE]))
  color = FORMAT_RED if totals[RATING_REGRESSION] else FORMAT_GREEN
  print ('Regressions: %s'
         % color.format(totals[RATING_REGRESSION]))
  color = FORMAT_BLUE if totals[RATING_IMPROVEMENT] else FORMAT_GREEN
  print ('Improvements: %s'
         % color.format(totals[RATING_IMPROVEMENT]))
