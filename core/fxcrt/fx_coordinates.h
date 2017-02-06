// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_COORDINATES_H_
#define CORE_FXCRT_FX_COORDINATES_H_

#include "core/fxcrt/fx_basic.h"

class CFX_Matrix;

template <class BaseType>
class CFX_PTemplate {
 public:
  CFX_PTemplate() : x(0), y(0) {}
  CFX_PTemplate(BaseType new_x, BaseType new_y) : x(new_x), y(new_y) {}
  CFX_PTemplate(const CFX_PTemplate& other) : x(other.x), y(other.y) {}
  void clear() {
    x = 0;
    y = 0;
  }
  CFX_PTemplate operator=(const CFX_PTemplate& other) {
    if (this != &other) {
      x = other.x;
      y = other.y;
    }
    return *this;
  }
  bool operator==(const CFX_PTemplate& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const CFX_PTemplate& other) const {
    return !(*this == other);
  }
  CFX_PTemplate& operator+=(const CFX_PTemplate<BaseType>& obj) {
    x += obj.x;
    y += obj.y;
    return *this;
  }
  CFX_PTemplate& operator-=(const CFX_PTemplate<BaseType>& obj) {
    x -= obj.x;
    y -= obj.y;
    return *this;
  }
  CFX_PTemplate& operator*=(BaseType factor) {
    x *= factor;
    y *= factor;
    return *this;
  }
  CFX_PTemplate& operator/=(BaseType divisor) {
    x /= divisor;
    y /= divisor;
    return *this;
  }
  CFX_PTemplate operator+(const CFX_PTemplate& other) {
    return CFX_PTemplate(x + other.x, y + other.y);
  }
  CFX_PTemplate operator-(const CFX_PTemplate& other) {
    return CFX_PTemplate(x - other.x, y - other.y);
  }
  CFX_PTemplate operator*(BaseType factor) {
    return CFX_PTemplate(x * factor, y * factor);
  }
  CFX_PTemplate operator/(BaseType divisor) {
    return CFX_PTemplate(x / divisor, y / divisor);
  }

  BaseType x;
  BaseType y;
};
typedef CFX_PTemplate<int32_t> CFX_Point;
typedef CFX_PTemplate<FX_FLOAT> CFX_PointF;

template <class BaseType>
class CFX_STemplate {
 public:
  CFX_STemplate() : x(0), y(0) {}
  CFX_STemplate(BaseType new_x, BaseType new_y) : x(new_x), y(new_y) {}
  CFX_STemplate(const CFX_STemplate& other) : x(other.x), y(other.y) {}
  void clear() {
    x = 0;
    y = 0;
  }
  CFX_STemplate operator=(const CFX_STemplate& other) {
    if (this != &other) {
      x = other.x;
      y = other.y;
    }
    return *this;
  }
  bool operator==(const CFX_STemplate& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const CFX_STemplate& other) const {
    return !(*this == other);
  }
  CFX_STemplate& operator+=(const CFX_STemplate<BaseType>& obj) {
    x += obj.x;
    y += obj.y;
    return *this;
  }
  CFX_STemplate& operator-=(const CFX_STemplate<BaseType>& obj) {
    x -= obj.x;
    y -= obj.y;
    return *this;
  }
  CFX_STemplate& operator*=(BaseType factor) {
    x *= factor;
    y *= factor;
    return *this;
  }
  CFX_STemplate& operator/=(BaseType divisor) {
    x /= divisor;
    y /= divisor;
    return *this;
  }
  CFX_STemplate operator+(const CFX_STemplate& other) {
    return CFX_STemplate(x + other.x, y + other.y);
  }
  CFX_STemplate operator-(const CFX_STemplate& other) {
    return CFX_STemplate(x - other.x, y - other.y);
  }
  CFX_STemplate operator*(BaseType factor) {
    return CFX_STemplate(x * factor, y * factor);
  }
  CFX_STemplate operator/(BaseType divisor) {
    return CFX_STemplate(x / divisor, y / divisor);
  }

