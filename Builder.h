#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Decoder.h"
#include "DecoderBase/Utils.h"
#include "Instances/NodeKinds/BlockNK.h"
#include "Instances/NodeKinds/CallNK.h"
#include "Instances/NodeKinds/CastNK.h"
#include "Instances/NodeKinds/ConstantNK.h"
#include "Instances/NodeKinds/DeleteNK.h"
#include "Instances/NodeKinds/ForNK.h"
#include "Instances/NodeKinds/FormatStringNK.h"
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
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Printers/UniquePrintGen.h"
#include "Instances/Scopes/GlobalScope.h"
#include <filesystem>
#include <string>
#include <vector>

#ifdef BUILDER_H
#error "Make sure you include this file once in each test harness"
#endif

#define BUILDER_H

#define STR(x) #x

using namespace decoder;
using namespace instances::nodekinds;
using namespace instances::printers;

struct VariableBuilder {
    const char *Name;
    int PtrDepth;
    int BaseSizeExp;
    bool Signed;
    bool Float;
    const char *ArraySize; // if null, not an array.
    const char *Initialization; // if null, do not declare it, like argc or argv.

    VariableBuilder() : Name(), PtrDepth(), BaseSizeExp(), Signed(), Float(), ArraySize(), Initialization() {}

    VariableBuilder(const char *Name, int PtrDepth, int BaseSizeExp, bool Signed, bool Float, const char *ArraySize, const char *Initialization) 
        : Name(Name), PtrDepth(PtrDepth), BaseSizeExp(BaseSizeExp), Signed(Signed), Float(Float), ArraySize(ArraySize), Initialization(Initialization) {}
};

struct FunctionBuilder {
    const char *Name;
    std::vector<ParamType> Params;
    const char *ReplacementName;

    FunctionBuilder() : Name(), Params(), ReplacementName() {}

    FunctionBuilder(const char *Name, std::vector<ParamType> Params, const char *ReplacementName)
        : Name(Name), Params(Params), ReplacementName(ReplacementName) {}
};

template<
    bool EnableMemory, // new, delete, indexation, ptr operations
    bool EnableShifts, // << and >>
    bool EnableLoops, // do we require loops
    bool NoFloat, // prohibit floats
    bool DisableDangerous // prohibit any operation causing RE except [] and funcs
> inline Decoder *buildDecoder(
    std::vector<VariableBuilder> Variables,
    std::vector<FunctionBuilder> Functions
) {
    // Add invariant NKs
    std::vector<ASTNodeKind *> NodeKinds = {
        ProgramNK::get(),
        BlockNK::get(),
        CastNK::get(),
        IfNK::get(true),
        IfNK::get(false),
    };
    if(EnableMemory) {
        if(!DisableDangerous) {
            NodeKinds.push_back(DeleteNK::get());
            NodeKinds.push_back(NewNK::get());
        }
        NodeKinds.push_back(IndexNK::get());
    }
    if(EnableLoops) {
        NodeKinds.push_back(ForNK::get());
        NodeKinds.push_back(WhileNK::get(false));
        NodeKinds.push_back(WhileNK::get(true));
    }

    #define MEMORY(op) if(EnableMemory) op
    #define SHIFT(op) if(EnableShifts) op
    #define DANGEROUS(op) if(!DisableDangerous) op
    #define OPERATION(Op, AllowFloat, Suffix, Kind) \
        NodeKinds.push_back(new OperationNK(Op, AllowFloat, Suffix, OpKind::Kind));
    #include "Include/AllOperations.inc"
    #undef MEMORY
    #undef SHIFT
    #undef DANGEROUS
    #undef OPERATION

    int TerminalCount = 3 - (EnableMemory & EnableShifts);
    ConstantNK *CNK = ConstantNK::get();
    ASTNodeKind *VNK;
    if(Variables.size()) {
        std::vector<Variable> VarUses;
        for(const VariableBuilder &VB : Variables)
            VarUses.emplace_back(VB.Name, VB.PtrDepth, VB.BaseSizeExp, VB.Signed, VB.Float, VB.ArraySize);
        VNK = new VariableNK(VarUses);
    } else VNK = CNK;

    while(TerminalCount--) {
        NodeKinds.push_back(VNK);
        NodeKinds.push_back(CNK);
    }
    NodeKinds.push_back(SpecialConstNK::get());
    
    if(Functions.size()) {
        std::vector<Function> FunUses;
        bool HasFormat = false;
        for(const FunctionBuilder &FB: Functions) {
            FunUses.emplace_back(FB.Name, FB.Params);
            if(!HasFormat) {
                for(ParamType PT : FB.Params)
                    if(PT == ParamType::Format || PT == ParamType::WFormat) {
                        HasFormat = true;
                        break;
                    }
            }
        }
        NodeKinds.push_back(new CallNK(FunUses));
        if(HasFormat)
            NodeKinds.push_back(FormatStringNK::get());
    }

    return new Decoder(instances::scopes::GlobalScope::getLarge(), NodeKinds, StubNK::get(), NoFloat);
}

