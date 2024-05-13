#ifndef INK_SPECIALCONSTNK_H
#define INK_SPECIALCONSTNK_H

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
        class SpecialConstNK : public ASTNodeKind {
        private:
            SpecialConstNK() {}

            static std::string buildConst(int SizeExp, bool Signed, bool Max) {
                std::string Result;
                if(!Signed) {
                    if(!Max) return "0";
                    Result.push_back('U');   
                }
                switch (SizeExp) {
                default: throw "Unknown size exp";
                case 0:
                    Result.append("CHAR");
                    break;
                case 1:
                    Result.append("SHRT");
                    break;
                case 2:
                    Result.append("INT");
                    break;
                case 3:
                    Result.append("LLONG");
                    break;
                }
                Result.append(Max ? "_MAX" : "_MIN");
                return Result;
            }
        public:
            static SpecialConstNK *get() {
                static SpecialConstNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S);
                if(!ES || !ES->getAllowRvalue() || ES->getPtrDepthMin() > 0 || !ES->getAllowInt()) return false;
                Sizes.push_back(getSizeForRange(0, MaxBaseSizeExp));
                Sizes.push_back(1); // Float/Int (not read, just for similarity to ConstantNK).
                Sizes.push_back(1); // Signed/Unsigned
                // _MIN/_MAX/1/0
                Sizes.push_back(2);
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                const ExpressionScope *ES = static_cast<const ExpressionScope *>(ResultScope);
                // Get the constant params.
                int BaseSizeExp = selectInRange(ES->getBaseSizeExpMin(), ES->getBaseSizeExpMax(), MaxBaseSizeExp, Values[0]);
                bool Signed = selectBool(ES->getAllowSigned(), ES->getAllowUnsigned(), Values[2]);
                int ConstType = Values[3] & 0b11;

                std::string Result;
                switch(ConstType) {
                case 0:
                    Result.push_back('0');
                    break;
                case 1:
                    Result.push_back('1');
                    break;
                case 2:
                    Result = buildConst(BaseSizeExp, Signed, false);
                    break;
                case 3:
                    Result = buildConst(BaseSizeExp, Signed, true);
                    break;
                }
                
                OperandsScopes.push_back(new SingleStringScope(Result, true));
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startConst();
                    break;
                case 1:
                    SP->endConst();
                    break;
                }
            }
        };
    }
}

#endif