  BaseType x;
  BaseType y;
};
typedef CFX_STemplate<int32_t> CFX_Size;
typedef CFX_STemplate<FX_FLOAT> CFX_SizeF;

template <class BaseType>
class CFX_VTemplate : public CFX_PTemplate<BaseType> {
 public:
  using CFX_PTemplate<BaseType>::x;
  using CFX_PTemplate<BaseType>::y;

  CFX_VTemplate() : CFX_PTemplate<BaseType>() {}
  CFX_VTemplate(BaseType new_x, BaseType new_y)
      : CFX_PTemplate<BaseType>(new_x, new_y) {}

  CFX_VTemplate(const CFX_VTemplate& other) : CFX_PTemplate<BaseType>(other) {}

  CFX_VTemplate(const CFX_PTemplate<BaseType>& point1,
                const CFX_PTemplate<BaseType>& point2)
      : CFX_PTemplate<BaseType>(point2.x - point1.x, point2.y - point1.y) {}

  FX_FLOAT Length() const { return FXSYS_sqrt(x * x + y * y); }
  void Normalize() {
    FX_FLOAT fLen = Length();
    if (fLen < 0.0001f)
      return;

    x /= fLen;
    y /= fLen;
  }
  void Translate(BaseType dx, BaseType dy) {
    x += dx;
    y += dy;
  }
  void Scale(BaseType sx, BaseType sy) {
    x *= sx;
    y *= sy;
  }
  void Rotate(FX_FLOAT fRadian) {
    FX_FLOAT cosValue = FXSYS_cos(fRadian);
    FX_FLOAT sinValue = FXSYS_sin(fRadian);
    x = x * cosValue - y * sinValue;
    y = x * sinValue + y * cosValue;
  }
};
typedef CFX_VTemplate<int32_t> CFX_Vector;
typedef CFX_VTemplate<FX_FLOAT> CFX_VectorF;

// Rectangles.
// TODO(tsepez): Consolidate all these different rectangle classes.

// LTRB rectangles (y-axis runs downwards).
struct FX_RECT {
  FX_RECT() : left(0), top(0), right(0), bottom(0) {}

  FX_RECT(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}

  int Width() const { return right - left; }
  int Height() const { return bottom - top; }
  bool IsEmpty() const { return right <= left || bottom <= top; }

  bool Valid() const {
    pdfium::base::CheckedNumeric<int> w = right;
    pdfium::base::CheckedNumeric<int> h = bottom;
    w -= left;
    h -= top;
    return w.IsValid() && h.IsValid();
  }

  void Normalize();

  void Intersect(const FX_RECT& src);
  void Intersect(int l, int t, int r, int b) { Intersect(FX_RECT(l, t, r, b)); }

  void Union(const FX_RECT& other_rect);
  void Union(int l, int t, int r, int b) { Union(FX_RECT(l, t, r, b)); }

  void Offset(int dx, int dy) {
    left += dx;
    right += dx;
    top += dy;
    bottom += dy;
  }

  bool operator==(const FX_RECT& src) const {
    return left == src.left && right == src.right && top == src.top &&
           bottom == src.bottom;
  }

  bool Contains(const FX_RECT& other_rect) const {
    return other_rect.left >= left && other_rect.right <= right &&
           other_rect.top >= top && other_rect.bottom <= bottom;
  }

  bool Contains(int x, int y) const {
    return x >= left && x < right && y >= top && y < bottom;
  }

  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
};

// LBRT rectangles (y-axis runs upwards).
class CFX_FloatPoint {
 public:
  CFX_FloatPoint() : x(0.0f), y(0.0f) {}
  CFX_FloatPoint(FX_FLOAT xx, FX_FLOAT yy) : x(xx), y(yy) {}

  bool operator==(const CFX_FloatPoint& that) const {
    return x == that.x && y == that.y;
  }
  bool operator!=(const CFX_FloatPoint& that) const { return !(*this == that); }

  FX_FLOAT x;
  FX_FLOAT y;
};

