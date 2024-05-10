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
#include "Instances/NodeKinds/SpecialConstNK.h"
#include "Instances/NodeKinds/StubNK.h"
#include "Instances/NodeKinds/VariableNK.h"
#include "Instances/NodeKinds/WhileNK.h"
#include "Instances/Printers/FeaturedPrinter.h"
#include "Instances/Printers/FlexiblePrinter.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/GlobalScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sched.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <unistd.h>

using namespace decoder;
using namespace instances::bytestreams;
using namespace instances::nodekinds;
using namespace instances::printers;
using namespace instances::scopes;

#define STR(x) #x

std::vector<std::pair<std::string, std::string>> FunctionReplacements;

Decoder buildDecoder(char* exe) {
    SingleStringScope::clearManaged();

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

    std::string ReplacementDefs;

    if(FunctionReplacements.empty()) {
        for(Function &F : Functions) {
            FunctionReplacements.emplace_back(F.getName(), F.ReplacementName);
            ReplacementDefs.append(F.ReplacementDef);
        }
    }

    static std::string SmartPointerDef;

    if(SmartPointerDef.empty()) {
        std::filesystem::directory_entry Entry(exe);
        std::ifstream spin(Entry.path().parent_path().string() + "/SmartPointer.h");
        SmartPointerDef = std::string(std::istreambuf_iterator<char>{spin}, {});
        spin.close();
    }

    static std::string Common;
    if(Common.empty()) {
        std::filesystem::directory_entry Entry(exe);
        std::ifstream spin(Entry.path().parent_path().string() + "/Common.h");
        Common = std::string(std::istreambuf_iterator<char>{spin}, {});
        spin.close();
    }

    static std::vector<std::string> Header = {
        // includes and other common things.
        Common,

        // SmartPointer
        SmartPointerDef,
        
        // functions
        #define PREFIX(op) STR(op x)
        #define SUFFIX(op) STR(x op)
        #define INFIX(op) STR(x op y)
        #define RETVAL STR(Z)
        #define RETREF STR(Z&)
        #define BYVAL STR(X x)
        #define BYREF STR(X& x)
        #define NOREF STR(X x) "," STR(Y y)
        #define LREF STR(X& x) "," STR(Y y)

        #define UNARYFUN(Name, Kind, Ret, Args) \
            "template<typename X, typename Z> NOINLINE " \
            Ret " " STR(Name) "(" Args ")" "{" "return " Kind ";" "}", "",
        #define BINARYFUN(Name, Kind, Ret, Args) \
            "template<typename X, typename Y, typename Z> NOINLINE " \
            Ret " " STR(Name) "(" Args ")" "{" "return " Kind ";" "}", "",
        
        #include "Include/OperationReplacements.inc"

        #undef UNARYFUN
        #undef BINARYFUN
        #undef PREFIX
        #undef SUFFIX
        #undef INFIX
        #undef RETVAL
        #undef RETREF
        #undef BYVAL
        #undef BYREF
        #undef NOREF
        #undef LREF

        ReplacementDefs,

        // main prefix
        "int main(void) {",
        //"  SmartPointer<SmartPointer<int>> ipp = 0;",
        "  int** ipp = 0;",
        "  int ip[10];",
        //"  int _ip[10];",
        //"  SmartPointer<int> ip = SmartPointer<int>::capture(_ip);",
        "  int x = 1;",
        "  int y = 0;",
        "  float z = 0.5;",
        //"  SmartPointer<SmartPointer<SmartPointer<int>>> ippp = 0;",
        "  int*** ippp = 0;",
    };
    static std::vector<std::string> Footer = {
        "  printf(\"%lld %d %d %d %d %f\", ipp, ip[0], ip[9], x, y, z);",
        "  return 0;",
        "}",
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
    
    static CallNK TheCallNK(Functions);
    static ProgramNK TheProgramNK(Header, Footer);
    static VariableNK TheVariableNK(Variables);

    static std::map<std::tuple<std::string, int, int, OpKind>, OperationNK *> OpNKs;
    if(OpNKs.empty()) {
        #define OPERATION(Op, AllowFloat, Suffix, Kind) \
            OpNKs[{Op, AllowFloat, Suffix, OpKind::Kind}] = new OperationNK(Op, AllowFloat, Suffix, OpKind::Kind);
        #include "Include/AllOperations.inc"
        #undef OPERATION
    }

    static std::vector<ASTNodeKind *> NodeKinds = {
        BlockNK::get(),
        &TheCallNK,
        CastNK::get(),
        ConstantNK::get(),
        SpecialConstNK::get(),
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
        #include "Include/AllOperations.inc"
        #undef OPERATION
        // TODO: two variants of delete.
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
const char *Command = "g++ input.cpp -fsigned-char -o prg";

int runCompilation() {
    return system(Command);
}

// Maybe there's some shortness bug, not sure.
// Or issues with probability.
// 1) try to fuzz just so
// 2) change probabilities [SEL]
// 3) check for every seed, what is the first value. [all OK]
int main(int argc, char** argv){
    for(int Seed = StartSeed; ; Seed++) {
        std::cout << "TEST #" << Seed << std::endl;
        instances::bytestreams::RandomByteStream BS(Seed);
        Decoder D = buildDecoder(argv[0]);
        ASTNode *Tree = D.GenerateAST(&BS);
        
        SimplePrinter P(Source);
        //FlexiblePrinter P(Source, "TEMPV");
        
        /*
        FlexiblePrinter SP("input2.cpp", "TEMPV");
        FeaturedPrinter P(Source, "TEMPV");
        P.addSmartPointersSupport();
        #define SET_LEFT(op, kind) P.getOperationReplacement(#op).kind
        #define PREFIX(op) SET_LEFT(op, Prefix)
        #define SUFFIX(op) SET_LEFT(op, Suffix)
        #define INFIX(op) SET_LEFT(op, Infix)

        #define UNARYFUN(Name, Kind, Ret, Args) \
            Kind = #Name;
        #define BINARYFUN(Name, Kind, Ret, Args) \
            Kind = #Name;

        #include "Include/OperationReplacements.inc"

        #undef SET_LEFT
        #undef PREFIX
        #undef SUFFIX
        #undef INFIX

        P.getOperationReplacement("&").Prefix = "uref_smart";

        for(auto FR : FunctionReplacements) {
            P.addFunctionReplacement(FR.first, FR.second);
        }
        */
        Tree->print(&P);
        //Tree->print(&SP);
        P.close();
        //SP.close();
        // TODO: add brackets for any op.+
        // TODO: fix stub printing (add auto&).+
        // TODO: add special const. // need???+
        // TODO: add argc to text + variables list.
        if(!P.checkState() || runCompilation()) {
            std::cout << "FAIL" << std::endl;
            //printFile(Source);
            return 0;
        }
    }
}
