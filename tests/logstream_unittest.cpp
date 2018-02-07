#include "gtest/gtest.h"
#include "log/logging.h"
#include "log/logstream.h"
namespace {
TEST(LogStreamTest,addBool) {
    yamq::log::LogStream os;
    os << true;
    EXPECT_STREQ(os.toString().c_str(),"1");

    os.reset();
    os << false;
    EXPECT_STREQ(os.toString().c_str(),"0");
}
}

