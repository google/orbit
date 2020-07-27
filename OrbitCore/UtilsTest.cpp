#include <gtest/gtest.h>

#include "Utils.h"

TEST(Utils, TestEllipsis) {
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 18), "17 char long text");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 17), "17 char long text");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 7), "17...xt");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 6), "1...xt");
  EXPECT_EQ(ShortenStringWithEllipsis("short", 4), "...t");
  EXPECT_EQ(ShortenStringWithEllipsis("short", 3), "...");
  EXPECT_EQ(ShortenStringWithEllipsis("sh", 2), "sh");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 2), "...");
  EXPECT_EQ(ShortenStringWithEllipsis("sh", 1), "sh");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", -1), "17 char long text");
}

