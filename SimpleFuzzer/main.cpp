#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>

void addArg(std::string &Out, const std::string &Arg) {
    Out.push_back(' ');
    Out.push_back('"');
    Out.append(Arg);
    Out.push_back('"');
}

int runTool(const char *Cmdline, const char *Toolname) {
    int ExitCode = system(Cmdline);
    if(ExitCode)
        std::cout << "Failed " << Toolname << std::endl;
    return ExitCode;
}

#define run(x) do { if(runTool(x.c_str(), #x)) return 0; } while(0)

int main(int argc, char **argv) {
    // Generator Decoder Checker limit bincode src1 src2 prg compiler
    int argp = 1;

    #define next argv[argp++]

    std::string Generator = next;
    std::string Decoder = next;
    std::string Checker = next;
    
    std::string Limit = next;
    addArg(Generator, Limit);
    
    std::string BinCode = next;
    addArg(Generator, BinCode);
    addArg(Decoder, BinCode);

    std::string Source1 = next;
    addArg(Decoder, Source1);
    addArg(Checker, Source1);

    std::string Source2 = next;
    addArg(Decoder, Source2);
    addArg(Checker, Source2);

    std::string Prg = next;
    addArg(Checker, Prg);

    std::string Compiler = next;
    addArg(Checker, Compiler);

    while(true) {
        run(Generator);
        run(Decoder);
        run(Checker);
    }
}
