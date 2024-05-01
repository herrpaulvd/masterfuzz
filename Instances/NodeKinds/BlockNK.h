#ifndef INK_BLOCKNK_H
#define INK_BLOCKNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/StatementScope.h"
#include <cassert>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        class BlockNK : public ASTNodeKind {
        private:
            BlockNK() {}
        public:
            static BlockNK *get() {
                static BlockNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                if(const StatementScope *SS = dynamic_cast<const StatementScope *>(S)) {
                    Sizes.push_back(1); // 2-3 substatements.
                    // Always 0 if short.
                    return true;
                }
                return false;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // Always 0 if short.
                if(ResultScope->isShort()) return;
                // Otherwise seed + 2
                int ChildrenCount = (Values[0] & 1) + 2;
                while(ChildrenCount--) OperandsScopes.push_back(StatementScope::getLarge());
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                if(Part == 0)
                    SP->startBlock();
                if(Last)
                    SP->endBlock();
            }
        };
    }
}

#endif
