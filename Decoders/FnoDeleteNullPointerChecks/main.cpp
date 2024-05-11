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
    {"argc", 0, 2, 1, 0, 0, 0},
    {"x", 0, 2, 1, 0, 0, "0"},
    {"y", 0, 2, 1, 0, 0, "0"},
    {"argv", 2, 0, 1, 0, 0, 0},
    {"nullarr", 2, 0, 1, 0, 0, "0"},
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
        Decoder *D = buildDecoder<true, false, false, true, false>(Variables, NoFunctions);
        ASTNode *Tree = D->GenerateAST(&BS);
        /*
            bool EnableFunctionReplacements,
            bool EnableOperationReplacements,
            bool EnableSmartPointers,
            bool EnableUniquePrint,
            bool OutsideFunction,
            bool CheckDiv,
            bool CheckShift
        */
        SimplePrinter *P = buildPrinter<false, false, false, true, false, false, false, false>(
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
        P = buildPrinter<false, true, true, true, false, false, false, false>(
            Variables, NoFunctions, "input.cpp", "TEMPV", argv[0]);
        Tree->print(P);
        P->close();
        if(runCompilation()) {
            std::cout << "FAIL" << std::endl;
            return 0;
        }
    }
}
