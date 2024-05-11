#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/RandomByteStream.h"
#include "Instances/Printers/SimplePrinter.h"
#include <cstdlib>
#include <iostream>

#include "Builder.h"
#include "Definitions.h"

//Name(), PtrDepth(), BaseSizeExp(), Signed(), Float(), ArraySize(), Initialization() {}
std::vector<VariableBuilder> Variables = {
    {"pc", 1, 0, 1, 0, 0, 0},
    {"ps", 1, 1, 1, 0, 0, 0},
    {"pi", 1, 2, 1, 0, 0, 0},
    {"pll", 1, 3, 1, 0, 0, 0},
    {"pull", 1, 3, 1, 0, 0, 0},
    {"pf", 1, 2, 1, 1, 0, 0},
    {"pd", 1, 3, 1, 1, 0, 0},
    {"pp", 2, 3, 1, 0, 0, 0},

    {"lc", 1, 0, 1, 0, 0, "0"},
    {"ls", 1, 1, 1, 0, 0, "0"},
    {"li", 1, 2, 1, 0, 0, "0"},
    {"lll", 1, 3, 1, 0, 0, "0"},
    {"lull", 1, 3, 1, 0, 0, "0"},
    {"lf", 1, 2, 1, 1, 0, "0"},
    {"ld", 1, 3, 1, 1, 0, "0"},
    {"lp", 2, 3, 1, 0, 0, "0"},
    {"c1", 0, 0, 0, 0, 0, "0"},
    {"c2", 0, 0, 0, 0, 0, "1"},
    {"c3", 0, 0, 0, 0, 0, "CHAR_MAX"},
};

const int StartSeed = 0;
const char *Source = "input.cpp";
const char *Binary = "prg";
const char *Command = "g++ input.cpp -fsigned-char -o prg";

int runCompilation() {
    return system(Command);
}

int main(int argc, char **argv) {
    for(int Seed = StartSeed; ; Seed++) {
        std::cout << "TEST #" << Seed << std::endl;
        instances::bytestreams::RandomByteStream BS(Seed);
        /*
            bool EnableMemory, // new, delete, indexation, ptr operations
            bool EnableShifts, // << and >>
            bool EnableLoops, // do we require loops
            bool NoFloat, // prohibit floats
            bool DisableDangerous // prohibit any operation causing RE except [] and funcs
        */
        Decoder *D = buildDecoder<true, false, false, false, false>(Variables, NoFunctions);
        ASTNode *Tree = D->GenerateAST(&BS);
        /*
            bool EnableFunctionReplacements,
            bool EnableOperationReplacements,
            bool EnableSmartPointers,
            bool EnableUniquePrint,
            bool OutsideFunction,
            bool CheckDiv,
            bool CheckShift,
            bool PrintAfterDelete
        */
        SimplePrinter *P = buildPrinter<false, false, false, true, true, false, false, false>(
            Variables, NoFunctions, "input.cpp", "TEMPV", argv[0]);
        Tree->print(P);
        P->close();
        if(runCompilation()) {
            std::cout << "FAIL" << std::endl;
            return 0;
        }
        system("cp input.cpp input2.cpp");

        /*
            bool EnableFunctionReplacements,
            bool EnableOperationReplacements,
            bool EnableSmartPointers,
            bool EnableUniquePrint,
            bool OutsideFunction,
            bool CheckDiv,
            bool CheckShift,
            bool PrintAfterDelete
        */
        P = buildPrinter<false, true, true, true, true, false, false, false>(
            Variables, NoFunctions, "input.cpp", "TEMPV", argv[0]);
        Tree->print(P);
        P->close();
        if(runCompilation()) {
            std::cout << "FAIL" << std::endl;
            return 0;
        }
    }
}
