#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/RandomByteStream.h"
#include "Instances/Printers/SimplePrinter.h"
#include <cstdlib>
#include <iostream>
#include <vector>

#include "Builder.h"
#include "Definitions.h"

//Name(), PtrDepth(), BaseSizeExp(), Signed(), Float(), ArraySize(), Initialization() {}
std::vector<VariableBuilder> Variables = {
    // two copies for all signed types
    {"a1", 0, 0, 1, 0, 0, "0"},
    {"b1", 0, 1, 1, 0, 0, "0"},
    {"c", 0, 2, 1, 0, 0, "0"},
    {"argc", 0, 2, 1, 0, 0, 0},
    {"d1", 0, 3, 1, 0, 0, "0"},
    {"argv", 2, 0, 1, 0, 0, 0},
    {"s", 1, 0, 1, 0, 0, "(char*)(\"Hello World!\")"}
};

const int StartSeed = 0;
const char *Source = "input.cpp";
const char *Binary = "prg";
const char *Command = "g++ input.cpp -fsigned-char -o prg";

int runCompilation() {
    return system(Command);
}

int main(int argc, char **argv) {
    bool Class2 = argc > 1 && argv[1][0] == '2';
    std::vector<FunctionBuilder> *Funcs = Class2 ? &Functions2only : &Functions;
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
        
        Decoder *D = buildDecoder<false, false, false, true, false>(Variables, *Funcs);
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
        SimplePrinter *P = buildPrinter<false, false, false, false, false, false, false, false>(
            Variables, *Funcs, "input.cpp", "TEMPV", argv[0]);
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
        P = buildPrinter<true, false, false, false, false, false, false, false>(
            Variables, *Funcs, "input.cpp", "TEMPV", argv[0]);
        Tree->print(P);
        P->close();
        if(runCompilation()) {
            std::cout << "FAIL" << std::endl;
            return 0;
        }
    }
}