template<
    bool EnableFunctionReplacements,
    bool EnableOperationReplacements,
    bool EnableSmartPointers,
    bool EnableUniquePrint,
    bool OutsideFunction,
    bool CheckDiv, bool CheckShift,
    bool PrintAfterDelete
>
SimplePrinter *buildPrinter(
    std::vector<VariableBuilder> Variables,
    std::vector<FunctionBuilder> Functions,
    std::string Filename, std::string Prefix,
    const char *exe
) {
    SimplePrinter *SP;
    FeaturedPrinter *FP;
    if(EnableFunctionReplacements || EnableOperationReplacements
        || EnableSmartPointers
        || CheckDiv || CheckShift || PrintAfterDelete) {
        SP = FP = new FeaturedPrinter(Filename, Prefix);
    } else {
        SP = new SimplePrinter(Filename);
        FP = nullptr;
    }

    std::vector<std::string> Header;
    Header.push_back("#include <limits.h>");
    if(EnableFunctionReplacements) Header.push_back("#include <stdarg.h>");
    Header.push_back("#include <stdio.h>");
    if(Functions.size()) {
        Header.push_back("#include <string.h>");
        Header.push_back("#include <wchar.h>");
    }

    if(FP || EnableUniquePrint || OutsideFunction || PrintAfterDelete)
        Header.push_back("#define NOINLINE __attribute__((noipa))");

    if(EnableUniquePrint) {
        Header.push_back("NOINLINE void print_unique(long long Number) {printf(\"%lld\\n\", Number);}");
        SP->setGen(new UniquePrintGen(0, "printf(\"%lld\", ", "LL);"));
    }

    if(PrintAfterDelete) {
        Header.push_back("NOINLINE void printAfterDelete(unsigned long long Value) {printf(\"%d\", (int)*(char*)Value);}");
        FP->setPrintAfterDelete("printAfterDelete");
    }
    
    if(EnableFunctionReplacements) {
        for(const FunctionBuilder &FB : Functions) {
            bool HasVarArgs = false;
            std::string ReplacementDef = "void ";
            ReplacementDef.append(FB.ReplacementName);
            ReplacementDef.append(" (");

            std::string ReplacementCall = FB.Name;
            ReplacementCall.push_back('(');

            const char InitParamName = 'a';
            char ParamName = InitParamName;
            char LastParamName = ParamName;

            for(ParamType T : FB.Params) {
                const char *TName;
                switch(T) {
                default: throw "Unknown param type";
                case ParamType::Int:
                    TName = "int";
                    break;
                case ParamType::SizeT:
                    TName = "size_t";
                    break;
                case ParamType::PVoid:
                    TName = "void*";
                    break;
                case ParamType::PChar:
                    TName = "char*";
                    break;
                case ParamType::PWChar:
                    TName = "wchar_t*";
                    break;
                case ParamType::File:
                    TName = "FILE*";
                    break;
                case ParamType::Format:
                    TName = "const char*";
                    break;
                case ParamType::WFormat:
                    TName = "const wchar_t*";
                    break;
                case ParamType::Varargs:
                    HasVarArgs = true;
                    TName = "...";
                    break;
                }

                if(ReplacementDef.back() != '(') {
                    ReplacementDef.append(", ");
                    ReplacementCall.append(", ");
                }
                ReplacementDef.append(TName);
                if(T == ParamType::Varargs) {
                    ReplacementCall.append("va");
                } else {
                    ReplacementDef.push_back(' ');
                    ReplacementDef.push_back(ParamName);
                    ReplacementCall.push_back(ParamName);
                    LastParamName = ParamName++;
                }
            }
            ReplacementCall.append(");");
            ReplacementDef.append(") {");
            if(HasVarArgs) {
                ReplacementDef.append("va_list va;va_start(va, ");
                ReplacementDef.push_back(LastParamName);
                ReplacementDef.append(");v");
            }
            ReplacementDef.append(ReplacementCall);
            if(HasVarArgs)
                ReplacementDef.append("va_end(va);");
            ReplacementDef.append("}");

            Header.push_back(ReplacementDef);
            FP->addFunctionReplacement(FB.Name, FB.ReplacementName);
        }
    }

    if(EnableSmartPointers) {
        std::filesystem::directory_entry Entry(exe);
        std::ifstream spin(Entry.path().parent_path().string() + "/SmartPointer.h");
        std::string SmartPointerDef(std::istreambuf_iterator<char>{spin}, {});
        spin.close();
        Header.push_back(SmartPointerDef);
        FP->addSmartPointersSupport();
    }

    if(EnableOperationReplacements) {
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
            Header.push_back("template<typename X, typename Z> NOINLINE " \
            Ret " " STR(Name) "(" Args ")" "{" "return " Kind ";" "}" );
        #define BINARYFUN(Name, Kind, Ret, Args) \
            Header.push_back("template<typename X, typename Y, typename Z> NOINLINE " \
            Ret " " STR(Name) "(" Args ")" "{" "return " Kind ";" "}" );
        
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
    }

    #define SET_LEFT(op, kind) FP->getOperationReplacement(#op).kind
    #define PREFIX(op) SET_LEFT(op, Prefix)
    #define SUFFIX(op) SET_LEFT(op, Suffix)
    #define INFIX(op) SET_LEFT(op, Infix)

    if(EnableOperationReplacements) {
        #define UNARYFUN(Name, Kind, Ret, Args) \
            Kind = #Name;
        #define BINARYFUN(Name, Kind, Ret, Args) \
            Kind = #Name;

        #include "Include/OperationReplacements.inc"
    }

    if(EnableSmartPointers)
        UNARYFUN(uref_smart, PREFIX(&), RETVAL, BYREF)

    #undef SET_LEFT
    #undef PREFIX
    #undef SUFFIX
    #undef INFIX
    #undef UNARYFUN
    #undef BINARYFUN

    if(CheckDiv) {
        Header.push_back(
            "template<typename X, typename Y, typename Z> "
            "NOINLINE Z bdiv_check(X x, Y y) {"
            "if(y == 0) {printf(\"-Wdiv-by-zero\"); return 0;}"
            "return x / y;}");
        Header.push_back(
            "template<typename X, typename Y, typename Z> "
            "NOINLINE  bmod_check(X x, Y y) {"
            "if(y == 0) {printf(\"-Wdiv-by-zero\"); return 0;}"
            "return x % y;}");
        FP->getOperationReplacement("/").Infix = "bdiv_check";
        FP->getOperationReplacement("%").Infix = "bmod_check";
    }

    if(CheckShift) {
        Header.push_back(
            "template<typename X, typename Y, typename Z> "
            "NOINLINE Z bshl_check(X x, Y y) {"
            "if(y < 0) {printf(\"-Wshift-count-negative\"); return x;}"
            "if(y >= sizeof(x)) {printf(\"-Wshift-count-overflow\"); return 0;}"
            "return x << y;}");
        Header.push_back(
            "template<typename X, typename Y, typename Z> "
            "NOINLINE Z bshr_check(X x, Y y) {"
            "if(y < 0) {printf(\"-Wshift-count-negative\"); return x;}"
            "if(y >= sizeof(x)) {printf(\"-Wshift-count-overflow\"); return 0;}"
            "return x >> y;}");
        FP->getOperationReplacement("<<").Infix = "bshl_check";
        FP->getOperationReplacement(">>").Infix = "bshr_check";
    }

    if(OutsideFunction) {
        std::string Foo = "NOINLINE void foo(";
        for(const VariableBuilder &VB : Variables) {
            if(!VB.Initialization) {
                if(Foo.back() != '(') Foo.append(", ");
                Foo.append(makeTypeName(VB.PtrDepth, VB.BaseSizeExp, VB.Float, VB.Signed, false));
                Foo.push_back(' ');
                Foo.append(VB.Name);
            }
        }
        Foo.append(") {");
        Header.push_back(Foo);
    } else {
        Header.push_back("int main(int argc, char **argv) {");
        Header.push_back("  if(argc > 3) return 0;");
    }
    
    for(const VariableBuilder &VB : Variables) {
        std::string Def = "  ";
        if(!VB.Initialization) {
            if(EnableSmartPointers && VB.PtrDepth) {
                Def.append(makeSmartTypeName(VB.PtrDepth, VB.BaseSizeExp, VB.Float, VB.Signed, VB.ArraySize));
                Def.push_back(' ');
                Def.push_back('_');
                Def.append(VB.Name);
                Def.push_back('(');
                Def.append(VB.Name);
                Def.push_back(')');
                Def.push_back(';');
                Header.push_back(Def);
                Def = "  #define ";
                Def.append(VB.Name);
                Def.push_back(' ');
                Def.push_back('_');
                Def.append(VB.Name);
                Header.push_back(Def);
            }
            continue;
        }
        Def.append(makeTypeName(VB.PtrDepth - (bool)VB.ArraySize, VB.BaseSizeExp, VB.Float, VB.Signed, VB.ArraySize));
        Def.push_back(' ');
        if(EnableSmartPointers && VB.PtrDepth)
            Def.push_back('_');
        Def.append(VB.Name);
        if(VB.ArraySize) {
            Def.push_back('[');
            Def.append(VB.ArraySize);
            Def.append("] = {0};");
        } else {
            Def.append(" = ");
            Def.append(VB.Initialization);            
        }
        Def.push_back(';');
        Header.push_back(Def);
        if(EnableSmartPointers && VB.PtrDepth) {
            Def = "  ";
            Def.append(makeSmartTypeName(VB.PtrDepth, VB.BaseSizeExp, VB.Float, VB.Signed, VB.ArraySize));
            Def.push_back(' ');
            Def.append(VB.Name);
            Def.append("(_");
            Def.append(VB.Name);
            Def.append(");");
            Header.push_back(Def);
        }
    }

    ProgramNK *Program = ProgramNK::get();
    Program->setHeader(Header);

    std::vector<std::string> Footer;

    std::string Strprintf = "  printf(\"";
    std::string Args;
    for(const VariableBuilder &VB : Variables) {
        Strprintf.append("%lld\\n");
        Args.append(", ");
        Args.append("(long long)");
        Args.append(VB.Name);
    }
    Strprintf.push_back('\"');
    Strprintf.append(Args);
    Strprintf.append(");");
    Footer.push_back(Strprintf);

    if(OutsideFunction) {
        Footer.push_back("}");
        Footer.push_back("int main(int argc, char **argv) {");
        Footer.push_back("  long long array[256] = {0};");
        std::string FooCall = "  foo(";
        for(const VariableBuilder &VB : Variables) {
            if(!VB.Initialization) {
                if(FooCall.back() != '(') FooCall.append(", ");
                FooCall.push_back('(');
                FooCall.append(makeTypeName(VB.PtrDepth, VB.BaseSizeExp, VB.Float, VB.Signed, false));
                FooCall.push_back(')');
                FooCall.append("(unsigned long long)");
                FooCall.append("array");
                if(!VB.PtrDepth) FooCall.append("[0]");
            }
        }
        FooCall.append(");");
        Footer.push_back(FooCall);
    }
    Footer.push_back("  return 0;");
    Footer.push_back("}");

    Program->setFooter(Footer);

    return SP;
}
