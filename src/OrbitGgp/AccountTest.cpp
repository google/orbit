// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitGgp/Account.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(OrbitGgpProject, GetDefaultAccountFromJson) {
  {  // invalid json
    const auto json = QString("json").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json), HasError("Unable to parse JSON"));
  }

  {  // empty array
    const auto json = QString("[]").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json),
                HasError("Failed to find default ggp account."));
  }

  {  // not an object in array
    const auto json = QString("[5]").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json),
                HasError("Unable to parse JSON: Object expected."));
  }

  {  // object does not contain default key
    const auto json = QString("[{}]").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json),
                HasError("Unable to parse JSON: \"default\" key missing."));
  }

  {  // object does not contain account key
    const auto json = QString(R"([{"default": "yes"}])").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json),
                HasError("Unable to parse JSON: \"account\" key missing."));
  }

  {  // object does not contain a default account
    const auto json = QString(R"([{"default": "no", "account": "username@email.com"}])").toUtf8();
    EXPECT_THAT(Account::GetDefaultAccountFromJson(json),
                HasError("Failed to find default ggp account."));
  }

  {  // object does contain a default account
    const auto json = QString(R"([{"default": "yes", "account": "username@email.com"}])").toUtf8();
    const auto result = Account::GetDefaultAccountFromJson(json);
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value().email, "username@email.com");
  }

  {  // object does contain mutliple accounts
    const auto json =
        QString(
            R"([{"default": "no", "account": "wrongaccount@email.com"},{"default": "yes", "account": "username@email.com"}])")
            .toUtf8();
    const auto result = Account::GetDefaultAccountFromJson(json);
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value().email, "username@email.com");
  }
}

}  // namespace orbit_ggp