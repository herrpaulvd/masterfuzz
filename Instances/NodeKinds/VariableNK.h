#ifndef INK_VARIABLENK_H
#define INK_VARIABLENK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <cassert>
#include <initializer_list>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        struct Variable {
            int PtrDepth, BaseSizeExp;
            bool Signed : 1;
            bool ReadOnly : 1;
            bool Float : 1;
            Scope *S;

            Variable() {}
            Variable(const std::string &Name, int PtrDepth, int BaseSizeExp,
                bool Signed, bool Float, bool ReadOnly)
                : PtrDepth(PtrDepth), BaseSizeExp(BaseSizeExp),
                Signed(Signed), ReadOnly(ReadOnly), Float(Float),
                S(new SingleStringScope(Name, false)) {}
        };
        
        class VariableNK : public ASTNodeKind {
        private:
            std::vector<Variable> Variables;
            int ParamWidth;
            mutable std::vector<int> Filtered;

            void filterVariables(const Scope *S) const {
                Filtered.clear();
                const ExpressionScope *ES =
                    dynamic_cast<const ExpressionScope *>(S);
                if(!ES) return;
                for(int I = 0; I < Variables.size(); I++) {
                    const Variable &V = Variables[I];
                    if(
                        (!V.ReadOnly || ES->getAllowRvalue())

                        && ES->getPtrDepthMin() <= V.PtrDepth
                        && V.PtrDepth <= ES->getPtrDepthMax()

                        && ES->getBaseSizeExpMin() <= V.BaseSizeExp
                        && V.BaseSizeExp <= ES->getBaseSizeExpMax()

                        && (V.Signed || ES->getAllowUnsigned())
                        && (!V.Signed || ES->getAllowSigned())

                        && (V.Float || ES->getAllowInt())
                        && (!V.Float || ES->getAllowFloat())
                    ) Filtered.push_back(I);
                }
            }

        public:
            VariableNK(const std::vector<Variable> &Variables)
                : Variables(Variables) {}
            VariableNK(Variable V) : Variables() {
                Variables.push_back(V);
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                filterVariables(S);
                if(Filtered.size() == 0) return false;
                Sizes.push_back(getSizeForRange(Variables.size()));
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                filterVariables(ResultScope);
                OperandsScopes.push_back(Variables[selectInSet(Filtered, Variables.size(), Values[0])].S);
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startVariable();
                    break;
                case 1:
                    SP->endVariable();
                    break;
                }
            }
        };
    }
}

#endif
