#ifndef INK_SINGLESTRINGNK_H
#define INK_SINGLESTRINGNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"

using namespace decoder;

namespace instances {
    namespace nodekinds {
        class SingleStringNK : ASTNodeKind {
        private:
            std::string S;
        public:
            SingleStringNK(const std::string &S) : S(S) {}

            const std::string &getString() const {return S;}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const {
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const {}
        };
    }
}

#endif
