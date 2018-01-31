#include "gtest/gtest.h"
#include "log/log.h"
namespace {
TEST(LogTest,init) {
    EXPECT_TRUE(yamq::InitLog());
}
TEST(LogTest,shutdown) {
    EXPECT_TRUE(yamq::ShutdownLog());
}
} // namespace
