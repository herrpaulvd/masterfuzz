#ifndef INK_FORNK_H
#define INK_FORNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/FormatStringScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

using namespace decoder;
using namespace instances::scopes;

namespace instances {
    namespace nodekinds {
        class ForNK : ASTNodeKind {
        // No fields because no variants.
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
        };
    }
}

#endif
