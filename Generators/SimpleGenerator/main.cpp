#include <chrono>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>

int main(int argc, char **argv) {
    int limit = atoi(argv[1]);
    std::ofstream out(argv[2], std::ios_base::binary);
    out.tie(0);
    unsigned long long time = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::nanoseconds(1);
    std::mt19937_64 random(time);
    while(limit--) out << (char)random();
    out.flush();
    out.close();
    return 0;
}
