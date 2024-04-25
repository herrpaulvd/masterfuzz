#include <iostream>
#include "DecoderBase/Decoder.h"

int main(int, char**){
    std::cout << "Hello, from masterfuzz!\n";
    Decoder d;
    std::cout << d.getValue() << std::endl;
    return 0;
}
