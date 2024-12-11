#include "../../test_common.h"
// #include "outfunction.h"

// gdb --args ./kv-test --gtest_filter=*.TestPrepareStmt
// b 01_simple_test.cc:127

class SimpleRelationDQLTest : public KVTest {
  public:
    void SetUp() {
        KVCSrvStart();
        sleep(1);
        std::cout << "SimpleRelationDQLTest::SetUP" << std::endl;
    }
    void TearDown() {
        KVCSrvStop();
        std::cout << "SimpleRelationDQLTest::TearDown" << std::endl;
    }
};

// 综合查询 ddl + dml + dql
TEST_F(SimpleRelationDQLTest, TestMulOperation2) {
    DbConnectT *conn = (DbConnectT *)malloc(sizeof(DbConnectT));
    ASSERT_FALSE(conn == NULL);
    memset(conn, 0, sizeof(DbConnectT));

    ASSERT_EQ(GMERR_OK, KVCConnect(conn));

    uint32_t dbId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateDb(conn, "db_multi_dql", &dbId));
    std::cout << "dbId: " << dbId << std::endl;

    std::string jsonStr1 = ReadFileCpp("/root/db/mul_database/test/sdv/01_simple_test/schema/label3.json");

    uint32_t labelId = 0;
    ASSERT_EQ(GMERR_OK, SRCCreateLabelWithJson(conn, dbId, jsonStr1.c_str(), &labelId));

    CliStmt *stmt = NULL;
    ASSERT_EQ(GMERR_OK, KVCPrepareStmt(conn, &stmt, dbId, labelId));

    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "libai", 50, "yupaochangjian", -100, "lishiming"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbao", 5, "xiaoniutuzhuang", 100, "xiaojiu"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbaoge", 5, "noSleep", 99, "xjzong"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbaoge", 5, "noSleep", 99, "xjzong"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbaoge", 5, "noSleep", 89, "xjzong"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbaoge", 15, "noSleepa", 99, "xjzong1"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbaoge", 5, "noSleep", 99, "xjzong"));
    ASSERT_EQ(GMERR_OK, SRCInsertData(stmt, "jinbao", 5, "xiaoniutuzhuang", 100, "xiaojiu"));

    ASSERT_EQ(GMERR_OK, SRCQueryDataWithCond(stmt, NULL));
    std::cout << "-----------------------" << std::endl;
    ASSERT_EQ(GMERR_OK, SRCQueryDataWithCond(stmt, "age>5"));
    std::cout << "-----------------------" << std::endl;
    ASSERT_EQ(GMERR_OK, SRCQueryDataWithCond(stmt, "luck<99"));
    std::cout << "-----------------------" << std::endl;
    ASSERT_EQ(GMERR_OK, SRCQueryDataWithCond(stmt, "hobby=xiaoniutuzhuang"));
    ASSERT_EQ(GMERR_OK, KVCReleaseStmt(&stmt));

    ASSERT_EQ(GMERR_OK, SRCDeleteDb(conn, "db_multi_dql"));

    ASSERT_EQ(GMERR_OK, KVCDisconnect(conn));
    free(conn);
}