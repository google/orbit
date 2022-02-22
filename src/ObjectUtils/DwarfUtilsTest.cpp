// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ObjectUtils/DwarfUtils.h"
#include "OrbitBase/Logging.h"

namespace {

struct FakeDWARFDie {
  [[nodiscard]] bool isValid() const { return is_valid_; }
  [[nodiscard]] std::vector<FakeDWARFDie> children() const { return children_; }
  [[nodiscard]] const char* getName(llvm::DINameKind /*unused*/) const { return name_; }
  [[nodiscard]] llvm::dwarf::Tag getTag() const { return tag_; }
  [[nodiscard]] FakeDWARFDie getAttributeValueAsReferencedDie(
      llvm::dwarf::Attribute attribute) const {
    ORBIT_CHECK(attribute == llvm::dwarf::DW_AT_type);
    return *type_;
  }

  bool is_valid_ = true;
  std::vector<FakeDWARFDie> children_{};
  const char* name_{};
  llvm::dwarf::Tag tag_ = static_cast<llvm::dwarf::Tag>(llvm::dwarf::DW_TAG_invalid);
  const FakeDWARFDie* type_{};
};

const FakeDWARFDie kBaseTypeDie{
    /*is_valid=*/true,
    /*children=*/{},
    /*name=*/"int",
    /*tag=*/llvm::dwarf::DW_TAG_base_type,
    /*type=*/{},
};

const FakeDWARFDie kUserSpecifiedType{
    /*is_valid=*/true,
    /*children=*/{},
    /*name=*/"Foo",
    /*tag=*/llvm::dwarf::DW_TAG_class_type,
    /*type=*/{},
};

}  // namespace

namespace orbit_object_utils {

TEST(DwarfTypeAsString, DiesIfInvalid) {
  FakeDWARFDie invalid_die;
  invalid_die.is_valid_ = false;

  EXPECT_DEATH((void)DwarfTypeAsString(invalid_die), "");
}

TEST(DwarfTypeAsString, ReturnsNameOnNamedTypes) {
  EXPECT_STREQ(DwarfTypeAsString(kBaseTypeDie).data(), "int");
  EXPECT_STREQ(DwarfTypeAsString(kUserSpecifiedType).data(), "Foo");
}

TEST(DwarfTypeAsString, PrependsAtomicModifier) {
  FakeDWARFDie atomic_die;
  atomic_die.tag_ = llvm::dwarf::DW_TAG_atomic_type;
  atomic_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(atomic_die).data(), "int _Atomic");
}

TEST(DwarfTypeAsString, PrependsConstModifier) {
  FakeDWARFDie const_die;
  const_die.tag_ = llvm::dwarf::DW_TAG_const_type;
  const_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(const_die).data(), "int const");
}

TEST(DwarfTypeAsString, PrependsVolatileModifier) {
  FakeDWARFDie volatile_die;
  volatile_die.tag_ = llvm::dwarf::DW_TAG_volatile_type;
  volatile_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(volatile_die).data(), "int volatile");
}

TEST(DwarfTypeAsString, PrependsRestrictModifier) {
  FakeDWARFDie restrict_die;
  restrict_die.tag_ = llvm::dwarf::DW_TAG_restrict_type;
  restrict_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(restrict_die).data(), "int restrict");
}

TEST(DwarfTypeAsString, AppendsArrayType) {
  FakeDWARFDie array_die;
  array_die.tag_ = llvm::dwarf::DW_TAG_array_type;
  array_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(array_die).data(), "int[]");
}

TEST(DwarfTypeAsString, AppendsPointerType) {
  FakeDWARFDie pointer_die;
  pointer_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  pointer_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(pointer_die).data(), "int*");
}

TEST(DwarfTypeAsString, AppendsReferenceType) {
  FakeDWARFDie reference_die;
  reference_die.tag_ = llvm::dwarf::DW_TAG_reference_type;
  reference_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(reference_die).data(), "int&");
}

TEST(DwarfTypeAsString, AppendsRValueReferenceType) {
  FakeDWARFDie rvalue_reference_die;
  rvalue_reference_die.tag_ = llvm::dwarf::DW_TAG_rvalue_reference_type;
  rvalue_reference_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(rvalue_reference_die).data(), "int&&");
}

TEST(DwarfTypeAsString, PointerToConstIntVsConstPointerToInt) {
  FakeDWARFDie pointer_die;
  pointer_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  pointer_die.type_ = &kBaseTypeDie;

  FakeDWARFDie const_die;
  const_die.tag_ = llvm::dwarf::DW_TAG_const_type;
  const_die.type_ = &pointer_die;

  EXPECT_STREQ(DwarfTypeAsString(const_die).data(), "int* const");

  // Let's switch parent/child and expect a const pointer to an int.
  pointer_die.type_ = &const_die;
  const_die.type_ = &kBaseTypeDie;

  EXPECT_STREQ(DwarfTypeAsString(pointer_die).data(), "int const*");
}

