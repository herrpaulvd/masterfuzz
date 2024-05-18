#ifndef INK_STUBNK_H
#define INK_STUBNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/NodeKinds/SingleStringNK.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class StubNK : public ASTNodeKind {
        private:
            StubNK() {}
        public:
            static StubNK *get() {
                static StubNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // The method will not be called.
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // It must be needed only for ES.
                const ExpressionScope *ES = static_cast<const ExpressionScope *>(ResultScope);

                // Get result properties = actual properties.
                int PtrDepth = ES->getPtrDepthMin();
                int BaseSizeExp = ES->getBaseSizeExpMin();
                bool AllowInt = ES->getAllowInt();
                bool AllowFloat = !AllowInt && ES->getAllowFloat();
                bool AllowUnsigned = ES->getAllowUnsigned();
                bool AllowSigned = !AllowUnsigned && ES->getAllowSigned();
                bool AllowRvalue = ES->getAllowRvalue();

                std::string NewExpr;
                NewExpr.push_back(AllowRvalue ? 'R' : 'L');
                NewExpr.append("valueStub<");
                NewExpr.append(makeTypeName(PtrDepth, BaseSizeExp, AllowFloat, AllowSigned, false));
                NewExpr.append(">()");
                OperandsScopes.push_back(new SingleStringScope(NewExpr, true));
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startStub();
                    break;
                case 1:
                    SP->endStub();
                    break;
                }
            }
        };
    }
}

#endif
