#include "gtest/gtest.h"
#include "rpc/io_buffer.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace {
TEST(IoBuffer,base) {
    IoBuffer buf;
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    string str1(200,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableBytes(),200);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    string str2(buf.Peek(),50);
    buf.Seek(50);
    EXPECT_STREQ(str2.c_str(),string(50,'x').c_str());
    EXPECT_EQ(buf.ReadableBytes(),150);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend+50);

    string str3(buf.Peek(),150);
    buf.Seek(150);
    EXPECT_STREQ(str3.c_str(),string(150,'x').c_str());
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);
}

TEST(IoBuffer,grow) {
    IoBuffer buf;

    string str1(400,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableBytes(),400);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    buf.Seek(50);
    EXPECT_EQ(buf.ReadableBytes(),350);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend+50);

    string str2(1000,'x');
    buf.Append(str2.c_str(),str2.length());
    EXPECT_EQ(buf.ReadableBytes(),1350);
    EXPECT_EQ(buf.WritableBytes(),0); // FIXME
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend+50);

    string str3(50,'x');
    buf.Append(str3.c_str(),str3.length());
    EXPECT_EQ(buf.ReadableBytes(),1400); // FIXME
    EXPECT_EQ(buf.WritableBytes(),0);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);
    EXPECT_EQ(buf.Size(),1408);

    buf.Seek(1400);
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),1400); // FIXME
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);
}
}

TEST(IoBuffer,shrink) {
    IoBuffer buf;
     
    string str1(400,'x');
    buf.Append(str1.c_str(),str1.length());
    EXPECT_EQ(buf.ReadableBytes(),400);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    buf.Shrink(0);
    EXPECT_EQ(buf.ReadableBytes(),400);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 400);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    buf.Shrink(2000);
    EXPECT_EQ(buf.ReadableBytes(),400);
    EXPECT_EQ(buf.WritableBytes(),2000);
    EXPECT_EQ(buf.Size(),2408);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);
}

TEST(IoBuffer, put_and_get) {
    IoBuffer buf;
    vector<char> str1(200,'x');
    buf.Put(str1.data(),200);

    EXPECT_STREQ(string(str1.data(),200).c_str(),string(200,'x').c_str());

    EXPECT_EQ(buf.ReadableBytes(),200);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    char str2[300];
    int n = buf.Get(str2,50);
    EXPECT_EQ(n,50);
    EXPECT_STREQ(string(str2,50).c_str(),string(50,'x').c_str());
    EXPECT_EQ(buf.ReadableBytes(),150);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend + 50);

    n = buf.Get(str2,150);
    EXPECT_EQ(n,150);
    EXPECT_STREQ(string(str2,150).c_str(),string(150,'x').c_str());
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    char str4[300];
    buf.Put(str1.data(),200);
    n = buf.Get(str4,300);

    EXPECT_EQ(n,200);
    EXPECT_STREQ(string(str4,200).c_str(),string(200,'x').c_str());
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    buf.Reset();
    buf.PutInt32(32);
    EXPECT_EQ(buf.ReadableBytes(),4);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize-4);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    int32_t v = buf.GetInt32();
    EXPECT_EQ(v,32);
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    int len =buf.PutString("hello");
    string str5 = buf.GetAsString(len);
    EXPECT_STREQ("hello",str5.c_str());
}

TEST(IoBuffer,reset) {
    IoBuffer buf;
    vector<char> str1(200,'x');
    buf.Put(str1.data(),200);

    EXPECT_STREQ(string(str1.data(),200).c_str(),string(200,'x').c_str());

    EXPECT_EQ(buf.ReadableBytes(),200);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize - 200);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);

    buf.Reset();
    char str2[300];
    int n = buf.Get(str2,50);
    EXPECT_EQ(n,0);
    EXPECT_EQ(buf.ReadableBytes(),0);
    EXPECT_EQ(buf.WritableBytes(),IoBuffer::kInitialSize);
    EXPECT_EQ(buf.PrependBytes(),IoBuffer::kCheapPrepend);
}