// LTWH rectangles (y-axis runs downwards).
template <class baseType>
class CFX_RTemplate {
 public:
  typedef CFX_PTemplate<baseType> FXT_POINT;
  typedef CFX_STemplate<baseType> FXT_SIZE;
  typedef CFX_VTemplate<baseType> FXT_VECTOR;
  typedef CFX_RTemplate<baseType> FXT_RECT;

  CFX_RTemplate() : left(0), top(0), width(0), height(0) {}
  CFX_RTemplate(baseType dst_left,
                baseType dst_top,
                baseType dst_width,
                baseType dst_height)
      : left(dst_left), top(dst_top), width(dst_width), height(dst_height) {}
  CFX_RTemplate(baseType dst_left, baseType dst_top, const FXT_SIZE& dst_size)
      : left(dst_left), top(dst_top), width(dst_size.x), height(dst_size.y) {}
  CFX_RTemplate(const FXT_POINT& p, baseType dst_width, baseType dst_height)
      : left(p.x), top(p.y), width(dst_width), height(dst_height) {}
  CFX_RTemplate(const FXT_POINT& p1, const FXT_SIZE& s2)
      : left(p1.x), top(p1.y), width(s2.x), height(s2.y) {}

  void Set(baseType dst_left,
           baseType dst_top,
           baseType dst_width,
           baseType dst_height) {
    left = dst_left;
    top = dst_top;
    width = dst_width;
    height = dst_height;
  }
  void Set(baseType dst_left, baseType dst_top, const FXT_SIZE& dst_size) {
    left = dst_left;
    top = dst_top;
    width = dst_size.x;
    height = dst_size.y;
  }
  void Set(const FXT_POINT& p, baseType dst_width, baseType dst_height) {
    left = p.x;
    top = p.y;
    width = dst_width;
    height = dst_height;
  }
  void Set(const FXT_POINT& p, const FXT_SIZE& s) {
    left = p.x;
    top = p.y;
    width = s.x;
    height = s.y;
  }
  void Set(const FXT_POINT& p1, const FXT_POINT& p2) {
    left = p1.x;
    top = p1.y;
    width = p2.x - p1.x;
    height = p2.y - p1.y;
    Normalize();
  }
  void Set(const FXT_POINT& p, const FXT_VECTOR& v) {
    left = p.x;
    top = p.y;
    width = v.x;
    height = v.y;
    Normalize();
  }
  void Reset() {
    left = 0;
    top = 0;
    width = 0;
    height = 0;
  }
  FXT_RECT& operator+=(const FXT_POINT& p) {
    left += p.x;
    top += p.y;
    return *this;
  }
  FXT_RECT& operator-=(const FXT_POINT& p) {
    left -= p.x;
    top -= p.y;
    return *this;
  }
  baseType right() const { return left + width; }
  baseType bottom() const { return top + height; }
  void Normalize() {
    if (width < 0) {
      left += width;
      width = -width;
    }
    if (height < 0) {
      top += height;
      height = -height;
    }
  }
  void Offset(baseType dx, baseType dy) {
    left += dx;
    top += dy;
  }
  void Inflate(baseType x, baseType y) {
    left -= x;
    width += x * 2;
    top -= y;
    height += y * 2;
  }
  void Inflate(const FXT_POINT& p) { Inflate(p.x, p.y); }
  void Inflate(baseType off_left,
               baseType off_top,
               baseType off_right,
               baseType off_bottom) {
    left -= off_left;
    top -= off_top;
    width += off_left + off_right;
    height += off_top + off_bottom;
  }
  void Inflate(const FXT_RECT& rt) {
    Inflate(rt.left, rt.top, rt.left + rt.width, rt.top + rt.height);
  }
  void Deflate(baseType x, baseType y) {
    left += x;
    width -= x * 2;
    top += y;
    height -= y * 2;
  }
  void Deflate(const FXT_POINT& p) { Deflate(p.x, p.y); }
  void Deflate(baseType off_left,
               baseType off_top,
               baseType off_right,
               baseType off_bottom) {
    left += off_left;
    top += off_top;
    width -= off_left + off_right;
    height -= off_top + off_bottom;
  }
  void Deflate(const FXT_RECT& rt) {
    Deflate(rt.left, rt.top, rt.top + rt.width, rt.top + rt.height);
  }
  bool IsEmpty() const { return width <= 0 || height <= 0; }
  bool IsEmpty(FX_FLOAT fEpsilon) const {
    return width <= fEpsilon || height <= fEpsilon;
  }
  void Empty() { width = height = 0; }
  bool Contains(baseType x, baseType y) const {
    return x >= left && x < left + width && y >= top && y < top + height;
  }
  bool Contains(const FXT_POINT& p) const { return Contains(p.x, p.y); }
  bool Contains(const FXT_RECT& rt) const {
    return rt.left >= left && rt.right() <= right() && rt.top >= top &&
           rt.bottom() <= bottom();
  }
  baseType Width() const { return width; }
  baseType Height() const { return height; }
  FXT_SIZE Size() const {
    FXT_SIZE size;
    size.Set(width, height);
    return size;
  }
  FXT_POINT TopLeft() const {
    FXT_POINT p;
    p.x = left;
    p.y = top;
    return p;
  }
  FXT_POINT TopRight() const {
    FXT_POINT p;
    p.x = left + width;
    p.y = top;
    return p;
  }
  FXT_POINT BottomLeft() const {
    FXT_POINT p;
    p.x = left;
    p.y = top + height;
    return p;
  }
  FXT_POINT BottomRight() const {
    FXT_POINT p;
    p.x = left + width;
    p.y = top + height;
    return p;
  }
  FXT_POINT Center() const {
    FXT_POINT p;
    p.x = left + width / 2;
    p.y = top + height / 2;
    return p;
  }
  void Union(baseType x, baseType y) {
    baseType r = right();
    baseType b = bottom();
    if (left > x)
      left = x;
    if (r < x)
      r = x;
    if (top > y)
      top = y;
    if (b < y)
      b = y;
    width = r - left;
    height = b - top;
  }
  void Union(const FXT_POINT& p) { Union(p.x, p.y); }
  void Union(const FXT_RECT& rt) {
    baseType r = right();
    baseType b = bottom();
    if (left > rt.left)
      left = rt.left;
    if (r < rt.right())
      r = rt.right();
    if (top > rt.top)
      top = rt.top;
    if (b < rt.bottom())
      b = rt.bottom();
    width = r - left;
    height = b - top;
  }
  void Intersect(const FXT_RECT& rt) {
    baseType r = right();
    baseType b = bottom();
    if (left < rt.left)
      left = rt.left;
    if (r > rt.right())
      r = rt.right();
    if (top < rt.top)
      top = rt.top;
    if (b > rt.bottom())
      b = rt.bottom();
    width = r - left;
    height = b - top;
  }
  bool IntersectWith(const FXT_RECT& rt) const {
    FXT_RECT rect = rt;
    rect.Intersect(*this);
    return !rect.IsEmpty();
  }
  bool IntersectWith(const FXT_RECT& rt, FX_FLOAT fEpsilon) const {
    FXT_RECT rect = rt;
    rect.Intersect(*this);
    return !rect.IsEmpty(fEpsilon);
  }
  friend bool operator==(const FXT_RECT& rc1, const FXT_RECT& rc2) {
    return rc1.left == rc2.left && rc1.top == rc2.top &&
           rc1.width == rc2.width && rc1.height == rc2.height;
  }
  friend bool operator!=(const FXT_RECT& rc1, const FXT_RECT& rc2) {
    return !(rc1 == rc2);
  }

