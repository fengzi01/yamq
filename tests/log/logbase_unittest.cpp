#include "gtest/gtest.h"
#include "log/logging.h"

namespace {
TEST(LogTest,init) {
    EXPECT_TRUE(yamq::initLogging());
}
TEST(LogTest,shutdown) {
    EXPECT_TRUE(yamq::shutdownLogging());
}
} // namespace
