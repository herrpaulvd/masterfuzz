#ifndef DECODERBASE_PRINTER_H
#define DECODERBASE_PRINTER_H

#include <string>

namespace decoder {
    class Printer {
        virtual void print(const std::string &S);
    };
}

#endif