  baseType left;
  baseType top;
  baseType width;
  baseType height;
};
typedef CFX_RTemplate<int32_t> CFX_Rect;
typedef CFX_RTemplate<FX_FLOAT> CFX_RectF;

class CFX_FloatRect {
 public:
  CFX_FloatRect() : CFX_FloatRect(0.0f, 0.0f, 0.0f, 0.0f) {}
  CFX_FloatRect(FX_FLOAT l, FX_FLOAT b, FX_FLOAT r, FX_FLOAT t)
      : left(l), bottom(b), right(r), top(t) {}

  explicit CFX_FloatRect(const FX_FLOAT* pArray)
      : CFX_FloatRect(pArray[0], pArray[1], pArray[2], pArray[3]) {}

  explicit CFX_FloatRect(const FX_RECT& rect);

  void Normalize();

  void Reset() {
    left = 0.0f;
    right = 0.0f;
    bottom = 0.0f;
    top = 0.0f;
  }

  bool IsEmpty() const { return left >= right || bottom >= top; }
  bool Contains(const CFX_FloatRect& other_rect) const;
  bool Contains(FX_FLOAT x, FX_FLOAT y) const;

  void Transform(const CFX_Matrix* pMatrix);
  void Intersect(const CFX_FloatRect& other_rect);
  void Union(const CFX_FloatRect& other_rect);

