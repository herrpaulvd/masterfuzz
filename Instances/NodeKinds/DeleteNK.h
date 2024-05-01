#ifndef INK_DELETENK_H
#define INK_DELETENK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class DeleteNK : public ASTNodeKind {
        private:
            DeleteNK() {}
        public:
            static DeleteNK *get() {
                static DeleteNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // No fields needed.
                return !S->isShort() && dynamic_cast<const StatementScope *>(S);
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {

                // Single operand - any lvalue ptr.
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        1, MaxPtrDepth,
                        0, MaxBaseSizeExp,
                        1, 1, 0,
                        1, 1
                    )
                );
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startDelete();
                    break;
                case 1:
                    SP->endDelete();
                    break;
                }
            }
        };
    }
}

#endif
