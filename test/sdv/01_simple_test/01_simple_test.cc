#include "../../test_common.h"
// #include "outfunction.h"

// gdb --args ./kv-test --gtest_filter=*.TestPrepareStmt
// b 01_simple_test.cc:127

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

TEST_F(SimpleRelationTest, TestCreateDb)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    // UsrDataSimpleRelT result = {0};

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "my_first_db_name", &dbId));

    // 检查结果
    // EXPECT_EQ(GMERR_OK, result.ret);
    std::cout << "dbId: " << dbId << std::endl;

    ASSERT_EQ(GMERR_OK, SRCDeleteDb(conn, "my_first_db_name"));

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
    free(conn);
}

TEST_F(SimpleRelationTest, TestCreateTable)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    // UsrDataSimpleRelT result = {0};

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "my_first_db_name2", &dbId));

    // 检查结果
    // EXPECT_EQ(GMERR_OK, result.ret);
    std::cout << "dbId2: " << dbId << std::endl;

    std::string jsonStr = ReadFileCpp("/root/db/mul_database/test/sdv/01_simple_test/schema/label1.json");

    uint32_t labelId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateLabelWithJson(conn, dbId, jsonStr.c_str(), &labelId));
    std::cout << "labelId: " << labelId << std::endl;

    ASSERT_EQ(GMERR_OK, SRCDeleteDb(conn, "my_first_db_name2"));

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
    free(conn);
}

TEST_F(SimpleRelationTest, TestCreateTableDFX)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    // UsrDataSimpleRelT result = {0};

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "my_first_db_name3", &dbId));

    // 检查结果
    // EXPECT_EQ(GMERR_OK, result.ret);
    std::cout << "dbId2: " << dbId << std::endl;

    std::string jsonStr = ReadFileCpp("/root/db/mul_database/test/sdv/01_simple_test/schema/label1.json");

    uint32_t labelId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateLabelWithJson(conn, dbId, jsonStr.c_str(), &labelId));

    ASSERT_EQ(GMERR_OK, SRCTraceDbDesc(conn, dbId));

    ASSERT_EQ(GMERR_OK, SRCDeleteDb(conn, "my_first_db_name3"));

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
    free(conn);
}

TEST_F(SimpleRelationTest, TestPrepareStmt)
{
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    // UsrDataSimpleRelT result = {0};

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "my_first_db_name3", &dbId));

    // 检查结果
    // EXPECT_EQ(GMERR_OK, result.ret);
    std::cout << "dbId2: " << dbId << std::endl;

    std::string jsonStr = ReadFileCpp("/root/db/mul_database/test/sdv/01_simple_test/schema/label1.json");

    uint32_t labelId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateLabelWithJson(conn, dbId, jsonStr.c_str(), &labelId));

    CliStmt *stmt = NULL;
    ASSERT_EQ(GMERR_OK, KVCPrepareStmt(conn, &stmt, dbId, labelId));
    
    ASSERT_EQ(GMERR_OK, KVCReleaseStmt(stmt));

    ASSERT_EQ(GMERR_OK, SRCTraceDbDesc(conn, dbId));

    ASSERT_EQ(GMERR_OK, SRCDeleteDb(conn, "my_first_db_name3"));

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
    free(conn);
}


