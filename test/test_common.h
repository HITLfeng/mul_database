#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <gtest/gtest.h>
// #include "vector_util.h"
#include "outfunction.h"


// TODO:delete
#include "../src/common/include/kv_json.h"

#include <string>

#define DEBUG_FLAG 1
#define GMERR_OK 0
typedef uint32_t Status;



class KVTest : public ::testing::Test
{
public:
    void SetUp()
    {
        std::cout << "KVTest::SetUP" << std::endl;
    }
    void TearDown()
    {
        std::cout << "KVTest::TearDown" << std::endl;
    }

    // static void SetUpTestCase()
    // {
    //     std::cout << "KVTest::SetUpTestCase" << std::endl;
    // }
    // static void TearDownTestCase()
    // {
    //     std::cout << "KVTest::TearDownTestCase" << std::endl;
    // }
};


// 使用后需要手动释放内存 char *
char* ReadFile(const char* filename);



std::string ReadFileCpp(const std::string& filename);
