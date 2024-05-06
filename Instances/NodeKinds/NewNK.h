#ifndef INK_NEWNK_H
#define INK_NEWNK_H

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
        class NewNK : public ASTNodeKind {
        private:
            NewNK() {}
        public:
            static NewNK *get() {
                static NewNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S);
                // Shortness is not allowed for complex nodes.
                // Lvalue is not allowed.
                // It returns ptr.
                if(!ES || ES->isShort() || !ES->getAllowRvalue() || ES->getPtrDepthMax() == 0)
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
                int PtrDepthMin = std::max(ES->getPtrDepthMin(), 1);
                int PtrDepthMax = ES->getPtrDepthMax();
                int BaseSizeExpMin = ES->getBaseSizeExpMin();
                int BaseSizeExpMax = ES->getBaseSizeExpMax();
                bool AllowInt = ES->getAllowInt();
                bool AllowFloat = ES->getAllowFloat();
                bool AllowSigned = ES->getAllowSigned();
                bool AllowUnsigned = ES->getAllowUnsigned();
                // L/R value does not matter.

                // Get the actual result properties.
                int ResPtrDepth = selectInRange(PtrDepthMin, PtrDepthMax, MaxPtrDepth, Values[0]);
                bool ResFloat = selectBool(AllowFloat, AllowInt, Values[2]);
                if(ResFloat) BaseSizeExpMin = std::max(BaseSizeExpMin, 2);
                int ResBaseSizeExp = selectInRange(BaseSizeExpMin, BaseSizeExpMax, MaxBaseSizeExp, Values[1]);
                bool ResSigned = selectBool(AllowSigned, AllowUnsigned, Values[3]);

                OperandsScopes.push_back(new SingleStringScope(makeTypeName(ResPtrDepth - 1, ResBaseSizeExp, ResFloat, ResSigned, false), true));
                // Any integer.
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, 0, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 0)
                );
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startNew();
                    break;
                case 1:
                    SP->betweenNewTypePartAndSize();
                    break;
                case 2:
                    SP->endNew();
                    break;
                }
            }
        };
    }
}

#endif
