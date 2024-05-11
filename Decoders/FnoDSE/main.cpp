#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/FileByteStream.h"
#include "Instances/Printers/SimplePrinter.h"

#include "Builder.h"
#include "Definitions.h"

//Name(), PtrDepth(), BaseSizeExp(), Signed(), Float(), ArraySize(), Initialization() {}
std::vector<VariableBuilder> Variables = {
    {"a1", 0, 0, 1, 0, 0, "0"},
    {"a2", 0, 0, 1, 0, 0, "0"},
    {"b1", 0, 1, 1, 0, 0, "0"},
    {"b2", 0, 1, 1, 0, 0, "0"},
    {"c", 0, 2, 1, 0, 0, "0"},
    {"argc", 0, 2, 1, 0, 0, 0},
    {"d1", 0, 3, 1, 0, 0, "0"},
    {"d2", 0, 3, 1, 0, 0, "0"},
    {"argv", 2, 0, 1, 0, 0, 0},
    {"argv2", 2, 0, 1, 0, 0, "0"},

    {"pc", 1, 0, 1, 0, 0, "new char[10]"},
    {"ps", 1, 1, 1, 0, 0, "new signed short[10]"},
    {"pi", 1, 2, 1, 0, 0, "new signed int[10]"},
    {"pll", 1, 3, 1, 0, 0, "new signed long long[10]"},
    {"pp", 2, 3, 1, 0, 0, "new signed long long*[10]"},

    {"lc", 1, 0, 1, 0, 0, "new char[20]"},
    {"ls", 1, 1, 1, 0, 0, "new signed short[20]"},
    {"li", 1, 2, 1, 0, 0, "new signed int[20]"},
    {"lll", 1, 3, 1, 0, 0, "new signed long long[20]"},
    {"lp", 2, 3, 1, 0, 0, "new signed long long*[10]"},
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
    Decoder *D = buildDecoder<true, false, false, true, false>(Variables, MemoryFunctions);
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
    SimplePrinter *P = buildPrinter<false, false, false, false, false, false, false, true>(
            Variables, MemoryFunctions, Source1, Prefix, exe);
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
    P = buildPrinter<false, true, true, false, false, false, false, true>(
        Variables, MemoryFunctions, Source2, Prefix, exe);
    Tree->print(P);
    P->close();

    return 0;
}
