#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "raft.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"

//--------------------------------
// EXPECT_TRUE  true
// EXPECT_FALSE false
//
// EXPECT_EQ  ==
// EXPECT_NE  !=
// EXPECT_NE  <
// EXPECT_LE  <=
// EXPECT_GT  >
// EXPECT_GE  >=
//--------------------------------

TEST(TPL, tpl) {
  EXPECT_TRUE(true);
  EXPECT_FALSE(false);
  EXPECT_EQ(1, 1);
  EXPECT_NE(1, 2);
  EXPECT_NE(1, 2);
  EXPECT_LE(2, 2);
  EXPECT_GT(3, 1);
  EXPECT_GE(3, 3);
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}