// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_coordinates.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_Matrix, ReverseIdentity) {
  CFX_Matrix m;
  CFX_Matrix rev;

  m.SetIdentity();
  rev.SetReverse(m);
  EXPECT_FLOAT_EQ(1.0, rev.a);
  EXPECT_FLOAT_EQ(0.0, rev.b);
  EXPECT_FLOAT_EQ(0.0, rev.c);
  EXPECT_FLOAT_EQ(1.0, rev.d);
  EXPECT_FLOAT_EQ(0.0, rev.e);
  EXPECT_FLOAT_EQ(0.0, rev.f);

  CFX_PointF expected(2, 3);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

TEST(CFX_Matrix, Reverse) {
  float data[6] = {3, 0, 2, 3, 1, 4};
  CFX_Matrix m(data);
  CFX_Matrix rev;

  rev.SetReverse(m);
  EXPECT_FLOAT_EQ(0.33333334, rev.a);
  EXPECT_FLOAT_EQ(0.0, rev.b);
  EXPECT_FLOAT_EQ(-0.22222222, rev.c);
  EXPECT_FLOAT_EQ(0.33333334, rev.d);
  EXPECT_FLOAT_EQ(0.55555556, rev.e);
  EXPECT_FLOAT_EQ(-1.3333334, rev.f);

  CFX_PointF expected(2, 3);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

// Note, I think these are a bug and the matrix should be the identity.
TEST(CFX_Matrix, ReverseCR702041) {
  // The determinate is < std::numeric_limits<float>::epsilon()
  float data[6] = {0.947368443, -0.108947366, -0.923076928,
                   0.106153846, 18,           787.929993};
  CFX_Matrix m(data);
  CFX_Matrix rev;

  rev.SetReverse(m);
  EXPECT_FLOAT_EQ(14247728, rev.a);
  EXPECT_FLOAT_EQ(14622668, rev.b);
  EXPECT_FLOAT_EQ(1.2389329e+08, rev.c);
  EXPECT_FLOAT_EQ(1.2715364e+08, rev.d);
  EXPECT_FLOAT_EQ(-9.7875698e+10, rev.e);
  EXPECT_FLOAT_EQ(-1.0045138e+11, rev.f);

  // Should be 2, 3
  CFX_PointF expected(0, 0);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

TEST(CFX_Matrix, ReverseCR714187) {
  // The determinate is < std::numeric_limits<float>::epsilon()
  float data[6] = {0.000037, 0, 0, -0.000037, 182.413101, 136.977646};
  CFX_Matrix m(data);
  CFX_Matrix rev;

  rev.SetReverse(m);
  EXPECT_FLOAT_EQ(27027.025, rev.a);
  EXPECT_FLOAT_EQ(0, rev.b);
  EXPECT_FLOAT_EQ(0, rev.c);
  EXPECT_FLOAT_EQ(-27027.025, rev.d);
  EXPECT_FLOAT_EQ(-4930083.5, rev.e);
  EXPECT_FLOAT_EQ(3702098.2, rev.f);

  // Should be 3 ....
  CFX_PointF expected(2, 2.75);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}
