#ifndef INK_CASTNK_H
#define INK_CASTNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <algorithm>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;

namespace instances {
    namespace nodekinds {
        class CastNK : ASTNodeKind {
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

                std::string TypeName;
                if(ResFloat) {
                    switch (ResBaseSizeExp) {
                    default: throw "Unknown float type";
                    case 2: TypeName.append("float"); break;
                    case 3: TypeName.append("double"); break;
                    }
                } else {
                    if(!ResSigned) TypeName.append("un");
                    TypeName.append("signed ");
                    switch (ResBaseSizeExp) {
                    default: throw "Unknown integer type";
                    case 0: TypeName.append("char"); break;
                    case 1: TypeName.append("short"); break;
                    case 2: TypeName.append("int"); break;
                    case 3: TypeName.append("long long"); break;
                    }
                }
                while(ResPtrDepth--) TypeName.push_back('*');
                OperandsScopes.push_back(new SingleStringScope(TypeName));
                OperandsScopes.push_back(
                    ExpressionScope::get(
                        0, MaxPtrDepth, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 1)
                );
            }
        };
    }
}

#endif
