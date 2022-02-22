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
    .name_ = "int",
    .tag_ = llvm::dwarf::DW_TAG_base_type,
};

const FakeDWARFDie kUserSpecifiedType{
    .name_ = "Foo",
    .tag_ = llvm::dwarf::DW_TAG_class_type,
};

}  // namespace

namespace orbit_object_utils {

TEST(DwarfTypeAsString, DiesIfInvalid) {
  FakeDWARFDie invalid_die{.is_valid_ = false};

  EXPECT_DEATH((void)DwarfTypeAsString(invalid_die), "");
}

TEST(DwarfTypeAsString, ReturnsNameOnNamedTypes) {
  EXPECT_STREQ(DwarfTypeAsString(kBaseTypeDie).data(), "int");
  EXPECT_STREQ(DwarfTypeAsString(kUserSpecifiedType).data(), "Foo");
}

TEST(DwarfTypeAsString, PrependsAtomicModifier) {
  FakeDWARFDie atomic_die{
      .tag_ = llvm::dwarf::DW_TAG_atomic_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(atomic_die).data(), "int _Atomic");
}

TEST(DwarfTypeAsString, PrependsConstModifier) {
  FakeDWARFDie const_die{
      .tag_ = llvm::dwarf::DW_TAG_const_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(const_die).data(), "int const");
}

TEST(DwarfTypeAsString, PrependsVolatileModifier) {
  FakeDWARFDie volatile_die{
      .tag_ = llvm::dwarf::DW_TAG_volatile_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(volatile_die).data(), "int volatile");
}

TEST(DwarfTypeAsString, PrependsRestrictModifier) {
  FakeDWARFDie restrict_die{
      .tag_ = llvm::dwarf::DW_TAG_restrict_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(restrict_die).data(), "int restrict");
}

TEST(DwarfTypeAsString, AppendsArrayType) {
  FakeDWARFDie array_die{
      .tag_ = llvm::dwarf::DW_TAG_array_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(array_die).data(), "int[]");
}

TEST(DwarfTypeAsString, AppendsPointerType) {
  FakeDWARFDie pointer_die{
      .tag_ = llvm::dwarf::DW_TAG_pointer_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(pointer_die).data(), "int*");
}

TEST(DwarfTypeAsString, AppendsReferenceType) {
  FakeDWARFDie reference_die{
      .tag_ = llvm::dwarf::DW_TAG_reference_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(reference_die).data(), "int&");
}

TEST(DwarfTypeAsString, AppendsRValueReferenceType) {
  FakeDWARFDie rvalue_reference_die{
      .tag_ = llvm::dwarf::DW_TAG_rvalue_reference_type,
      .type_ = &kBaseTypeDie,
  };

  EXPECT_STREQ(DwarfTypeAsString(rvalue_reference_die).data(), "int&&");
}

TEST(DwarfTypeAsString, PointerToConstIntVsConstPointerToInt) {
  FakeDWARFDie pointer_die{
      .tag_ = llvm::dwarf::DW_TAG_pointer_type,
      .type_ = &kBaseTypeDie,
  };
  FakeDWARFDie const_die{
      .tag_ = llvm::dwarf::DW_TAG_const_type,
      .type_ = &pointer_die,
  };

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

  FakeDWARFDie unsigned_char_die{
      .name_ = "unsigned char",
      .tag_ = llvm::dwarf::DW_TAG_base_type,
  };

  FakeDWARFDie const_die{
      .tag_ = llvm::dwarf::DW_TAG_const_type,
      .type_ = &unsigned_char_die,
  };

  FakeDWARFDie pointer_die{
      .tag_ = llvm::dwarf::DW_TAG_pointer_type,
      .type_ = &const_die,
  };

  FakeDWARFDie volatile_die{
      .tag_ = llvm::dwarf::DW_TAG_volatile_type,
      .type_ = &pointer_die,
  };

  EXPECT_STREQ(DwarfTypeAsString(volatile_die).data(), "unsigned char const* volatile");
}

TEST(DwarfTypeAsString, OrderMattersCont) {
  // Example from the spec (https://dwarfstd.org/doc/DWARF5.pdf):
  // DW_TAG_restrict_type -->
  //   DW_TAG_const_type -->
  //     DW_TAG_pointer_type -->
  //       DW_TAG_volatile_type -->
  //         DW_TAG_base_type(unsigned char)

  FakeDWARFDie unsigned_char_die{
      .name_ = "unsigned char",
      .tag_ = llvm::dwarf::DW_TAG_base_type,
  };

  FakeDWARFDie volatile_die{
      .tag_ = llvm::dwarf::DW_TAG_volatile_type,
      .type_ = &unsigned_char_die,
  };

  FakeDWARFDie pointer_die{
      .tag_ = llvm::dwarf::DW_TAG_pointer_type,
      .type_ = &volatile_die,
  };

  FakeDWARFDie const_die{
      .tag_ = llvm::dwarf::DW_TAG_const_type,
      .type_ = &pointer_die,
  };

  FakeDWARFDie restrict_die{
      .tag_ = llvm::dwarf::DW_TAG_restrict_type,
      .type_ = &const_die,
  };

  EXPECT_STREQ(DwarfTypeAsString(restrict_die).data(), "unsigned char volatile* const restrict");
}

TEST(DwarfParameterListAsString, EmptyParameterList) {
  FakeDWARFDie empty_parameter_list_die{
      .children_ = {},
      .tag_ = llvm::dwarf::DW_TAG_subprogram,
  };

  EXPECT_STREQ(DwarfParameterListAsString(empty_parameter_list_die).data(), "()");
}

TEST(DwarfParameterListAsString, OneBaseParameter) {
  FakeDWARFDie formal_parameter{
      .tag_ = llvm::dwarf::DW_TAG_formal_parameter,
      .type_ = &kBaseTypeDie,
  };
  FakeDWARFDie parameter_list_die{
      .children_ = {formal_parameter},
      .tag_ = llvm::dwarf::DW_TAG_subprogram,
  };

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die).data(), "(int)");
}

