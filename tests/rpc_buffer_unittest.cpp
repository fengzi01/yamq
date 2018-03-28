#include "gtest/gtest.h"
#include "rpc/io_buffer.h"
#include <string>
using std::string;

namespace {
TEST(IoBuffer,base) {
    IoBuffer buf;
    EXPECT_EQ(buf.ReadableChars(),0);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);

    string str1(200,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableChars(),200);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);

    string str2(buf.Peek(),50);
    buf.Seek(50);
    EXPECT_STREQ(str2.c_str(),string(50,'x').c_str());
    EXPECT_EQ(buf.ReadableChars(),150);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend+50);

    string str3(buf.Peek(),150);
    buf.Seek(150);
    EXPECT_STREQ(str3.c_str(),string(150,'x').c_str());
    EXPECT_EQ(buf.ReadableChars(),0);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);
}

TEST(IoBuffer,grow) {
    IoBuffer buf;

    string str1(400,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableChars(),400);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);

    buf.Seek(50);
    EXPECT_EQ(buf.ReadableChars(),350);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend+50);

    string str2(1000,'x');
    buf.Append(str2.c_str(),str2.length());
    EXPECT_EQ(buf.ReadableChars(),1350);
    EXPECT_EQ(buf.WriteableChars(),0); // FIXME
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend+50);

    string str3(50,'x');
    buf.Append(str3.c_str(),str3.length());
    EXPECT_EQ(buf.ReadableChars(),1400); // FIXME
    EXPECT_EQ(buf.WriteableChars(),0);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);
    EXPECT_EQ(buf.Size(),1408);

    buf.Seek(1400);
    EXPECT_EQ(buf.ReadableChars(),0);
    EXPECT_EQ(buf.WriteableChars(),1400); // FIXME
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);
}
}

TEST(IoBuffer,shrink) {
    IoBuffer buf;
     
    string str1(400,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableChars(),400);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);

    buf.Shrink(0);
    EXPECT_EQ(buf.ReadableChars(),400);
    EXPECT_EQ(buf.WriteableChars(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);

    buf.Shrink(2000);
    EXPECT_EQ(buf.ReadableChars(),400);
    EXPECT_EQ(buf.WriteableChars(),2000);
    EXPECT_EQ(buf.Size(),2408);
    EXPECT_EQ(buf.PrependChars(),IoBuffer::kCheapPrepend);
}
