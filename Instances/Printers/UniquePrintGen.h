#ifndef IP_UNIQUEPRINTGEN_H
#define IP_UNIQUEPRINTGEN_H

#include <random>
#include <sstream>
#include <string>

namespace instances {
    namespace printers {
        class UniquePrintGen {
        private:
            std::mt19937 Random;
            std::string Prefix, Suffix;
        public:
            UniquePrintGen(int Seed, const std::string &Prefix, const std::string &Suffix)
                : Random(Seed), Prefix(Prefix), Suffix(Suffix) {}

            std::string make() {
                std::ostringstream out;
                out << Prefix << Random() << Suffix;
                return out.str();
            }
        };
    }
}

#endif
