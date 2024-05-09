#ifndef INK_CALLNK_H
#define INK_CALLNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/FormatStringScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        enum class ParamType {
            Int = 0,
            SizeT = 1,
            PVoid = 2,
            PChar = 3,
            PWChar = 4,
            File = 5,
            Format = 6,
            WFormat = 7,
            Varargs = 8,
        };

        struct Function {
            Scope *NameScope;
            std::vector<SingleStringScope *> CastScopes;
            std::vector<Scope *> ParamScopes; // with varargs.
            bool HasVarArgs;
            std::string ReplacementDef;
            std::string ReplacementName;

            Function(const char *Name, std::initializer_list<ParamType> Params) {
                NameScope = new SingleStringScope(Name, false);
                
                static SingleStringScope *NotNeeded = new SingleStringScope("", false);
                static SingleStringScope *PWCharCast = new SingleStringScope("wchar_t*", false);

                static Scope *IntScope = ExpressionScope::get(
                    0, 0,
                    2, 2,
                    1, 0, 1,
                    1, 0
                );
                static Scope *SizeScope = ExpressionScope::get(
                    0, 0,
                    2, 2,
                    0, 1, 1,
                    1, 0
                );
                static Scope *PVoidScope = ExpressionScope::get(
                    1, MaxPtrDepth,
                    0, MaxBaseSizeExp,
                    1, 1, 1,
                    1, 1
                );
                // Consider char signed.
                static Scope *PCharScope = ExpressionScope::get(
                    1, 1,
                    0, 0,
                    1, 0, 1,
                    1, 0
                );
                // PWChar scope is actually PVoid scope with cast.
                // File scope is stdout only.
                static Scope *FileScope = new SingleStringScope("stdout", false);
                static Scope *FormatScope = FormatStringScope::get(false);
                static Scope *WFormatScope = FormatStringScope::get(true);
                static Scope *VarargsScope = ExpressionScope::get(
                    0, MaxPtrDepth,
                    0, MaxBaseSizeExp,
                    1, 1, 1,
                    1, 1
                );

                HasVarArgs = false;
                ReplacementName = "repl_";
                ReplacementName.append(Name);
                ReplacementDef = "void ";
                ReplacementDef.append(ReplacementName);
                ReplacementDef.append(" (");
                std::string ReplacementCall = Name;
                ReplacementCall.push_back('(');

                const char InitParamName = 'a';
                char ParamName = InitParamName;
                char LastParamName = ParamName;
                for(ParamType T : Params) {
                    SingleStringScope *Cast;
                    Scope *Param;
                    const char *TName;
                    switch(T) {
                    default: throw "Unknown param type";
                    case ParamType::Int:
                        Cast = NotNeeded;
                        Param = IntScope;
                        TName = "int";
                        break;
                    case ParamType::SizeT:
                        Cast = NotNeeded;
                        Param = SizeScope;
                        TName = "size_t";
                        break;
                    case ParamType::PVoid:
                        Cast = NotNeeded;
                        Param = PVoidScope;
                        TName = "void*";
                        break;
                    case ParamType::PChar:
                        Cast = NotNeeded;
                        Param = PCharScope;
                        TName = "char*";
                        break;
                    case ParamType::PWChar:
                        Cast = PWCharCast;
                        Param = PVoidScope;
                        TName = "wchar_t*";
                        break;
                    case ParamType::File:
                        Cast = NotNeeded;
                        Param = FileScope;
                        TName = "FILE*";
                        break;
                    case ParamType::Format:
                        Cast = NotNeeded;
                        Param = FormatScope;
                        TName = "const char*";
                        break;
                    case ParamType::WFormat:
                        Cast = NotNeeded;
                        Param = WFormatScope;
                        TName = "const wchar_t*";
                        break;
                    case ParamType::Varargs:
                        Cast = NotNeeded;
                        Param = VarargsScope;
                        HasVarArgs = true;
                        TName = "...";
                        break;
                    }
                    CastScopes.push_back(Cast);
                    ParamScopes.push_back(Param);

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
                ReplacementDef.append("}\n");
            }

            const std::string &getName() {
                return ((SingleStringScope *)NameScope)->getString();
            }
        };

        class CallNK : public ASTNodeKind {
        private:
            std::vector<Function> Functions;
        public:
            CallNK(std::vector<Function> Functions) : Functions(Functions) {}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // Shortness is not allowed for complex nodes.
                if(S->isShort()) return false;
                const StatementScope *SS = dynamic_cast<const StatementScope *>(S);
                // Despite the fact that they return values, allow them only in SS to keep it simple.
                if(!SS) return false;

                // Function number is needed.
                Sizes.push_back(getSizeForRange(Functions.size()));
                // Some functions have varargs, get their number, 0-3.
                Sizes.push_back(2);
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {

                // Select function and its varargs count.
                int LastF = Functions.size() - 1;
                const Function &F = Functions[selectInRange(0, LastF, LastF, Values[0])];
                int VarargsCount = Values[1] & 0b11;

                // Push function name as argument.
                OperandsScopes.push_back(F.NameScope);

                int FormalParamCount = F.ParamScopes.size();
                int LastParam = FormalParamCount - 1;
                int ActualParamCount = F.HasVarArgs ? LastParam + VarargsCount : FormalParamCount;
                for(int Actual = 0; Actual < ActualParamCount; Actual++) {
                    int Formal = std::min(Actual, LastParam);
                    OperandsScopes.push_back(F.CastScopes[Formal]);
                    OperandsScopes.push_back(F.ParamScopes[Formal]);
                }
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                if(Part == 0) {
                    SP->startCallNamePart();
                    return;
                }

                if(Part & 1) {
                    if(Part == 1)
                        SP->endCallNamePart();
                    else {
                        SP->endCastArg();
                        if(Last) {
                            SP->endCall();
                            return;
                        }
                        SP->endFunctionArg();
                    }
                    SP->startFunctionArg();
                    SP->startCastTypePart();
                } else {
                    SP->endCastTypePart();
                    SP->startCastArg();
                }
            }
        };
    }
}

#endif
