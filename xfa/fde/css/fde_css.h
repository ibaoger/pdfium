// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSS_H_
#define XFA_FDE_CSS_FDE_CSS_H_

#include "core/fxge/fx_dib.h"
#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fgas/crt/fgas_utils.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

class CFDE_CSSAccelerator;
class CFDE_CSSDeclaration;
class CFDE_CSSComputedStyle;
class CFDE_CSSSelector;
class CFDE_CSSStyleSheet;
class CXFA_CSSTagProvider;
class IFDE_CSSRule;
class IFDE_CSSValue;

enum FDE_CSSMEDIATYPE {
  FDE_CSSMEDIATYPE_Braille = 0x01,
  FDE_CSSMEDIATYPE_Emboss = 0x02,
  FDE_CSSMEDIATYPE_Handheld = 0x04,
  FDE_CSSMEDIATYPE_Print = 0x08,
  FDE_CSSMEDIATYPE_Projection = 0x10,
  FDE_CSSMEDIATYPE_Screen = 0x20,
  FDE_CSSMEDIATYPE_TTY = 0x40,
  FDE_CSSMEDIATYPE_TV = 0x80,
  FDE_CSSMEDIATYPE_ALL = 0xFF,
};

enum FDE_CSSVALUETYPE {
  FDE_CSSVALUETYPE_Primitive = 1 << 0,
  FDE_CSSVALUETYPE_List = 1 << 1,
  FDE_CSSVALUETYPE_Shorthand = 1 << 2,
  // Note the values below this comment must be > 0x0F so we can mask the above.
  FDE_CSSVALUETYPE_MaybeNumber = 1 << 4,
  FDE_CSSVALUETYPE_MaybeEnum = 1 << 5,
  FDE_CSSVALUETYPE_MaybeURI = 1 << 6,
  FDE_CSSVALUETYPE_MaybeString = 1 << 7,
  FDE_CSSVALUETYPE_MaybeColor = 1 << 8,
  FDE_CSSVALUETYPE_MaybeFunction = 1 << 9
};

enum class FDE_CSSPrimitiveType : uint8_t {
  Unknown = 0,
  Number,
  Percent,
  EMS,
  EXS,
  Pixels,
  CentiMeters,
  MilliMeters,
  Inches,
  Points,
  Picas,
  String,
  URI,
  RGB,
  Enum,
  Function,
};

enum class FDE_CSSPropertyValue : uint8_t {
  Bolder = 0,
  None,
  Dot,
  Sub,
  Top,
  Right,
  Normal,
  Auto,
  Text,
  XSmall,
  Thin,
  Small,
  Bottom,
  Underline,
  Double,
  Lighter,
  Oblique,
  Super,
  Center,
  XxLarge,
  Smaller,
  Baseline,
  Thick,
  Justify,
  Middle,
  Medium,
  ListItem,
  XxSmall,
  Bold,
  SmallCaps,
  Inline,
  Overline,
  TextBottom,
  Larger,
  InlineTable,
  InlineBlock,
  Blink,
  Block,
  Italic,
  LineThrough,
  XLarge,
  Large,
  Left,
  TextTop,
  LAST_MARKER
};

enum class FDE_CSSProperty : uint8_t {
  BorderLeft = 0,
  Top,
  Margin,
  TextIndent,
  Right,
  PaddingLeft,
  MarginLeft,
  Border,
  BorderTop,
  Bottom,
  PaddingRight,
  BorderBottom,
  FontFamily,
  FontWeight,
  Color,
  LetterSpacing,
  TextAlign,
  BorderRightWidth,
  VerticalAlign,
  PaddingTop,
  FontVariant,
  BorderWidth,
  BorderBottomWidth,
  BorderRight,
  FontSize,
  BorderSpacing,
  FontStyle,
  Font,
  LineHeight,
  MarginRight,
  BorderLeftWidth,
  Display,
  PaddingBottom,
  BorderTopWidth,
  WordSpacing,
  Left,
  TextDecoration,
  Padding,
  MarginBottom,
  MarginTop,
  LAST_MARKER
};

enum class FDE_CSSPseudo : uint8_t { After, Before, NONE };

enum class FDE_CSSSelectorType : uint8_t {
  Element = 0,
  Descendant,
  Class,
  Pseudo,
  ID,
};

enum class FDE_CSSRuleType : uint8_t { Style, Media, FontFace };

