#ifndef IBS_FILEBS_H
#define IBS_FILEBS_H

#include "DecoderBase/ByteStream.h"
#include <fstream>
#include <ios>

namespace instances {
    namespace bytestreams {
        class FileByteStream : public decoder::ByteStream {
        private:
            std::ifstream in;
        public:
            FileByteStream(const char *Filename) : in(Filename, std::ios_base::binary) {
                in.sync_with_stdio(0);
                in.tie(0);
            }

            bool isAlive() override {
                return in.good();
            }

            void read(char &Next) override {
                if(isAlive()) in >> Next;
                else Next = 0;
            }

            void close() {
                in.close();
            }
        };
    }
}

#endif
