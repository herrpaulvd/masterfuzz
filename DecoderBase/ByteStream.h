#ifndef DECODERBASE_BYTESTREAM_H
#define DECODERBASE_BYTESTREAM_H

namespace decoder {
    class ByteStream {
    public:
        virtual bool isAlive() = 0;

        virtual void read(char &Next) = 0;

        virtual void readBytes(int Count, void *Next) {
            char *Buffer = (char *)Next;
            for(int i = 0; i < Count; i++)
                read(Buffer[i]);
        }
    };
}

#endif
