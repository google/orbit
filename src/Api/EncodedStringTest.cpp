// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include "capture.pb.h"
#include "include/Api/EncodedString.h"
using orbit_grpc_protos::ApiScopeStart;

using ::testing::ElementsAre;

namespace orbit_api {

TEST(EncodedString, DecodeEmptyString) {
  std::string decoded = DecodeString(0, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);
  EXPECT_STREQ("", decoded.c_str());
}

TEST(EncodedString, DecodeEightByteString) {
  std::string decoded = DecodeString(0x1122334455667788, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, DecodeFourByteString) {
  std::string decoded = DecodeString(0x1122334400000000, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, DecodeNineByteString) {
  std::string decoded =
      DecodeString(0x1122334455667788, 0x9900000000000000, 0, 0, 0, 0, 0, 0, nullptr, 0);
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x99);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode64ByteString) {
  std::string decoded = DecodeString(0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                                     0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                                     0x1122334455667788, 0x1122334455667788, nullptr, 0);
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode65ByteStringIncludingOneAdditionalCharacter) {
  std::vector<uint64_t> additional_char;
  additional_char.push_back(0x1100000000000000);
  std::string decoded =
      DecodeString(0x1122334455667788, 0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                   0x1122334455667788, 0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                   additional_char.data(), additional_char.size());
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode80ByteStringIncluding16AdditionalCharacters) {
  std::vector<uint64_t> additional_char;
  additional_char.push_back(0x1122334455667788);
  additional_char.push_back(0x1122334455667788);
  std::string decoded =
      DecodeString(0x1122334455667788, 0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                   0x1122334455667788, 0x1122334455667788, 0x1122334455667788, 0x1122334455667788,
                   additional_char.data(), additional_char.size());
  std::string expected;
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);
  expected.push_back(0x11);
  expected.push_back(0x22);
  expected.push_back(0x33);
  expected.push_back(0x44);
  expected.push_back(0x55);
  expected.push_back(0x66);
  expected.push_back(0x77);
  expected.push_back(0x88);

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, EncodeEmptyString) {
  ApiScopeStart encoded_string;
  EncodeString("", &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode1ByteString) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1100000000000000);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode4ByteString) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334400000000);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode8ByteString) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode9ByteString) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);
  decoded_string.push_back(0x99);
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x9900000000000000);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode64ByteString) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x1122334455667788);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode65ByteStringWithIncludingOneAdditionalByte) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x1122334455667788);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre(0x1100000000000000));
}

TEST(EncodedString, Encode80ByteStringWithIncludingOneAdditionalByte) {
  ApiScopeStart encoded_string;
  std::string decoded_string;
  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x11);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x88);

  decoded_string.push_back(0x88);
  decoded_string.push_back(0x77);
  decoded_string.push_back(0x66);
  decoded_string.push_back(0x55);
  decoded_string.push_back(0x44);
  decoded_string.push_back(0x33);
  decoded_string.push_back(0x22);
  decoded_string.push_back(0x11);

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x1122334455667788);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x1122334455667788);
  EXPECT_THAT(encoded_string.encoded_name_additional(),
              ElementsAre(0x1122334455667788, 0x8877665544332211));
}

TEST(EncodedString, SmokeSmallString) {
  std::string expected_string = "Some short string";
  ApiScopeStart encoded_string;
  EncodeString(expected_string.c_str(), &encoded_string);
  std::string decoded_string =
      DecodeString(encoded_string.encoded_name_1(), encoded_string.encoded_name_2(),
                   encoded_string.encoded_name_3(), encoded_string.encoded_name_4(),
                   encoded_string.encoded_name_5(), encoded_string.encoded_name_6(),
                   encoded_string.encoded_name_7(), encoded_string.encoded_name_8(),
                   encoded_string.encoded_name_additional().data(),
                   encoded_string.encoded_name_additional_size());

  EXPECT_EQ(expected_string, decoded_string);
}

TEST(EncodedString, SmokeLargeString) {
  std::string expected_string =
      "na na na na na na na na na na na na na na na na na na na na na na na na na na na na na na "
      "na na na na na na na na na nana na na na na na na na na na na na na na na na na na na na "
      "BATMAN!";
  ApiScopeStart encoded_string;
  EncodeString(expected_string.c_str(), &encoded_string);
  std::string decoded_string =
      DecodeString(encoded_string.encoded_name_1(), encoded_string.encoded_name_2(),
                   encoded_string.encoded_name_3(), encoded_string.encoded_name_4(),
                   encoded_string.encoded_name_5(), encoded_string.encoded_name_6(),
                   encoded_string.encoded_name_7(), encoded_string.encoded_name_8(),
                   encoded_string.encoded_name_additional().data(),
                   encoded_string.encoded_name_additional_size());

  EXPECT_EQ(expected_string, decoded_string);
}

}  // namespace orbit_api