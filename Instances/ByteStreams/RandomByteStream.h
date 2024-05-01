#ifndef IBS_RANDOMBS_H
#define IBS_RANDOMBS_H

#include "DecoderBase/ByteStream.h"
#include <random>

namespace instances {
    namespace bytestreams {
        class RandomByteStream : public decoder::ByteStream {
        private:
            std::mt19937 Random;
            int Remained;
        public:
            RandomByteStream(int Seed) : Random(Seed) {
                Remained = Random() % 1024;
            }

            bool isAlive() override {
                return Remained;
            }

            void read(char &Next) override {
                if(Remained) {
                    Next = Random();
                    Remained--;
                } else Next = 0;
            }
        };
    }
}

#endif