TEST(DwarfParameterListAsString, TwoBaseParameters) {
  FakeDWARFDie formal_parameter{
      .tag_ = llvm::dwarf::DW_TAG_formal_parameter,
      .type_ = &kBaseTypeDie,
  };

  FakeDWARFDie parameter_list_die{
      .children_ = {formal_parameter, formal_parameter},
      .tag_ = llvm::dwarf::DW_TAG_subprogram,
  };

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die).data(), "(int, int)");
}

TEST(DwarfParameterListAsString, HasCorrectOrderOfParameters) {
  FakeDWARFDie formal_parameter1{
      .tag_ = llvm::dwarf::DW_TAG_formal_parameter,
      .type_ = &kBaseTypeDie,
  };

  FakeDWARFDie int_ptr_die{
      .tag_ = llvm::dwarf::DW_TAG_pointer_type,
      .type_ = &kBaseTypeDie,
  };
  FakeDWARFDie int_ptr_const_die{
      .tag_ = llvm::dwarf::DW_TAG_const_type,
      .type_ = &int_ptr_die,
  };
  FakeDWARFDie formal_parameter2{
      .tag_ = llvm::dwarf::DW_TAG_formal_parameter,
      .type_ = &int_ptr_const_die,
  };

  FakeDWARFDie foo_pointer_die{.tag_ = llvm::dwarf::DW_TAG_pointer_type,
                               .type_ = &kUserSpecifiedType};
  FakeDWARFDie formal_parameter3{
      .tag_ = llvm::dwarf::DW_TAG_formal_parameter,
      .type_ = &foo_pointer_die,
  };

  FakeDWARFDie parameter_list_die1{
      .children_ = {formal_parameter1, formal_parameter2, formal_parameter3},
      .tag_ = llvm::dwarf::DW_TAG_subprogram,
  };

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die1).data(), "(int, int* const, Foo*)");

  FakeDWARFDie parameter_list_die2{
      .children_ = {formal_parameter3, formal_parameter1, formal_parameter2},
      .tag_ = llvm::dwarf::DW_TAG_subprogram,
  };

  EXPECT_STREQ(DwarfParameterListAsString(parameter_list_die2).data(), "(Foo*, int, int* const)");
}
}  // namespace orbit_object_utils
