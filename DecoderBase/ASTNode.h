#ifndef DECODERBASE_ASTNODE_H
#define DECODERBASE_ASTNODE_H

#include "ASTNodeKind.h"
#include "Scope.h"
#include <vector>

namespace decoder {
    class ASTNode {
    private:
        const ASTNodeKind *Kind;
        std::vector<const ASTNode *> Children;
    public:
        ASTNode(const ASTNodeKind *Kind,
            const std::vector<const ASTNode *> &Children)
            : Kind(Kind), Children(Children) {}
        
        void print(Printer *P) const {
            int Count = Children.size();
            for(int I = 0; I < Count; I++) {
                Kind->print(P, I);
                Children[I]->print(P);
            }
            Kind->print(P, Count);
        }
    };
}

#endif
