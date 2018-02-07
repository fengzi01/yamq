#include "gtest/gtest.h"
#include "log/logging.h"
#include "log/logstream.h"
#include <stdio.h>
namespace {
TEST(FixedBufferTest,append) {
    yamq::internal::FixedBuffer<1000> buf;
    buf.append("1",1);
    EXPECT_TRUE(buf.length() == 1);
}
TEST(LogStreamTest,addBool) {
    yamq::log::LogStream os;
    os << true;
    EXPECT_STREQ(os.toString().c_str(),"1");

    os.reset();
    os << false;
    EXPECT_STREQ(os.toString().c_str(),"0");
}

TEST(LogStreamTest,addFloat) {
    yamq::log::LogStream os;
    os << 0.01;
    EXPECT_STREQ(os.toString().c_str(),"0.01");
    os.reset();
    os << -0.01;
    EXPECT_STREQ(os.toString().c_str(),"-0.01");
}

TEST(LogStreamTest,addStr) {
    yamq::log::LogStream os;
    os << "hello world!";
    EXPECT_STREQ(os.toString().c_str(),"hello world!");

    os << " " << "c++";
    EXPECT_STREQ(os.toString().c_str(),"hello world! c++");
}
}

