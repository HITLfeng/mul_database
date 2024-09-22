#include "test_common.h"

class ClientTest : public ::testing::Test
{
public:
    void SetUP()
    {
        std::cout << "SetUP" << std::endl;
    }
    void TearDown()
    {
        std::cout << "TearDown" << std::endl;
    }

    // static void SetUpTestCase();
    // static void TearDownTestCase();
};

TEST_F(ClientTest, testBase)
{
    std::cout << "testBase" << std::endl;
    std::cout << "testBase2" << std::endl;
    std::cout << "testBase3" << std::endl;
}

// int sum(int a, int b)
// {
//     return a + b;
// }

// TEST(sum, testSum)
// {
//     EXPECT_EQ(5, sum(2, 3)); // 求合2+3=5
//     EXPECT_NE(3, sum(3, 4)); // 求合3+4 != 3
// }
// 如果在此处不写main函数，那么在链接库的时候还需要链接-lgtest_main, 否则只需链接-lgtest即可。

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
