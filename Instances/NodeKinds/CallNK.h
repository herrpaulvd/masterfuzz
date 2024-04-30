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

            Function(const char *Name, std::initializer_list<ParamType> Params) {
                NameScope = new SingleStringScope(Name);
                
                static SingleStringScope *NotNeeded = new SingleStringScope("");
                static SingleStringScope *PWCharCast = new SingleStringScope("wchar_t*");

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
                    1, 1
                );
                // PWChar scope is actually PVoid scope with cast.
                // File scope is stdout only.
                static Scope *FileScope = new SingleStringScope("stdout");
                static Scope *FormatScope = FormatStringScope::get(false);
                static Scope *WFormatScope = FormatStringScope::get(true);
                static Scope *VarargsScope = ExpressionScope::get(
                    0, MaxPtrDepth,
                    0, MaxBaseSizeExp,
                    1, 1, 1,
                    1, 1
                );

                for(ParamType T : Params) {
                    Scope *Cast;
                    Scope *Param;
                    switch(T) {
                    default: throw "Unknown param type";
                    case ParamType::Int:
                        Cast = NotNeeded;
                        Param = IntScope;
                        break;
                    case ParamType::SizeT:
                        Cast = NotNeeded;
                        Param = SizeScope;
                        break;
                    case ParamType::PVoid:
                        Cast = NotNeeded;
                        Param = PVoidScope;
                        break;
                    case ParamType::PChar:
                        Cast = NotNeeded;
                        Param = PCharScope;
                        break;
                    case ParamType::PWChar:
                        Cast = PWCharCast;
                        Param = PVoidScope;
                        break;
                    case ParamType::File:
                        Cast = NotNeeded;
                        Param = FileScope;
                        break;
                    case ParamType::Format:
                        Cast = NotNeeded;
                        Param = FormatScope;
                        break;
                    case ParamType::WFormat:
                        Cast = NotNeeded;
                        Param = WFormatScope;
                        break;
                    case ParamType::Varargs:
                        Cast = NotNeeded;
                        Param = VarargsScope;
                        break;
                    }
                }
            }
        };

        class CallNK : ASTNodeKind {
        private:
            std::vector<Function> Functions;
        public:
            CallNK(std::initializer_list<Function> Functions) : Functions(Functions) {}

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
                int ActualParamCount = F.HasVarArgs ? FormalParamCount : LastParam + VarargsCount;
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
                    // Let printer know that the next child is function name.
                    SP->setParentInfo(ParentInfo::StringFuncall);
                    // Then the printer will emit funcall when meeting
                    // the child.
                    return;
                }

                // Clear after prev part.
                SP->clearParentInfo();

                // Odds are casts, evens are args.
                if(Part & 1) {
                    // Probably, some arg has been emitted before,
                    // need to close it.
                    if(Part > 1) {
                        SP->endCast();
                        if(Last) {
                            SP->endCall();
                            return;
                        }
                        SP->endArg();
                    }

                    // Let printer know that the next child is a cast.
                    SP->setParentInfo(ParentInfo::StringCast);
                } else {
                    // Let the child know it is an expression.
                    SP->setParentInfo(ParentInfo::Expression);
                }
            }
        };
    }
}

#endif
