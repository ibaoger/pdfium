// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_PATHDATA_H_
#define CORE_FXGE_CFX_PATHDATA_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_renderdevice.h"

struct FX_PATHPOINT {
  bool IsTypeAndOpen(FXPT_TYPE type) const {
    return m_Type == type && !m_CloseFigure;
  }

  FX_FLOAT m_PointX;
  FX_FLOAT m_PointY;
  FXPT_TYPE m_Type;
  bool m_CloseFigure;
};

class CFX_PathData {
 public:
  CFX_PathData();
  CFX_PathData(const CFX_PathData& src);
  ~CFX_PathData();

  void Clear();

  FXPT_TYPE GetType(int index) const { return m_Points[index].m_Type; }
  bool IsClosingFigure(int index) const {
    return m_Points[index].m_CloseFigure;
  }

  FX_FLOAT GetPointX(int index) const { return m_Points[index].m_PointX; }
  FX_FLOAT GetPointY(int index) const { return m_Points[index].m_PointY; }
  const std::vector<FX_PATHPOINT>& GetPoints() const { return m_Points; }
  std::vector<FX_PATHPOINT>& GetPoints() { return m_Points; }

  CFX_FloatRect GetBoundingBox() const;
  CFX_FloatRect GetBoundingBox(FX_FLOAT line_width, FX_FLOAT miter_limit) const;

  void Transform(const CFX_Matrix* pMatrix);
  bool IsRect() const;
  bool GetZeroAreaPath(CFX_PathData* NewPath,
                       CFX_Matrix* pMatrix,
                       bool& bThin,
                       bool bAdjust) const;
  bool IsRect(const CFX_Matrix* pMatrix, CFX_FloatRect* rect) const;

  void Append(const CFX_PathData& data);
  void Append(const CFX_PathData* pSrc, const CFX_Matrix* pMatrix);
  void AppendRect(FX_FLOAT left, FX_FLOAT bottom, FX_FLOAT right, FX_FLOAT top);
  void AppendPoint(FX_FLOAT x, FX_FLOAT y, FXPT_TYPE type, bool closeFigure);
  void ClosePath();

 private:
  std::vector<FX_PATHPOINT> m_Points;
};

#endif  // CORE_FXGE_CFX_PATHDATA_H_
