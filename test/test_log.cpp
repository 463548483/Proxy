#include "gtest/gtest.h"
#include "log.h"

TEST(logtest, test0) {
  Logger log;
  log.open("test_log.txt");
  log << "test";
  log << "test again";
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv); 
  return RUN_ALL_TESTS();
}
