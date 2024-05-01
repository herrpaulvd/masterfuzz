#ifndef INK_CASTNK_H
#define INK_CASTNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <algorithm>
#include <cassert>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class CastNK : public ASTNodeKind {
        private:
            CastNK() {}
        public:
            static CastNK *get() {
                static CastNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S);
                // Shortness is not allowed for complex nodes.
                // Lvalue not allowed too.
                if(!ES || ES->isShort() || !ES->getAllowRvalue())
                    return false;

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

                OperandsScopes.push_back(new SingleStringScope(makeTypeName(ResPtrDepth, ResBaseSizeExp, ResFloat, ResSigned, false), true));
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, MaxPtrDepth, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 1)
                );
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    // Let printer know that the next arg is a cast.
                    SP->setParentInfo(ParentInfo::StringCast);
                    break;
                case 1:
                    // Reset info for the next arg child.
                    SP->clearParentInfo();
                    SP->setParentInfo(ParentInfo::Expression);
                    break;
                case 2:
                    SP->clearParentInfo();
                    SP->endCast();
                    break;
                }
            }
        };
    }
}

#endif
