#include <iostream>
#include "DecoderBase/Decoder.h"

int main(int, char**){
    std::cout << "Hello, from masterfuzz!\n";
    Decoder d;
    std::cout << d.getValue() + 10 << std::endl;
    return 0;
}
