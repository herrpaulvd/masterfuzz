#ifndef INK_SINGLESTRINGNK_H
#define INK_SINGLESTRINGNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "Instances/Printers/SimplePrinter.h"
#include <cassert>
#include <string>

using namespace decoder;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class SingleStringNK : public ASTNodeKind {
        private:
            std::string S;
        public:
            SingleStringNK(const std::string &S) : S(S) {}

            const std::string &getString() const {return S;}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const override {
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {}
            
            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);
                SP->emitSingleString(S);
            }
        };
    }
}

#endif
