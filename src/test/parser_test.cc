#include "parser.h"

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
// ASSERT_TRUE  true
// ASSERT_FALSE false
//
// EXPECT_EQ  ==
// EXPECT_NE  !=
// EXPECT_NE  <
// EXPECT_LE  <=
// EXPECT_GT  >
// EXPECT_GE  >=
//
// ASSERT_EQ  ==
// ASSERT_NE  !=
// ASSERT_LT  <
// ASSERT_LE  <=
// ASSERT_GT  >
// ASSERT_GE  >=
//--------------------------------

TEST(Parser, kCmdError) {
  vectordb::Parser parser("xx");
  ASSERT_EQ(parser.cmd(), vectordb::kCmdError);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

TEST(Parser, kCmdHelp) {
  vectordb::Parser parser(vectordb::example_cmdstr(vectordb::kCmdHelp));
  ASSERT_EQ(parser.cmd(), vectordb::kCmdHelp);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

TEST(Parser, kCmdVersion) {
  vectordb::Parser parser(vectordb::example_cmdstr(vectordb::kCmdVersion));
  ASSERT_EQ(parser.cmd(), vectordb::kCmdVersion);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

TEST(Parser, kCmdQuit) {
  vectordb::Parser parser(vectordb::example_cmdstr(vectordb::kCmdQuit));
  ASSERT_EQ(parser.cmd(), vectordb::kCmdQuit);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

TEST(Parser, kCmdMeta) {
  vectordb::Parser parser(vectordb::example_cmdstr(vectordb::kCmdMeta));
  ASSERT_EQ(parser.cmd(), vectordb::kCmdMeta);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

TEST(Parser, kCmdCreateTable) {
  vectordb::Parser parser(vectordb::example_cmdstr(vectordb::kCmdCreateTable));
  ASSERT_EQ(parser.cmd(), vectordb::kCmdCreateTable);
  std::cout << parser.ToJsonString(false, false) << std::endl;
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}