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
    KVConnectT *conn = (KVConnectT *)malloc(sizeof(KVConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(KVConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    UsrResultCalcT result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 1, 2, CALC_ADD, (UsrResultBaseT *)&result));

    // 检查结果
    EXPECT_EQ(GMERR_OK, result.ret);
    EXPECT_EQ(3, result.calcAns);

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
}

TEST_F(KVCalcTest, ConnectTryManyTest)
{
    KVConnectT *conn = (KVConnectT *)malloc(sizeof(KVConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(KVConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    UsrResultCalcT result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 1, 2, CALC_ADD, (UsrResultBaseT *)&result));

    // 检查结果
    EXPECT_EQ(GMERR_OK, result.ret);
    EXPECT_EQ(3, result.calcAns);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 11, 20, CALC_MUL, (UsrResultBaseT *)&result));
    EXPECT_EQ(GMERR_OK, result.ret);
    EXPECT_EQ(220, result.calcAns);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 12, 20, CALC_SUB, (UsrResultBaseT *)&result));
    EXPECT_EQ(GMERR_OK, result.ret);
    EXPECT_EQ(-8, result.calcAns);

    result = {0};
    ASSERT_EQ(GMERR_OK, KVCCalcTwoNumber(conn, 9, 3, CALC_DIV, (UsrResultBaseT *)&result));
    EXPECT_EQ(GMERR_OK, result.ret);
    EXPECT_EQ(3, result.calcAns);

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
}