#ifndef INK_CONSTANTNK_H
#define INK_CONSTANTNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <cassert>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        // Here, ptr consts are not allowed.
        // They will be simulated via so-inited variables.
        class ConstantNK : ASTNodeKind {
        public:
            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S);
                if(!ES || !ES->getAllowRvalue() || ES->getPtrDepthMin() > 0) return false;
                Sizes.push_back(getSizeForRange(0, MaxBaseSizeExp));
                Sizes.push_back(1); // Float/Int
                Sizes.push_back(1); // Signed/Unsigned
                // For parameters int is used.
                // But, the const can be long long
                // If it's ll, divide it into two ints.
                int ConstSize = ByteSize << ES->getBaseSizeExpMax();
                while(ConstSize > IntSize) {
                    Sizes.push_back(IntSize);
                    ConstSize -= IntSize;
                }
                Sizes.push_back(ConstSize);
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                const ExpressionScope *ES = static_cast<const ExpressionScope *>(ResultScope);
                // Get the constant params.
                int BaseSize = ByteSize << selectInRange(ES->getBaseSizeExpMin(), ES->getBaseSizeExpMax(), MaxBaseSizeExp, Values[0]);
                bool Float = selectBool(ES->getAllowFloat(), ES->getAllowInt(), Values[1]);
                bool Signed = Float || selectBool(ES->getAllowSigned(), ES->getAllowUnsigned(), Values[2]);
                long long Result = Values[3];
                if(BaseSize > IntSize) Result |= ((long long)Values[4] << IntSize);
                Result &= ((1LL << BaseSize) - 1);
                
                // Alternative interpretations.
                unsigned long long &UResult = *(unsigned long long *)(&Result);
                float &FResult = *(float *)(&Result);
                double &DResult = *(double *)(&Result);

                // Convert Result to string and return it via SSS.
                std::string S;
                if(Float)
                    S = BaseSize > IntSize ? std::to_string(FResult) : std::to_string(DResult);
                else
                    S = Signed ? std::to_string(Result) : std::to_string(UResult);

                if(Float) {
                    if(BaseSize == IntSize) {
                        if(S.find('.') == S.npos)
                            S.push_back('.');
                        S.push_back('F');
                    }
                } else {
                    if(!Signed)
                        S.push_back('U');
                    if(BaseSize > IntSize) {
                        S.push_back('L');
                        S.push_back('L');
                    }
                }
                
                OperandsScopes.push_back(new SingleStringScope(S));
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    // Let printer know that the next arg is a const.
                    SP->setParentInfo(ParentInfo::StringConst);
                    break;
                case 1:
                    SP->clearParentInfo();
                    break;
                }
            }
        };
    }
}

#endif
