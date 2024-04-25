#ifndef INK_INDEXNK_H
#define INK_INDEXNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Scopes/ExpressionScope.h"
#include <algorithm>
#include <vector>

using namespace decoder;
using namespace instances::scopes;

namespace instances {
    namespace nodekinds {
        class IndexNK : ASTNodeKind {
        private:
            IndexNK() {}
        public:
            static IndexNK *get() {
                static IndexNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S);
                // Shortness is not allowed for complex nodes.
                if(!ES || ES->isShort()) return false;

                // Cannot return max high-depth ptr,
                // because indexable will exceed the limit.
                if(ES->getPtrDepthMin() == MaxPtrDepth)
                    return false;
                // Any base size exp is allowed in any cases.
                // Any signess too.
                
                // Get all fields from the input.
                Sizes.push_back(getSizeForRange(0, MaxPtrDepth));
                Sizes.push_back(getSizeForRange(0, MaxBaseSizeExp));
                Sizes.push_back(1); // Float/Int
                Sizes.push_back(1); // Signed/Unsigned
                // L/R value does not matter.
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

                if(PtrDepthMax == MaxPtrDepth) PtrDepthMax--;

                // Get the actual result properties.
                int ResPtrDepth = selectInRange(PtrDepthMin, PtrDepthMax, MaxPtrDepth, Values[0]);
                bool ResFloat = selectBool(AllowFloat, AllowInt, Values[2]);
                if(ResFloat) BaseSizeExpMin = std::max(BaseSizeExpMin, 2);
                int ResBaseSizeExp = selectInRange(BaseSizeExpMin, BaseSizeExpMax, MaxBaseSizeExp, Values[1]);
                bool ResSigned = selectBool(AllowSigned, AllowUnsigned, Values[3]);
                AllowSigned = ResSigned ? 1 : 0;
                AllowUnsigned = 1 - AllowSigned;
                AllowFloat = ResFloat ? 1 : 0;
                AllowInt = 1 - AllowFloat;

                // Left scope is same as result scope except for PtrDepth++.
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        ResPtrDepth + 1, ResPtrDepth + 1, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat)
                );
                // Right scope is any integer.
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, 0, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 0)
                );
            }
        };
    }
}

#endif
