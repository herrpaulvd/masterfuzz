#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/FileByteStream.h"
#include "Instances/Printers/SimplePrinter.h"

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

int main(int argc, char **argv) {
    // For Fnobuiltin, args are shifted to 1 because of class detection
    instances::bytestreams::FileByteStream BS(Bincode);

    /*
        bool EnableMemory, // new, delete, indexation, ptr operations
        bool EnableShifts, // << and >>
        bool EnableLoops, // do we require loops
        bool NoFloat, // prohibit floats
        bool DisableDangerous // prohibit any operation causing RE except [] and funcs
    */
    Decoder *D = buildDecoder<true, false, false, true, false>(Variables, NoFunctions);
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
    SimplePrinter *P = buildPrinter<false, false, false, true, false, false, false, false>(
            Variables, NoFunctions, Source1, Prefix, exe);
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
    P = buildPrinter<false, true, true, true, false, false, false, false>(
        Variables, NoFunctions, Source2, Prefix, exe);
    Tree->print(P);
    P->close();

    return 0;
}