  FX_RECT GetInnerRect() const;
  FX_RECT GetOuterRect() const;
  FX_RECT GetClosestRect() const;

  int Substract4(CFX_FloatRect& substract_rect, CFX_FloatRect* pRects);

  void InitRect(FX_FLOAT x, FX_FLOAT y) {
    left = x;
    right = x;
    bottom = y;
    top = y;
  }
  void UpdateRect(FX_FLOAT x, FX_FLOAT y);

  FX_FLOAT Width() const { return right - left; }
  FX_FLOAT Height() const { return top - bottom; }

  void Inflate(FX_FLOAT x, FX_FLOAT y) {
    Normalize();
    left -= x;
    right += x;
    bottom -= y;
    top += y;
  }

  void Inflate(FX_FLOAT other_left,
               FX_FLOAT other_bottom,
               FX_FLOAT other_right,
               FX_FLOAT other_top) {
    Normalize();
    left -= other_left;
    bottom -= other_bottom;
    right += other_right;
    top += other_top;
  }

  void Inflate(const CFX_FloatRect& rt) {
    Inflate(rt.left, rt.bottom, rt.right, rt.top);
  }

  void Deflate(FX_FLOAT x, FX_FLOAT y) {
    Normalize();
    left += x;
    right -= x;
    bottom += y;
    top -= y;
  }

  void Deflate(FX_FLOAT other_left,
               FX_FLOAT other_bottom,
               FX_FLOAT other_right,
               FX_FLOAT other_top) {
    Normalize();
    left += other_left;
    bottom += other_bottom;
    right -= other_right;
    top -= other_top;
  }

  void Deflate(const CFX_FloatRect& rt) {
    Deflate(rt.left, rt.bottom, rt.right, rt.top);
  }

  void Translate(FX_FLOAT e, FX_FLOAT f) {
    left += e;
    right += e;
    top += f;
    bottom += f;
  }

  static CFX_FloatRect GetBBox(const CFX_PointF* pPoints, int nPoints);

  FX_RECT ToFxRect() const {
    return FX_RECT(static_cast<int32_t>(left), static_cast<int32_t>(top),
                   static_cast<int32_t>(right), static_cast<int32_t>(bottom));
  }

  static CFX_FloatRect FromCFXRectF(const CFX_RectF& rect) {
    return CFX_FloatRect(rect.left, rect.top, rect.right(), rect.bottom());
  }

  FX_FLOAT left;
  FX_FLOAT bottom;
  FX_FLOAT right;
  FX_FLOAT top;
};

class CFX_Matrix {
 public:
  CFX_Matrix() { SetIdentity(); }

  CFX_Matrix(FX_FLOAT a1,
             FX_FLOAT b1,
             FX_FLOAT c1,
             FX_FLOAT d1,
             FX_FLOAT e1,
             FX_FLOAT f1) {
    a = a1;
    b = b1;
    c = c1;
    d = d1;
    e = e1;
    f = f1;
  }

  void Set(FX_FLOAT a,
           FX_FLOAT b,
           FX_FLOAT c,
           FX_FLOAT d,
           FX_FLOAT e,
           FX_FLOAT f);
  void Set(const FX_FLOAT n[6]);

