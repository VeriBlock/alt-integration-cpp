#include <gtest/gtest.h>
#include <vector>
#include <veriblock/base58.h>


TEST(base58, Encode)
{
     //  test data
     std::vector<uint8_t> inputA = { 27,53,84,6,78,51,29,38,57,2,0,95,23,89,4,6,7,92,35,61,30,95,12,96,29 };
     std::vector<uint8_t> inputB = { 2,9,5,7,9,3,4,5,7,3,8,9,4,6,7,3,7,8,9,6,7,9,5,7,2,9,8,5,2,9,8,5,7,2 };
     std::vector<uint8_t> inputC = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
     std::vector<uint8_t> inputD = { };
     //  correct output
     std::string correctOutputA = "BwzG4EVMJLbJdA9WPgQW2hxeFB9v31nYFE";
     std::string correctOutputB = "3fmxeMiA19wm9A3ReBdfMQRQSQ6nkXWnBWHRM83DZMukjK";
     std::string correctOutputC = "1111111111111111111111111111111111";
     std::string correctOutputD = "";

     EXPECT_EQ(Veriblock::base58_encode(inputA.data(), inputA.size()), correctOutputA);
     EXPECT_EQ(Veriblock::base58_encode(inputB.data(), inputB.size()), correctOutputB);
     EXPECT_EQ(Veriblock::base58_encode(inputC.data(), inputC.size()), correctOutputC);
     EXPECT_EQ(Veriblock::base58_encode(inputD.data(), inputD.size()), correctOutputD);
}

TEST(base58, Decode)
{
    //  test data
    std::string inputA = "BwzG4EVMJLbJdA9WPgQW2hxeFB9v31nYFE";
    std::string inputB = "3fmxeMiA19wm9A3ReBdfMQRQSQ6nkXWnBWHRM83DZMukjK";
    std::string inputC = "1111111111111111111111111111111111";
    std::string inputD = "";

    //  correct output
    std::vector<uint8_t> correctOutputA = { 27,53,84,6,78,51,29,38,57,2,0,95,23,89,4,6,7,92,35,61,30,95,12,96,29 };
    std::vector<uint8_t> correctOutputB = { 2,9,5,7,9,3,4,5,7,3,8,9,4,6,7,3,7,8,9,6,7,9,5,7,2,9,8,5,2,9,8,5,7,2 };
    std::vector<uint8_t> correctOutputC = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    std::vector<uint8_t> correctOutputD = {};

    EXPECT_EQ(Veriblock::base58_decode(inputA), correctOutputA);
    EXPECT_EQ(Veriblock::base58_decode(inputB), correctOutputB);
    EXPECT_EQ(Veriblock::base58_decode(inputC), correctOutputC);
    EXPECT_EQ(Veriblock::base58_decode(inputD), correctOutputD);
    
}