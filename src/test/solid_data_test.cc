#include "coding.h"
#include "solid_data.h"
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>

TEST(SolidData, construct) {
  system("rm -rf /tmp/soliddata_test_dir");
  vraft::SolidData solid_data("/tmp/soliddata_test_dir");
  std::cout << "term: " << solid_data.term() << std::endl;
  std::cout << "vote: " << solid_data.vote() << std::endl;
  EXPECT_EQ(solid_data.term(), 0);
  EXPECT_EQ(solid_data.vote(), 0);
}

TEST(SolidData, persist) {
  system("rm -rf /tmp/soliddata_test_dir");

  {
    vraft::SolidData solid_data("/tmp/soliddata_test_dir");
    std::cout << "term: " << solid_data.term() << std::endl;
    std::cout << "vote: " << solid_data.vote() << std::endl;
    EXPECT_EQ(solid_data.term(), 0);
    EXPECT_EQ(solid_data.vote(), 0);

    solid_data.SetTerm(3);
    EXPECT_EQ(solid_data.term(), 3);
    std::cout << "term: " << solid_data.term() << std::endl;

    solid_data.IncrTerm();
    EXPECT_EQ(solid_data.term(), 4);
    std::cout << "term: " << solid_data.term() << std::endl;

    solid_data.SetVote(7);
    EXPECT_EQ(solid_data.vote(), 7);
    std::cout << "vote: " << solid_data.vote() << std::endl;
  }

  {
    vraft::SolidData solid_data("/tmp/soliddata_test_dir");
    EXPECT_EQ(solid_data.term(), 4);
    EXPECT_EQ(solid_data.vote(), 7);

    std::cout << "term: " << solid_data.term() << std::endl;
    std::cout << "vote: " << solid_data.vote() << std::endl;
  }
}

int main(int argc, char **argv) {
  vraft::CodingInit();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}