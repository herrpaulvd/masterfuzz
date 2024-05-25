#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/FileByteStream.h"
#include "Instances/Printers/SimplePrinter.h"
#include <string>

#define EXTRAARG
#include "Builder.h"
#include "Definitions.h"

//Name(), PtrDepth(), BaseSizeExp(), Signed(), Float(), ArraySize(), Initialization() {}
std::vector<VariableBuilder> Variables = {
    // two copies for all signed types
    {"a1", 0, 0, 1, 0, 0, "CHAR_MAX"},
    {"a2", 0, 0, 1, 0, 0, "CHAR_MAX"},
    {"b1", 0, 1, 1, 0, 0, "SHRT_MAX"},
    {"b2", 0, 1, 1, 0, 0, "SHRT_MAX"},
    {"c", 0, 2, 1, 0, 0, "INT_MAX"},
    {"argc", 0, 2, 1, 0, 0, 0},
    {"d1", 0, 3, 1, 0, 0, "LLONG_MAX"},
    {"d2", 0, 3, 1, 0, 0, "LLONG_MAX"},
    {"argv", 2, 0, 1, 0, 0, 0},
    {"argv2", 2, 0, 1, 0, 0, "0"},
};

int main(int argc, char **argv) {
    std::string Config = Extra;

    // For Fnobuiltin, args are shifted to 1 because of class detection
    instances::bytestreams::FileByteStream BS(Bincode);

    /*
        bool EnableMemory, // new, delete, indexation, ptr operations
        bool EnableShifts, // << and >>
        bool EnableLoops, // do we require loops
        bool NoFloat, // prohibit floats
        bool DisableDangerous // prohibit any operation causing RE except [] and funcs
    */
    Decoder *D = nullptr;
    if(Config == "0")
        D = buildDecoder<false, false, false, true, true>(Variables, NoFunctions);
    else if(Config == "S")
        D = buildDecoder<false, true, false, true, true>(Variables, NoFunctions);
    else if(Config == "D")
        D = buildDecoder<false, false, false, true, false>(Variables, NoFunctions);
    else if(Config == "SD")
        D = buildDecoder<false, true, false, true, false>(Variables, NoFunctions);
    else if(Config == "--")
        D = buildDecoder<false, false, false, true, true, true>(Variables, NoFunctions);
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
    P = buildPrinter<false, true, false, true, false, false, false, false>(
        Variables, NoFunctions, Source2, Prefix, exe);
    Tree->print(P);
    P->close();

    return 0;
}
