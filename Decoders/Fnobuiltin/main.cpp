#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/FileByteStream.h"
#include "Instances/Printers/SimplePrinter.h"
#include <vector>

#define EXTRAARG
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

int main(int argc, char **argv) {
    bool Class2 = Extra[0] == '2';
    std::vector<FunctionBuilder> *Funcs = Class2 ? &Functions2only : &Functions;
    // For Fnobuiltin, args are shifted to 1 because of class detection
    instances::bytestreams::FileByteStream BS(Bincode);

    /*
        bool EnableMemory, // new, delete, indexation, ptr operations
        bool EnableShifts, // << and >>
        bool EnableLoops, // do we require loops
        bool NoFloat, // prohibit floats
        bool DisableDangerous // prohibit any operation causing RE except [] and funcs
    */
    Decoder *D = buildDecoder<true, true, true, true, false>(Variables, *Funcs);
    ASTNode *Tree = D->GenerateAST(&BS);
    BS.close();

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
        Variables, *Funcs, Source1, Prefix, exe);
    Tree->print(P);
    P->close();

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
    P = buildPrinter<true, true, false, false, false, false, false, false>(
        Variables, *Funcs, Source2, Prefix, exe);
    Tree->print(P);
    P->close();

    return 0;
}
