// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "ApiUtils/EncodedString.h"
#include "GrpcProtos/capture.pb.h"

using orbit_grpc_protos::ApiScopeStart;

using ::testing::ElementsAre;

namespace orbit_api {

TEST(EncodedString, DecodeEmptyString) {
  std::string decoded = DecodeString(0, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);
  EXPECT_STREQ("", decoded.c_str());
}

TEST(EncodedString, DecodeEightByteString) {
  std::string decoded = DecodeString(0x8877665544332211, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);

  std::string expected{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88)};

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, DecodeFourByteString) {
  std::string decoded = DecodeString(0x0000000044332211, 0, 0, 0, 0, 0, 0, 0, nullptr, 0);

  std::string expected{0x11, 0x22, 0x33, 0x44};

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, DecodeNineByteString) {
  std::string decoded =
      DecodeString(0x8877665544332211, 0x0000000000000099, 0, 0, 0, 0, 0, 0, nullptr, 0);

  std::string expected{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88), static_cast<char>(0x99)};

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode64ByteString) {
  std::string decoded = DecodeString(0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                                     0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                                     0x8877665544332211, 0x8877665544332211, nullptr, 0);
  std::string expected{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
  };

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode65ByteStringIncludingOneAdditionalCharacter) {
  std::vector<uint64_t> additional_chunks;
  additional_chunks.push_back(0x0000000000000011);
  std::string decoded =
      DecodeString(0x8877665544332211, 0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                   0x8877665544332211, 0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                   additional_chunks.data(), additional_chunks.size());
  std::string expected{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
                       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
                       0x11};

  EXPECT_EQ(expected, decoded);
}

TEST(EncodedString, Decode80ByteStringIncluding16AdditionalCharacters) {
  std::vector<uint64_t> additional_chunks;
  additional_chunks.push_back(0x8877665544332211);
  additional_chunks.push_back(0x8877665544332211);
  std::string decoded =
      DecodeString(0x8877665544332211, 0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                   0x8877665544332211, 0x8877665544332211, 0x8877665544332211, 0x8877665544332211,
                   additional_chunks.data(), additional_chunks.size());
  std::string expected{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 9
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 10
  };

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
  std::string decoded_string{0x11};
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x0000000000000011);
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
  std::string decoded_string{0x11, 0x22, 0x33, 0x44};
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x0000000044332211);
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
  std::string decoded_string{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88)};
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x8877665544332211);
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
  std::string decoded_string{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88), static_cast<char>(0x99)};
  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x0000000000000099);
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
  std::string decoded_string{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
  };

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x8877665544332211);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre());
}

TEST(EncodedString, Encode65ByteStringWithIncludingOneAdditionalByte) {
  ApiScopeStart encoded_string;
  std::string decoded_string{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
      0x11,
  };

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x8877665544332211);
  EXPECT_THAT(encoded_string.encoded_name_additional(), ElementsAre(0x0000000000000011));
}

TEST(EncodedString, Encode80ByteStringWithIncludingOneAdditionalByte) {
  ApiScopeStart encoded_string;
  // clang-format off
  std::string decoded_string{
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 1
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 2
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 3
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 4
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 5
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 6
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 7
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 8
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, static_cast<char>(0x88),  // 9
      static_cast<char>(0x88), 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,  // 10
  };
// clang-format off

  EncodeString(decoded_string.c_str(), &encoded_string);
  EXPECT_EQ(encoded_string.encoded_name_1(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_2(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_3(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_4(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_5(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_6(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_7(), 0x8877665544332211);
  EXPECT_EQ(encoded_string.encoded_name_8(), 0x8877665544332211);
  EXPECT_THAT(encoded_string.encoded_name_additional(),
              ElementsAre(0x8877665544332211, 0x1122334455667788));
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
      "na na na na na na na na na na na na na na na na na na na na na na na na na na na na na na "
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