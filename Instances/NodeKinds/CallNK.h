#ifndef INK_CALL_H
#define INK_CALL_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;

namespace instances {
    namespace nodekinds {
        enum class ReturnType {
            Void, ByParam, ByScope
        };

        struct FunctionParamType {
            ReturnType Kind;
            union {
                struct {
                    int Index;
                    int DepthMod;
                    int Lvalue;
                } Param;
                ExpressionScope *Scope;
            } Value;

            FunctionParamType(int Index, int DepthMod, int Lvalue) : Kind(ReturnType::ByParam) {
                Value.Param.Index = Index;
                Value.Param.DepthMod = DepthMod;
                Value.Param.Lvalue = Lvalue;
            }

            FunctionParamType(ExpressionScope *Scope) : Kind(ReturnType::ByScope) {
                Value.Scope = Scope;
            }
        };

        struct Function {
            std::string Name;
            std::vector<ExpressionScope *> Params;
            FunctionParamType ReturnType;
        };

        class CallNK : ASTNodeKind {
        private:
            std::vector<Function> Functions;
            mutable std::vector<int> Filtered;

            static bool Intersect(const ExpressionScope *A, const ExpressionScope *B,
                int &PtrDepthMin, int &PtrDepthMax,
                int &BaseSizeExpMin, int &BaseSizeExpMax,
                int &AllowSigned, int &AllowUnsigned, int &AllowRvalue,
                int &AllowInt, int &AllowFloat
                ) {
                PtrDepthMin = std::max(A->getPtrDepthMin(), B->getPtrDepthMin());
                PtrDepthMax = std::min(A->getPtrDepthMax(), B->getPtrDepthMax());
                if(PtrDepthMin > PtrDepthMax) return false;
                BaseSizeExpMin = std::max(A->getBaseSizeExpMin(), B->getBaseSizeExpMin());
                BaseSizeExpMax = std::min(A->getBaseSizeExpMax(), B->getBaseSizeExpMax());
                if(BaseSizeExpMin > BaseSizeExpMax) return false;
                AllowSigned = A->getAllowSigned() & B->getAllowSigned();
                AllowUnsigned = A->getAllowUnsigned() & B->getAllowUnsigned();
                if(!AllowSigned && !AllowUnsigned) return false;
                AllowInt = A->getAllowInt() & B->getAllowInt();
                AllowFloat = A->getAllowFloat() & B->getAllowFloat();
                if(!AllowInt && !AllowFloat) return false;
                AllowRvalue = A->getAllowRvalue() & B->getAllowRvalue();
                return true;
            }

            // TODO understand what kind of functions we want.
            // Maybe different kinds.
            // 1) printf-like, with their behaviour
            // 2) memset-like, with their behaviour.
            void filterFunctions(const Scope *S) const {
                Filtered.clear();
                // Any function is accepted by SS.
                if(const StatementScope *SS = dynamic_cast<const StatementScope *>(S)) {
                    for(int I = 0; I < Functions.size(); I++)
                        Filtered.push_back(I);
                } else if(const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S)) {
                    for(int I = 0; I < Functions.size(); I++) {
                        const Function &F = Functions[I];
                        if(F.ReturnType.Kind == ReturnType::Void) continue;
                        ExpressionScope *RetScope =
                            F.ReturnType.Kind == ReturnType::ByScope
                            ? F.ReturnType.Value.Scope
                            : F.Params[F.ReturnType.Value.Param.];
                        int _1, _2;
                        if(Intersect(ES, RetScope, _1, _2, _1, _2, _1, _2, _1, _1, _2))
                            Filtered.push_back(I);
                    }
                }
            }
        public:
            CallNK(std::vector<Function> Functions) : Functions(Functions) {}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // Shortness is not allowed for complex nodes.
                if(S->isShort()) return false;
                filterFunctions(S);
                if(Filtered.size() == 0) return false;

                // 

                // Get all fields from the input.
                Sizes.push_back(getSizeForRange(0, MaxPtrDepth));
                Sizes.push_back(getSizeForRange(0, MaxBaseSizeExp));
                Sizes.push_back(1); // Float/Int
                Sizes.push_back(1); // Signed/Unsigned
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                const ExpressionScope *ES = static_cast<const ExpressionScope *>(ResultScope);

                // Get result properties.
                int PtrDepthMin = ES->getPtrDepthMin();
                int PtrDepthMax = ES->getPtrDepthMax();
                int BaseSizeExpMin = ES->getPtrDepthMin();
                int BaseSizeExpMax = ES->getPtrDepthMax();
                int AllowInt = ES->getAllowInt();
                int AllowFloat = ES->getAllowFloat();
                int AllowSigned = ES->getAllowSigned();
                int AllowUnsigned = ES->getAllowUnsigned();
                // L/R value does not matter.

                // Get the actual result properties.
                int ResPtrDepth = selectInRange(PtrDepthMin, PtrDepthMax, MaxPtrDepth, Values[0]);
                bool ResFloat = selectBool(AllowFloat, AllowInt, Values[2]);
                if(ResFloat) BaseSizeExpMin = std::max(BaseSizeExpMin, 2);
                int ResBaseSizeExp = selectInRange(BaseSizeExpMin, BaseSizeExpMax, MaxBaseSizeExp, Values[1]);
                bool ResSigned = selectBool(AllowSigned, AllowUnsigned, Values[3]);

                std::string TypeName;
                if(ResFloat) {
                    switch (ResBaseSizeExp) {
                    default: throw "Unknown float type";
                    case 2: TypeName.append("float"); break;
                    case 3: TypeName.append("double"); break;
                    }
                } else {
                    if(!ResSigned) TypeName.append("un");
                    TypeName.append("signed ");
                    switch (ResBaseSizeExp) {
                    default: throw "Unknown integer type";
                    case 0: TypeName.append("char"); break;
                    case 1: TypeName.append("short"); break;
                    case 2: TypeName.append("int"); break;
                    case 3: TypeName.append("long long"); break;
                    }
                }
                while(ResPtrDepth--) TypeName.push_back('*');
                OperandsScopes.push_back(new SingleStringScope(TypeName));
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, MaxPtrDepth, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 1)
                );
            }
        };
    }
}

#endif
