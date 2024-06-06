#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "coding.h"
#include "solid_data.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << argv[0] << " meta_path" << std::endl;
    return 0;
  }

  std::string path = argv[1];
  vraft::SolidData meta(path);
  std::cout << meta.ToJsonString(false, true) << std::endl;

  return 0;
}