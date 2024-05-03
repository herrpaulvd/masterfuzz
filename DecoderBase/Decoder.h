#ifndef DECODERBASE_DECODER_H
#define DECODERBASE_DECODER_H

#include "ASTNode.h"
#include "ASTNodeKind.h"
#include "ByteStream.h"
#include "Scope.h"
#include <vector>

namespace decoder {
    class Decoder {
    private:
        Scope *Initial;
        std::vector<ASTNodeKind *> AllKinds;
        ASTNodeKind *Stub;
        ASTNode *GenerateNode(ByteStream *Stream, Scope *CurrentScope);
    public:
        Decoder(Scope *Initial, const std::vector<ASTNodeKind *> &AllKinds, ASTNodeKind *Stub)
            : Initial(Initial), AllKinds(AllKinds), Stub(Stub) {}

        ASTNode *GenerateAST(ByteStream *Stream) {
            return GenerateNode(Stream, Initial);
        }
    };
}

#endif
