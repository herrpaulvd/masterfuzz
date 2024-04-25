#ifndef DECODERBASE_ASTNODEKIND_H
#define DECODERBASE_ASTNODEKIND_H

#include "Printer.h"
#include "Scope.h"
#include <vector>

namespace decoder {
    class ASTNodeKind {
    public:
        virtual bool getInfoFields(const Scope *S, std::vector<int> &Sizes)
            const = 0;
        virtual void getOperandsScopes(const Scope *ResultScope,
            const std::vector<int> &Values,
            std::vector<Scope *> &OperandsScopes) const = 0;
        virtual void print(Printer *P, int Part) const = 0;
    };
}

#endif
