#include "../../test_common.h"
// #include "outfunction.h"

class KVCalcTest : public KVTest
{
public:
    void SetUp()
    {
        KVCSrvStart();
        sleep(1);
        std::cout << "KVCalcTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        KVCSrvStop();
        std::cout << "KVCalcTest::TearDown" << std::endl;
    }
};

TEST_F(KVCalcTest, ConnectTest)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    int32_t result = 0;
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 1, 2, CALC_ADD, &result));

    // 检查结果
    EXPECT_EQ(3, result);

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
}

TEST_F(KVCalcTest, ConnectTryManyTest)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    int32_t result = 0;
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 1, 2, CALC_ADD, &result));

    // 检查结果
    EXPECT_EQ(3, result);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 11, 20, CALC_MUL, &result));
    EXPECT_EQ(220, result);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 12, 20, CALC_SUB, &result));
    EXPECT_EQ(-8, result);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 9, 3, CALC_DIV, &result));
    EXPECT_EQ(3, result);

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
}