TEST(DwarfTypeAsString, OrderMatters) {
  // Example from the spec (https://dwarfstd.org/doc/DWARF5.pdf):
  // DW_TAG_volatile_type -->
  //   DW_TAG_pointer_type -->
  //     DW_TAG_const_type -->
  //       DW_TAG_base_type(unsigned char)

  FakeDWARFDie unsigned_char_die;
  unsigned_char_die.name_ = "unsigned char";
  unsigned_char_die.tag_ = llvm::dwarf::DW_TAG_base_type;

  FakeDWARFDie const_die;
  const_die.tag_ = llvm::dwarf::DW_TAG_const_type;
  const_die.type_ = &unsigned_char_die;

  FakeDWARFDie pointer_die;
  pointer_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  pointer_die.type_ = &const_die;

  FakeDWARFDie volatile_die;
  volatile_die.tag_ = llvm::dwarf::DW_TAG_volatile_type;
  volatile_die.type_ = &pointer_die;

  EXPECT_STREQ(DwarfTypeAsString(volatile_die).data(), "unsigned char const* volatile");
}

TEST(DwarfTypeAsString, OrderMattersCont) {
  // Example from the spec (https://dwarfstd.org/doc/DWARF5.pdf):
  // DW_TAG_restrict_type -->
  //   DW_TAG_const_type -->
  //     DW_TAG_pointer_type -->
  //       DW_TAG_volatile_type -->
  //         DW_TAG_base_type(unsigned char)

  FakeDWARFDie unsigned_char_die;
  unsigned_char_die.name_ = "unsigned char";
  unsigned_char_die.tag_ = llvm::dwarf::DW_TAG_base_type;

  FakeDWARFDie volatile_die;
  volatile_die.tag_ = llvm::dwarf::DW_TAG_volatile_type;
  volatile_die.type_ = &unsigned_char_die;

  FakeDWARFDie pointer_die;
  pointer_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  pointer_die.type_ = &volatile_die;

  FakeDWARFDie const_die;
  const_die.tag_ = llvm::dwarf::DW_TAG_const_type;
  const_die.type_ = &pointer_die;

  FakeDWARFDie restrict_die;
  restrict_die.tag_ = llvm::dwarf::DW_TAG_restrict_type;
  restrict_die.type_ = &const_die;

  EXPECT_STREQ(DwarfTypeAsString(restrict_die).data(), "unsigned char volatile* const restrict");
}

TEST(DwarfParameterListAsString, EmptyParameterList) {
  FakeDWARFDie empty_parameter_list_die;
  empty_parameter_list_die.children_ = {};
  empty_parameter_list_die.tag_ = llvm::dwarf::DW_TAG_subprogram;

  EXPECT_STREQ(DwarfParameterListAsString(empty_parameter_list_die).data(), "()");
}

TEST(DwarfParameterListAsString, OneBaseParameter) {
  FakeDWARFDie formal_parameter;
  formal_parameter.tag_ = llvm::dwarf::DW_TAG_formal_parameter;
  formal_parameter.type_ = &kBaseTypeDie;
  FakeDWARFDie parameter_list_die;
  parameter_list_die.children_ = {formal_parameter};
  parameter_list_die.tag_ = llvm::dwarf::DW_TAG_subprogram;

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die).data(), "(int)");
}

TEST(DwarfParameterListAsString, TwoBaseParameters) {
  FakeDWARFDie formal_parameter;
  formal_parameter.tag_ = llvm::dwarf::DW_TAG_formal_parameter;
  formal_parameter.type_ = &kBaseTypeDie;

  FakeDWARFDie parameter_list_die;
  parameter_list_die.children_ = {formal_parameter, formal_parameter};
  parameter_list_die.tag_ = llvm::dwarf::DW_TAG_subprogram;

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die).data(), "(int, int)");
}

TEST(DwarfParameterListAsString, HasCorrectOrderOfParameters) {
  FakeDWARFDie formal_parameter1;
  formal_parameter1.tag_ = llvm::dwarf::DW_TAG_formal_parameter;
  formal_parameter1.type_ = &kBaseTypeDie;

  FakeDWARFDie int_ptr_die;
  int_ptr_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  int_ptr_die.type_ = &kBaseTypeDie;
  FakeDWARFDie int_ptr_const_die;
  int_ptr_const_die.tag_ = llvm::dwarf::DW_TAG_const_type;
  int_ptr_const_die.type_ = &int_ptr_die;

  FakeDWARFDie formal_parameter2;
  formal_parameter2.tag_ = llvm::dwarf::DW_TAG_formal_parameter;
  formal_parameter2.type_ = &int_ptr_const_die;

  FakeDWARFDie foo_pointer_die;
  foo_pointer_die.tag_ = llvm::dwarf::DW_TAG_pointer_type;
  foo_pointer_die.type_ = &kUserSpecifiedType;
  FakeDWARFDie formal_parameter3;
  formal_parameter3.tag_ = llvm::dwarf::DW_TAG_formal_parameter;
  formal_parameter3.type_ = &foo_pointer_die;

  FakeDWARFDie parameter_list_die1;
  parameter_list_die1.children_ = {formal_parameter1, formal_parameter2, formal_parameter3};
  parameter_list_die1.tag_ = llvm::dwarf::DW_TAG_subprogram;

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die1).data(), "(int, int* const, Foo*)");

  FakeDWARFDie parameter_list_die2;
  parameter_list_die2.children_ = {formal_parameter3, formal_parameter1, formal_parameter2};
  parameter_list_die2.tag_ = llvm::dwarf::DW_TAG_subprogram;

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die2).data(), "(Foo*, int, int* const)");
}
}  // namespace orbit_object_utils
