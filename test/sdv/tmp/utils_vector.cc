#include "test_common.h"

class VectorTest : public KVTest
{
public:
    void SetUP()
    {
        std::cout << "VectorTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        std::cout << "VectorTest::TearDown" << std::endl;
    }

    // static void SetUpTestCase()
    // {
    //     std::cout << "VectorTest::SetUpTestCase" << std::endl;
    // }
    // static void TearDownTestCase()
    // {
    //     std::cout << "VectorTest::TearDownTestCase" << std::endl;
    // }
};

void ShowVector(DbVectorT *vector)
{
    DB_ASSERT(vector);
    for (uint32_t i = 0; i < DbVectorGetSize(vector); i++)
    {
        uint32_t *item = (uint32_t *)DbVectorGetItem(vector, i);
        std::cout << *item << " ";
    }
    std::cout << std::endl;
}

TEST_F(VectorTest, vectorInsertAndRemove)
{
    Status ret = GMERR_OK;
    DbVectorT vector = {0};
    DbVectorInit(&vector, sizeof(uint32_t));
    for (uint32_t i = 0; i < 50; i++)
    {
        ret = DbVectorAppendItem(&vector, &i);
        EXPECT_EQ(ret, GMERR_OK);
    }
    ShowVector(&vector);
    for (uint32_t i = 0; i < 10; i++)
    {
        DbVectorRemoveItem(&vector, 1);
    }
    ShowVector(&vector);
    DbVectorDestroy(&vector);
}

