#include "vengine.h"

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

int32_t dim = 10;

TEST(Vec, Vec) {
  vectordb::Vec v;
  for (int32_t i = 0; i < dim; ++i) {
    float f32 = vraft::RandomFloat(1);
    v.data.push_back(f32);
  }

  std::string str;
  int32_t bytes = v.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << v.ToJsonString(false, true) << std::endl;

  vectordb::Vec v2;
  int32_t bytes2 = v2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << v2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(v.data.size(), v2.data.size());
  for (size_t i = 0; i < v.data.size(); ++i) {
    ASSERT_EQ(v.data[i], v2.data[i]);
  }
}

TEST(VecValue, VecValue) {
  vectordb::VecValue vo;
  for (int32_t i = 0; i < dim; ++i) {
    float f32 = vraft::RandomFloat(1);
    vo.vec.data.push_back(f32);
  }
  vo.attach_value = "attach_value.jpg";

  std::string str;
  int32_t bytes = vo.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << vo.ToJsonString(false, true) << std::endl;

  vectordb::VecValue vo2;
  int32_t bytes2 = vo2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << vo2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(vo.vec.data.size(), vo2.vec.data.size());
  for (size_t i = 0; i < vo.vec.data.size(); ++i) {
    ASSERT_EQ(vo.vec.data[i], vo2.vec.data[i]);
  }
  ASSERT_EQ(vo.attach_value, vo2.attach_value);
}

TEST(VecObj, VecObj) {
  vectordb::VecObj vo;
  vo.key = "key";
  for (int32_t i = 0; i < dim; ++i) {
    float f32 = vraft::RandomFloat(1);
    vo.vv.vec.data.push_back(f32);
  }
  vo.vv.attach_value = "attach_value.jpg";

  std::string str;
  int32_t bytes = vo.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << vo.ToJsonString(false, true) << std::endl;

  vectordb::VecObj vo2;
  int32_t bytes2 = vo2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << vo2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(vo.key, vo2.key);
  ASSERT_EQ(vo.vv.vec.data.size(), vo2.vv.vec.data.size());
  for (size_t i = 0; i < vo.vv.vec.data.size(); ++i) {
    ASSERT_EQ(vo.vv.vec.data[i], vo2.vv.vec.data[i]);
  }
  ASSERT_EQ(vo.vv.attach_value, vo2.vv.attach_value);
}

TEST(VEngine, VEngine) {
  system("rm -rf /tmp/vengine_test_dir");

  {
    vectordb::VEngine ve("/tmp/vengine_test_dir", dim);
    std::cout << ve.ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve.Dim(), dim);
  }

  {
    vectordb::VEngine ve("/tmp/vengine_test_dir", dim + 99);
    std::cout << ve.ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve.Dim(), dim);
  }
}

TEST(VEngine, OP) {
  system("rm -rf /tmp/vengine_test_dir");
  std::string key = "kkk";

  {
    vectordb::VEngine ve("/tmp/vengine_test_dir", dim);
    std::cout << ve.ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve.Dim(), dim);

    vectordb::VecValue vv;
    for (int32_t i = 0; i < dim; ++i) {
      float f32 = vraft::RandomFloat(1);
      vv.vec.data.push_back(f32);
    }
    vv.attach_value = "aaavvv";
    int32_t rv = ve.Put(key, vv);
    ASSERT_EQ(rv, 0);
    std::cout << "put: " << vv.ToJsonString(false, true) << std::endl;

    vectordb::VecObj vo;
    rv = ve.Get(key, vo);
    ASSERT_EQ(rv, 0);
    std::cout << "get: " << vo.ToJsonString(false, true) << std::endl;

    ASSERT_EQ(vo.key, key);
    ASSERT_EQ(vo.vv.vec.data.size(), vv.vec.data.size());
    for (size_t i = 0; i < vo.vv.vec.data.size(); ++i) {
      ASSERT_EQ(vo.vv.vec.data[i], vv.vec.data[i]);
    }
    ASSERT_EQ(vo.vv.attach_value, vv.attach_value);
  }

  {
    vectordb::VEngine ve("/tmp/vengine_test_dir", dim + 99);
    std::cout << ve.ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve.Dim(), dim);

    vectordb::VecObj vo;
    int32_t rv = ve.Get(key, vo);
    ASSERT_EQ(rv, 0);
    std::cout << "get: " << vo.ToJsonString(false, true) << std::endl;

    rv = ve.Delete(key);
    ASSERT_EQ(rv, 0);

    rv = ve.Get(key, vo);
    ASSERT_EQ(rv, -2);
  }
}

TEST(VEngine, Load) {
  system("rm -rf /tmp/vengine_test_dir");

  {
    vectordb::VEngine ve("/tmp/vengine_test_dir", dim);
    std::cout << ve.ToJsonString(true, true) << std::endl;
    ASSERT_EQ(ve.Dim(), dim);

    int32_t rv = ve.Load("/tmp/vec.txt");
    ASSERT_EQ(rv, 0);

    vectordb::VecObj vo;
    rv = ve.Get("key_0", vo);
    ASSERT_EQ(rv, 0);
    std::cout << "get: " << vo.ToJsonString(false, true) << std::endl;
  }
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}