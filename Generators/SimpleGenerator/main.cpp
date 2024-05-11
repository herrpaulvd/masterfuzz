#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ios>
#include <iostream>
#include <random>

int main(int argc, char **argv) {
    int limit = atoi(argv[1]);
    std::ofstream out(argv[2], std::ios_base::binary);
    out.tie(0);
    std::mt19937 random(time(0));
    int total = random() % limit;
    while(total--) out << (char)random();
    out.flush();
    out.close();
    return 0;
}
