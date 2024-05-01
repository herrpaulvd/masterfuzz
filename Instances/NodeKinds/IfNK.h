#ifndef INK_IFNK_H
#define INK_IFNK_H

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
        class IfNK : public ASTNodeKind {
        private:
            bool HasElse;
            // For smaller bit amount needed, it's better to create
            // two instances of the NK without one asking for a bit to
            // determine whether we have else or not.
            IfNK(bool HasElse) : HasElse(HasElse) {}
        public:
            static IfNK *get(bool HasElse) {
                static IfNK If(false);
                static IfNK IfElse(true);
                return HasElse ? &IfElse : &If;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // No fields needed because else branch existence depends
                // on the NK instance properties.
                return !S->isShort() && dynamic_cast<const StatementScope *>(S);
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // For a certain instance, children scopes are fixed.
                // First, the condition scope is any expression.
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, MaxPtrDepth,
                        0, MaxBaseSizeExp,
                        1, 1, 1,
                        1, 1
                    )
                );
                // Then if-branch
                OperandsScopes.push_back(StatementScope::getLarge());
                // Then else-branch if exists
                if(HasElse)
                    OperandsScopes.push_back(StatementScope::getLarge());
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startIf();
                    break;
                case 1:
                    SP->startIfBody();
                    break;
                case 2:
                    SP->endIfBody();
                    if(HasElse)
                        SP->startElse();
                    break;
                case 3:
                    if(!HasElse) throw "Has no else branch";
                    SP->endElse();
                    break;
                }
            }
        };
    }
}

#endif
