#pragma once
#include <list>
#include <memory>

class Codec;
class CodecChain {
    public:
        void AddFirst(Codec *c);
        void AddLast(Codec *c);
    private:
        std::list<std::unique_ptr<Codec>> _codec_chain;
};
