#ifndef INK_PROGRAMNK_H
#define INK_PROGRAMNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/GlobalScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <cassert>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class ProgramNK : public ASTNodeKind {
        private:
            std::vector<std::string> Header;
            std::vector<std::string> Footer;
        public:
            ProgramNK(const std::vector<std::string> &Header, const std::vector<std::string> &Footer)
                : Header(Header), Footer(Footer) {}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                return dynamic_cast<const GlobalScope *>(S);
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                OperandsScopes.push_back(StatementScope::getLarge());
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                if(Part == 0) {
                    SP->printParts(Header);
                    SP->pusht();
                    SP->setParentInfo(ParentInfo::Statement);
                } else {
                    SP->printParts(Footer);
                    SP->popt();
                    SP->clearParentInfo();
                }
            }
        };
    }
}

#endif