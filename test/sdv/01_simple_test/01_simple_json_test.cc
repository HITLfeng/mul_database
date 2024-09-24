#include "../../test_common.h"
// #include "outfunction.h"
#include <jansson.h>

class SimpleRelationJsonTest : public KVTest
{
public:
    void SetUp()
    {
        KVCSrvStart();
        sleep(1);
        std::cout << "SimpleRelationJsonTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        KVCSrvStop();
        std::cout << "SimpleRelationJsonTest::TearDown" << std::endl;
    }
};

TEST_F(SimpleRelationJsonTest, TestJsonOperation)
{
    std::string filePath = "sdv/01_simple_test/schema/label1.json";
    std::string jsonContent = ReadFileCpp(filePath);
    std::cout << jsonContent << std::endl;

    json_t *root = NULL;
    // 使用完记得引用计数-1
    KVStringToJSON(jsonContent.c_str(), &root);

    json_t *nameJson = NULL;
    KVJsonGetObject(root, "labelName", &nameJson);

    const char *getName = NULL;
    KVJsonParseStringObj(nameJson, &getName);
    std::cout << "get name " << getName << std::endl;
}


