#include "test_common.h"

class SocketTest : public KVTest
{
public:
    void SetUP()
    {
        std::cout << "SocketTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        std::cout << "SocketTest::TearDown" << std::endl;
    }
};


TEST_F(SocketTest, test_epoll)
{

}

