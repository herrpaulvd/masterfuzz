#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Decoder.h"
#include "Instances/ByteStreams/RandomByteStream.h"
#include "Instances/NodeKinds/BlockNK.h"
#include "Instances/NodeKinds/CallNK.h"
#include "Instances/NodeKinds/CastNK.h"
#include "Instances/NodeKinds/ConstantNK.h"
#include "Instances/NodeKinds/DeleteNK.h"
#include "Instances/NodeKinds/FormatStringNK.h"
#include "Instances/NodeKinds/ForNK.h"
#include "Instances/NodeKinds/IfNK.h"
#include "Instances/NodeKinds/IndexNK.h"
#include "Instances/NodeKinds/NewNK.h"
#include "Instances/NodeKinds/OperationNK.h"
#include "Instances/NodeKinds/ProgramNK.h"
#include "Instances/NodeKinds/StubNK.h"
#include "Instances/NodeKinds/VariableNK.h"
#include "Instances/NodeKinds/WhileNK.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/GlobalScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace decoder;
using namespace instances::bytestreams;
using namespace instances::nodekinds;
using namespace instances::printers;
using namespace instances::scopes;

Decoder buildDecoder() {
    SingleStringScope::clearManaged();

    static std::vector<std::string> Header = {
        "#include <string.h>",
        "#include <wchar.h>",
        "#include <stdio.h>",
        "",
        "int main(void) {",
        "  int **ipp = 0;",
        "  int *ip[10];",
        "  int x = 1;",
        "  int y = 0;",
        "  float z = 0.5;",
        "  int *** ippp = 0;"
    };
    static std::vector<std::string> Footer = {
        "  printf(\"%lld %d %d %d %d %f\", ipp, ip[0], ip[9], x, y, z);",
        "  return 0;",
        "}"
    };

    static std::vector<Variable> Variables = {
        // {name, ptrdepth, sizeexp, signess, float, readonly}
        {"ipp", 2, 2, 1, 0, 0},
        {"ip", 1, 2, 1, 0, 1},
        {"x", 0, 2, 1, 0, 0},
        {"y", 0, 2, 1, 0, 0},
        {"z", 0, 2, 1, 1, 0},
        {"ippp", 3, 2, 1, 0, 0},
    };

    static std::vector<Function> Functions = {
        // {name, {ParamType::Int|SizeT|PVoid|PChar|PWChar|File|Format|WFormat|Varargs}}
        {"printf", {ParamType::Format, ParamType::Varargs}}, 
        {"fprintf", {ParamType::File, ParamType::Format, ParamType::Varargs}}, 
        {"sprintf", {ParamType::PChar, ParamType::Format, ParamType::Varargs}}, 
        {"snprintf", {ParamType::PChar, ParamType::SizeT, ParamType::Format, ParamType::Varargs}},

        {"wprintf", {ParamType::WFormat, ParamType::Varargs}},
        {"fwprintf", {ParamType::File, ParamType::WFormat, ParamType::Varargs}},
        {"swprintf", {ParamType::PWChar, ParamType::SizeT, ParamType::WFormat, ParamType::Varargs}},

        {"memcpy", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}},
        {"memmove", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}},
        {"memset", {ParamType::PVoid, ParamType::Int, ParamType::SizeT}},

        {"wmemcpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}},
        {"wmemmove", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}},
        {"wmemset", {ParamType::PWChar, ParamType::Int, ParamType::SizeT}},

        {"strcat", {ParamType::PChar, ParamType::PChar}},
        {"strncat", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}},
        {"strcpy", {ParamType::PChar, ParamType::PChar}},
        {"strncpy", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}},

        {"wcscat", {ParamType::PWChar, ParamType::PWChar}},
        {"wcsncat", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}},
        {"wcscpy", {ParamType::PWChar, ParamType::PWChar}},
        {"wcsncpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}},
    };
    
    static CallNK TheCallNK(Functions);
    static ProgramNK TheProgramNK(Header, Footer);
    static VariableNK TheVariableNK(Variables);

    static std::map<std::tuple<std::string, int, int, OpKind>, OperationNK *> OpNKs;
    if(OpNKs.empty()) {
        #define OPERATION(Op, AllowFloat, Suffix, Kind) \
            OpNKs[{Op, AllowFloat, Suffix, OpKind::Kind}] = new OperationNK(Op, AllowFloat, Suffix, OpKind::Kind);
        #include "Instances/NodeKinds/AllOperations.inc"
        #undef OPERATION
    }

    static std::vector<ASTNodeKind *> NodeKinds = {
        BlockNK::get(),
        &TheCallNK,
        CastNK::get(),
        ConstantNK::get(),
        DeleteNK::get(),
        FormatStringNK::get(),
        ForNK::get(),
        IfNK::get(true),
        IfNK::get(false),
        IndexNK::get(),
        NewNK::get(),
        &TheProgramNK,
        &TheVariableNK,
        WhileNK::get(true),
        WhileNK::get(false),
        #define OPERATION(Op, AllowFloat, Suffix, Kind) \
            OpNKs[{Op, AllowFloat, Suffix, OpKind::Kind}],
        #include "Instances/NodeKinds/AllOperations.inc"
        #undef OPERATION
    };

    // Build new decoder every time because after use it becomes invalid.
    return Decoder(GlobalScope::getLarge(), NodeKinds, StubNK::get());
}

void printFile(const char *Filename) {
    std::ifstream in(Filename);
    std::string line;
    while(std::getline(in, line))
        std::cout << line << std::endl;
    in.close();
}

const int StartSeed = 0;
const char *Source = "input.cpp";
const char *Binary = "prg";
const char *Command = "g++ input.cpp -o prg";

// Maybe there's some shortness bug, not sure.
// Or issues with probability.
// 1) try to fuzz just so
// 2) change probabilities
// 3) check for every seed, what is the first value.
int main(int argc, char** argv){
    for(int Seed = StartSeed; ; Seed++) {
        std::cout << "TEST #" << Seed << std::endl << std::endl << std::endl;
        instances::bytestreams::RandomByteStream BS(Seed);
        Decoder D = buildDecoder();
        ASTNode *Tree = D.GenerateAST(&BS);
        SimplePrinter P(Source);
        Tree->print(&P);
        P.close();
        //printFile(Source);
    }
}
