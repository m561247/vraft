#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "spdlog/spdlog.h"
#include "vraft_logger.h"

TEST(Logger, test) {
  vraft::LoggerOptions logger_options{"vraft", false, 1, 8192,
                                      vraft::kLoggerTrace};
  logger_options.level = vraft::U8ToLevel(0);
  logger_options.enable_debug = true;
  vraft::vraft_logger.Init("./logger_test.log", logger_options);
  vraft::vraft_logger.FInfo("%s", "info log");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}