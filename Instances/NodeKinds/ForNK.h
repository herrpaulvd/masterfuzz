#ifndef INK_FORNK_H
#define INK_FORNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <cassert>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class ForNK : public ASTNodeKind {
        private:
            ForNK() {}
        public:
            static ForNK *get() {
                static ForNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                return !S->isShort() && dynamic_cast<const StatementScope *>(S);
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // Children scopes are fixed.

                static Scope *Condition = ExpressionScope::get(
                    0, MaxPtrDepth,
                    0, MaxBaseSizeExp,
                    1, 1, 1,
                    1, 1
                );

                static Scope *Body = StatementScope::getLarge();

                OperandsScopes.push_back(Condition); //x;;
                OperandsScopes.push_back(Condition); //;x;
                OperandsScopes.push_back(Condition); // ;;x
                OperandsScopes.push_back(Body);
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startFor();
                    break;
                case 1:
                    SP->betweenForInitAndCondition();
                    break;
                case 2:
                    SP->betweenForConditionAndStep();
                    break;
                case 3:
                    SP->betweenForStepAndBody();
                    break;
                case 4:
                    SP->endFor();
                    break;
                }
            }
        };
    }
}

#endif
