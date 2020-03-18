#include <gtest/gtest.h>

#include <string>

#include "veriblock/fmt.hpp"

TEST(Format, Works) {
  using std::string_literals::operator""s;
  std::string weekday = "Wednesday";
  const char* month = "July";
  size_t day = 27;
  long hour = 14;
  int min = 44;

  std::string actual =
      AltIntegrationLib::format("%s, %s %d, %.2d:%.2d", weekday, month, day, hour, min);

  EXPECT_EQ(actual, "Wednesday, July 27, 14:44"s);
}