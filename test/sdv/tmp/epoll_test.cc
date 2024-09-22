#include <sys/epoll.h>
#include "test_common.h"

class EpollTest : public KVTest
{
public:
    void SetUP()
    {
        std::cout << "EpollTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        std::cout << "EpollTest::TearDown" << std::endl;
    }
};


// 创建epoll实例，通过一棵红黑树管理待检测集合
// int epoll_create(int size);
// 管理红黑树上的文件描述符(添加、修改、删除)
// int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// 检测epoll树中是否有就绪的文件描述符
// int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);


TEST_F(EpollTest, test_epoll)
{
    

}

