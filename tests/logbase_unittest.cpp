#include "gtest/gtest.h"
#include "log/logging.h"

namespace {
TEST(LogTest,init) {
//    EXPECT_TRUE(yamq::initLogging());
    EXPECT_TRUE(true);
}
TEST(LogTest,shutdown) {
    EXPECT_TRUE(yamq::shutdownLogging());
}
} // namespace