  void SetIdentity() {
    a = d = 1;
    b = c = e = f = 0;
  }

  void SetReverse(const CFX_Matrix& m);

  void Concat(FX_FLOAT a,
              FX_FLOAT b,
              FX_FLOAT c,
              FX_FLOAT d,
              FX_FLOAT e,
              FX_FLOAT f,
              bool bPrepended = false);
  void Concat(const CFX_Matrix& m, bool bPrepended = false);
  void ConcatInverse(const CFX_Matrix& m, bool bPrepended = false);

  bool IsIdentity() const {
    return a == 1 && b == 0 && c == 0 && d == 1 && e == 0 && f == 0;
  }

  bool IsInvertible() const;
  bool Is90Rotated() const;
  bool IsScaled() const;

  void Translate(FX_FLOAT x, FX_FLOAT y, bool bPrepended = false);
  void TranslateI(int32_t x, int32_t y, bool bPrepended = false) {
    Translate((FX_FLOAT)x, (FX_FLOAT)y, bPrepended);
  }

  void Scale(FX_FLOAT sx, FX_FLOAT sy, bool bPrepended = false);
  void Rotate(FX_FLOAT fRadian, bool bPrepended = false);
  void RotateAt(FX_FLOAT fRadian,
                FX_FLOAT x,
                FX_FLOAT y,
                bool bPrepended = false);

  void Shear(FX_FLOAT fAlphaRadian,
             FX_FLOAT fBetaRadian,
             bool bPrepended = false);

  void MatchRect(const CFX_FloatRect& dest, const CFX_FloatRect& src);
  FX_FLOAT GetXUnit() const;
  FX_FLOAT GetYUnit() const;
  void GetUnitRect(CFX_RectF& rect) const;
  CFX_FloatRect GetUnitRect() const;

  FX_FLOAT GetUnitArea() const;
  FX_FLOAT TransformXDistance(FX_FLOAT dx) const;
  int32_t TransformXDistance(int32_t dx) const;
  FX_FLOAT TransformYDistance(FX_FLOAT dy) const;
  int32_t TransformYDistance(int32_t dy) const;
  FX_FLOAT TransformDistance(FX_FLOAT dx, FX_FLOAT dy) const;
  int32_t TransformDistance(int32_t dx, int32_t dy) const;
  FX_FLOAT TransformDistance(FX_FLOAT distance) const;

  void TransformPoint(FX_FLOAT& x, FX_FLOAT& y) const;
  void TransformPoint(int32_t& x, int32_t& y) const;

  void Transform(FX_FLOAT& x, FX_FLOAT& y) const { TransformPoint(x, y); }
  void Transform(FX_FLOAT x, FX_FLOAT y, FX_FLOAT& x1, FX_FLOAT& y1) const {
    x1 = x, y1 = y;
    TransformPoint(x1, y1);
  }

  void TransformVector(CFX_VectorF& v) const;
  void TransformVector(CFX_Vector& v) const;
  void TransformRect(CFX_RectF& rect) const;
  void TransformRect(CFX_Rect& rect) const;
  void TransformRect(FX_FLOAT& left,
                     FX_FLOAT& right,
                     FX_FLOAT& top,
                     FX_FLOAT& bottom) const;
  void TransformRect(CFX_FloatRect& rect) const {
    TransformRect(rect.left, rect.right, rect.top, rect.bottom);
  }

  FX_FLOAT GetA() const { return a; }
  FX_FLOAT GetB() const { return b; }
  FX_FLOAT GetC() const { return c; }
  FX_FLOAT GetD() const { return d; }
  FX_FLOAT GetE() const { return e; }
  FX_FLOAT GetF() const { return f; }

 public:
  FX_FLOAT a;
  FX_FLOAT b;
  FX_FLOAT c;
  FX_FLOAT d;
  FX_FLOAT e;
  FX_FLOAT f;
};

#endif  // CORE_FXCRT_FX_COORDINATES_H_