enum class FDE_CSSSyntaxStatus : uint8_t {
  Error,
  EOS,
  None,
  Charset,
  ImportRule,
  ImportClose,
  PageRule,
  StyleRule,
  FontFaceRule,
  MediaRule,
  MediaType,
  URI,
  Selector,
  DeclOpen,
  DeclClose,
  PropertyName,
  PropertyValue,
};

enum class FDE_CSSLengthUnit : uint8_t {
  Auto,
  None,
  Normal,
  Point,
  Percent,
};

enum class FDE_CSSDisplay : uint8_t {
  None,
  ListItem,
  Block,
  Inline,
  InlineBlock,
  InlineTable,
};

enum class FDE_CSSFontStyle : uint8_t {
  Normal,
  Italic,
};

enum class FDE_CSSTextAlign : uint8_t {
  Left,
  Right,
  Center,
  Justify,
  JustifyAll,
};

enum class FDE_CSSVerticalAlign : uint8_t {
  Baseline,
  Sub,
  Super,
  Top,
  TextTop,
  Middle,
  Bottom,
  TextBottom,
  Number,
};

enum class FDE_CSSFontVariant : uint8_t {
  Normal,
  SmallCaps,
};

enum FDE_CSSTEXTDECORATION {
  FDE_CSSTEXTDECORATION_None = 0,
  FDE_CSSTEXTDECORATION_Underline = 1 << 0,
  FDE_CSSTEXTDECORATION_Overline = 1 << 1,
  FDE_CSSTEXTDECORATION_LineThrough = 1 << 2,
  FDE_CSSTEXTDECORATION_Blink = 1 << 3,
  FDE_CSSTEXTDECORATION_Double = 1 << 4,
};

enum class FDE_CSSStyleSheetGroup : uint8_t {
  UserAgent = 0,
  User,
  Author,
};

enum class FDE_CSSStyleSheetPriority : uint8_t {
  High = 0,
  Mid,
  Low,
};

class IFDE_CSSValue {
 public:
  virtual ~IFDE_CSSValue() {}
  virtual FDE_CSSVALUETYPE GetType() const = 0;
};

class IFDE_CSSRule {
 public:
  virtual ~IFDE_CSSRule() {}
  virtual FDE_CSSRuleType GetType() const = 0;
};

struct FDE_CSSLENGTH {
  FDE_CSSLENGTH() {}

  explicit FDE_CSSLENGTH(FDE_CSSLengthUnit eUnit) : m_unit(eUnit) {}

  FDE_CSSLENGTH(FDE_CSSLengthUnit eUnit, FX_FLOAT fValue)
      : m_unit(eUnit), m_fValue(fValue) {}

  FDE_CSSLENGTH& Set(FDE_CSSLengthUnit eUnit) {
    m_unit = eUnit;
    return *this;
  }

  FDE_CSSLENGTH& Set(FDE_CSSLengthUnit eUnit, FX_FLOAT fValue) {
    m_unit = eUnit;
    m_fValue = fValue;
    return *this;
  }

  FDE_CSSLengthUnit GetUnit() const { return m_unit; }

  FX_FLOAT GetValue() const { return m_fValue; }
  bool NonZero() const { return static_cast<int>(m_fValue) != 0; }

 private:
  FDE_CSSLengthUnit m_unit;
  FX_FLOAT m_fValue;
};

struct FDE_CSSRECT {
  FDE_CSSRECT() {}

  FDE_CSSRECT(FDE_CSSLengthUnit eUnit, FX_FLOAT val)
      : left(eUnit, val),
        top(eUnit, val),
        right(eUnit, val),
        bottom(eUnit, val) {}

  FDE_CSSRECT& Set(FDE_CSSLengthUnit eUnit) {
    left.Set(eUnit);
    top.Set(eUnit);
    right.Set(eUnit);
    bottom.Set(eUnit);
    return *this;
  }
  FDE_CSSRECT& Set(FDE_CSSLengthUnit eUnit, FX_FLOAT fValue) {
    left.Set(eUnit, fValue);
    top.Set(eUnit, fValue);
    right.Set(eUnit, fValue);
    bottom.Set(eUnit, fValue);
    return *this;
  }

  FDE_CSSLENGTH left, top, right, bottom;
};

#endif  // XFA_FDE_CSS_FDE_CSS_H_
