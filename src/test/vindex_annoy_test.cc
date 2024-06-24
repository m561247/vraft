#include "vindex_annoy.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "raft.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"
#include "vengine.h"

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

int32_t dim = 10;
TEST(VindexAnnoy, VindexAnnoy) {
  system("rm -rf /tmp/vindex_annoy_test_dir");

  {
    vectordb::VEngineSPtr ve =
        std::make_shared<vectordb::VEngine>("/tmp/vindex_annoy_test_dir", dim);
    std::cout << ve->ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve->Dim(), dim);

    vectordb::VIndexParam param;
    param.path = "/tmp/vindex_annoy_test_dir/index/annoy";
    param.timestamp = vraft::Clock::NSec();
    param.dim = dim;
    param.index_type = vectordb::kIndexAnnoy;
    param.distance_type = vectordb::kCosine;
    vectordb::VindexSPtr vindex =
        std::make_shared<vectordb::VindexAnnoy>(param, ve);
    assert(vindex);
  }
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}