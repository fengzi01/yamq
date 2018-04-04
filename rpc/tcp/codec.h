#pragma once
#include<string>

class IoBuffer;
template<class T>
class Codec {
    public:
        virtual int Encode(IoBuffer &buf,T &message);
        virtual int Decode(IoBuffer &buf,T &message);
};

class StringCodec : public Codec<std::string> {
    public:
        int Encode(IoBuffer &buf,std::string &message);
        int Decode(IoBuffer &buf,std::string &message);
};
