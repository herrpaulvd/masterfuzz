#ifndef INK_WHILENK_H
#define INK_WHILENK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <utility>
#include <vector>

using namespace decoder;
using namespace instances::scopes;

namespace instances {
    namespace nodekinds {
        class WhileNK : ASTNodeKind {
        private:
            bool DoWhile;
            // For smaller bit amount needed, it's better to create
            // two instances of the NK without one asking for a bit to
            // determine whether we have else or not.
            WhileNK(bool DoWhile) : DoWhile(DoWhile) {}
        public:
            static WhileNK *get(bool IsDoWhile) {
                static WhileNK While(false);
                static WhileNK DoWhile(true);
                return IsDoWhile ? &DoWhile : &While;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // No fields needed because loop type depends
                // on the NK instance properties.
                return !S->isShort() && dynamic_cast<const StatementScope *>(S);
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // For any instance, children scopes are fixed.
                // They order is different, though.

                static Scope *Condition = ExpressionScope::get(
                    0, MaxPtrDepth,
                    0, MaxBaseSizeExp,
                    1, 1, 1,
                    1, 1
                );

                static Scope *Body = StatementScope::getLarge();

                OperandsScopes.push_back(Condition);
                OperandsScopes.push_back(Body);
                if(DoWhile) std::swap(OperandsScopes[0], OperandsScopes[1]);
            }
        };
    }
}

#endif
