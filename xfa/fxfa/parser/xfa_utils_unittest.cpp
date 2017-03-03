// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/xfa_utils.h"

#include <memory>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(XfaUtilsImp, XFA_MapRotation) {
  struct TestCase {
    int input;
    int expected_output;
  } TestCases[] = {{-1000000, 80}, {-361, 359}, {-360, 0},  {-359, 1},
                   {-91, 269},     {-90, 270},  {-89, 271}, {-1, 359},
                   {0, 0},         {1, 1},      {89, 89},   {90, 90},
                   {91, 91},       {359, 359},  {360, 0},   {361, 1},
                   {100000, 280}};

  for (size_t i = 0; i < FX_ArraySize(TestCases); ++i) {
    EXPECT_EQ(TestCases[i].expected_output,
              XFA_MapRotation(TestCases[i].input));
  }
}

class XFANodeIteratorTest : public testing::Test {
 public:
  class Node {
   public:
    class Strategy {
     public:
      static Node* GetFirstChild(Node* pNode, void* pUserData = nullptr) {
        return pNode && !pNode->children_.empty() ? pNode->children_.front()
                                                  : nullptr;
      }
      static Node* GetNextSibling(Node* pNode, void* pUserData = nullptr) {
        return pNode ? pNode->next_sibling_ : nullptr;
      }
      static Node* GetParent(Node* pNode, void* pUserData = nullptr) {
        return pNode ? pNode->parent_ : nullptr;
      }
    };

    explicit Node(Node* parent) : parent_(parent), next_sibling_(nullptr) {
      if (parent) {
        if (!parent->children_.empty())
          parent->children_.back()->next_sibling_ = this;
        parent->children_.push_back(this);
      }
    }

   private:
    Node* parent_;
    Node* next_sibling_;
    std::vector<Node*> children_;
  };

  using Iterator = CXFA_NodeIteratorTemplate<Node, Node::Strategy>;

  void SetUp() override {
    root_ = pdfium::MakeUnique<Node>(nullptr);
    child1_ = pdfium::MakeUnique<Node>(root_.get());
    child2_ = pdfium::MakeUnique<Node>(root_.get());
    child3_ = pdfium::MakeUnique<Node>(child2_.get());
    child4_ = pdfium::MakeUnique<Node>(child2_.get());
    child5_ = pdfium::MakeUnique<Node>(child2_.get());
    child6_ = pdfium::MakeUnique<Node>(child3_.get());
  }

  Node* root() const { return root_.get(); }
  Node* child1() const { return child1_.get(); }
  Node* child2() const { return child2_.get(); }
  Node* child3() const { return child3_.get(); }
  Node* child4() const { return child4_.get(); }
  Node* child5() const { return child5_.get(); }
  Node* child6() const { return child6_.get(); }

 protected:
  std::unique_ptr<Node> root_;
  std::unique_ptr<Node> child1_;  // Parent root.
  std::unique_ptr<Node> child2_;  // Parent root.
  std::unique_ptr<Node> child3_;  // Parent child2.
  std::unique_ptr<Node> child4_;  // Parent child2.
  std::unique_ptr<Node> child5_;  // Parent child2.
  std::unique_ptr<Node> child6_;  // Parent child3.
};

TEST_F(XFANodeIteratorTest, Empty) {
  Iterator iter(nullptr);
  EXPECT_EQ(nullptr, iter.GetRoot());
  EXPECT_EQ(nullptr, iter.GetCurrent());
  EXPECT_EQ(nullptr, iter.MoveToNext());
  EXPECT_EQ(nullptr, iter.MoveToPrev());
  EXPECT_EQ(nullptr, iter.SkipChildrenAndMoveToNext());
}

TEST_F(XFANodeIteratorTest, GetRootAndCurrent) {
  Iterator iter(root());
  iter.SetCurrent(child1());
  EXPECT_EQ(root(), iter.GetRoot());
  EXPECT_EQ(child1(), iter.GetCurrent());
}

TEST_F(XFANodeIteratorTest, MoveToPrev) {
  Iterator iter(root());
  iter.SetCurrent(child6());
  EXPECT_EQ(child3(), iter.MoveToPrev());
  EXPECT_EQ(child2(), iter.MoveToPrev());
  EXPECT_EQ(child1(), iter.MoveToPrev());
  EXPECT_EQ(root(), iter.MoveToPrev());
  EXPECT_EQ(nullptr, iter.MoveToPrev());
}

TEST_F(XFANodeIteratorTest, MoveToNext) {
  Iterator iter(root());
  iter.SetCurrent(child2());
  EXPECT_EQ(child3(), iter.MoveToNext());
  EXPECT_EQ(child6(), iter.MoveToNext());
  EXPECT_EQ(child4(), iter.MoveToNext());
  EXPECT_EQ(child5(), iter.MoveToNext());
  EXPECT_EQ(nullptr, iter.MoveToNext());
}

TEST_F(XFANodeIteratorTest, SkipChildrenAndMoveToNext) {
  Iterator iter(root());
  iter.SetCurrent(child3());
  EXPECT_EQ(child4(), iter.SkipChildrenAndMoveToNext());
  EXPECT_EQ(child5(), iter.SkipChildrenAndMoveToNext());
  EXPECT_EQ(nullptr, iter.SkipChildrenAndMoveToNext());
}

TEST_F(XFANodeIteratorTest, BackAndForth) {
  Iterator iter(root());
  iter.SetCurrent(child2());
  EXPECT_EQ(child3(), iter.MoveToNext());
  EXPECT_EQ(child6(), iter.MoveToNext());
  EXPECT_EQ(child4(), iter.MoveToNext());
  EXPECT_EQ(child6(), iter.MoveToPrev());
  EXPECT_EQ(child3(), iter.MoveToPrev());
  EXPECT_EQ(child2(), iter.MoveToPrev());
  EXPECT_EQ(child1(), iter.MoveToPrev());
  EXPECT_EQ(child2(), iter.MoveToNext());
}
