#include "DecoderBase/ASTNode.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/FileByteStream.h"
#include "Instances/Printers/SimplePrinter.h"

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
    Decoder *D = buildDecoder<true, false, false, false, false>(Variables, NoFunctions);
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
    SimplePrinter *P = buildPrinter<false, false, false, true, true, false, false, false>(
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
    P = buildPrinter<false, true, true, true, true, false, false, false>(
        Variables, NoFunctions, Source2, Prefix, exe);
    Tree->print(P);
    P->close();

    return 0;
}
