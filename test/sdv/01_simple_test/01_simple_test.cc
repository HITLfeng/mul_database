#include "../../test_common.h"
// #include "outfunction.h"

class SimpleRelationTest : public KVTest
{
public:
    void SetUp()
    {
        KVCSrvStart();
        sleep(1);
        std::cout << "SimpleRelationTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        KVCSrvStop();
        std::cout << "SimpleRelationTest::TearDown" << std::endl;
    }
};

TEST_F(SimpleRelationTest, CreateDb)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    UsrResultCreateDbT result = {0};

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "my_first_db_name", &dbId));

    // 检查结果
    EXPECT_EQ(GMERR_OK, result.ret);
    std::cout << "dbId: " << dbId << std::endl;

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
}
