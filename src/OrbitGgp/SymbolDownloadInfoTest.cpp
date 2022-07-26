// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QByteArray>
#include <QString>

#include "OrbitBase/Result.h"
#include "OrbitGgp/SymbolDownloadInfo.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

TEST(OrbitGgpSymbolDownloadInfoTest, GetListFromJsonError) {
  std::vector<QByteArray> test_cases = {// Empty json
                                        QByteArray("{}"),
                                        // Invalid json
                                        QByteArray("json"),
                                        // One empty json object
                                        QByteArray(R"(
{
 "symbols": [{}]
} 
)"),
                                        // One partial (invalid) element
                                        QByteArray(R"(
{
 "symbols": [
  {
   "downloadUrl": "valid_url_for_symbol_0",
  }
 ]
} 
)"),
                                        // One valid and one invalid element
                                        QByteArray(R"(
{
 "symbols": [
  {
   "downloadUrl": "valid_url_for_symbol_0",
   "fileId": "symbolFiles/build_id_0/symbol_filename_0"
  },
  {
   "downloadUrl": "valid_url_for_symbol_1",
   "wrong id identifier": "symbolFiles/build_id_1/symbol_filename_1"
  }
 ]
}       
)")};

  for (const auto& json : test_cases) {
    EXPECT_THAT(SymbolDownloadInfo::GetListFromJson(json),
                orbit_test_utils::HasError("Unable to parse JSON"));
  }
}

TEST(OrbitGgpSymbolDownloadInfoTest, GetListFromJsonSuccess) {
  // Two valid elements
  const auto json = QByteArray(R"(
{
 "symbols": [
  {
   "downloadUrl": "valid_url_for_symbol_0",
   "fileId": "symbolFiles/build_id_0/symbol_filename_0"
  },
  {
   "downloadUrl": "valid_url_for_symbol_1",
   "fileId": "symbolFiles/build_id_1/symbol_filename_1"
  }
 ]
}
)");

  const auto result = SymbolDownloadInfo::GetListFromJson(json);
  ASSERT_THAT(result, orbit_test_utils::HasNoError());
  const std::vector<SymbolDownloadInfo>& symbols{result.value()};
  ASSERT_EQ(symbols.size(), 2);
  EXPECT_EQ(symbols[0].file_id, "symbolFiles/build_id_0/symbol_filename_0");
  EXPECT_EQ(symbols[0].url, "valid_url_for_symbol_0");
  EXPECT_EQ(symbols[1].file_id, "symbolFiles/build_id_1/symbol_filename_1");
  EXPECT_EQ(symbols[1].url, "valid_url_for_symbol_1");
}

TEST(OrbitGgpSymbolDownloadInfoTest, EqualToOperator) {
  SymbolDownloadInfo symbol_0;
  SymbolDownloadInfo symbol_1;
  EXPECT_TRUE(symbol_0 == symbol_1);

  symbol_0.file_id = "a_symbol_filename";
  symbol_0.url = "valid_url_for_symbol";
  EXPECT_FALSE(symbol_0 == symbol_1);

  symbol_1.file_id = "a_symbol_filename";
  symbol_1.url = "valid_url_for_symbol";
  EXPECT_TRUE(symbol_0 == symbol_1);
}

TEST(OrbitGgpSymbolDownloadInfoTest, NotEqualToOperator) {
  SymbolDownloadInfo symbol_0;
  SymbolDownloadInfo symbol_1;
  EXPECT_FALSE(symbol_0 != symbol_1);

  symbol_0.file_id = "a_symbol_filename";
  symbol_0.url = "valid_url_for_symbol";
  EXPECT_TRUE(symbol_0 != symbol_1);

  symbol_1.file_id = "a_symbol_filename";
  symbol_1.url = "valid_url_for_symbol";
  EXPECT_FALSE(symbol_0 != symbol_1);
}

}  // namespace orbit_ggp
