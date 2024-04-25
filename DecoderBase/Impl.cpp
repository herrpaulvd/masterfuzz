#include "ASTNodeKind.h"
#include "Decoder.h"
#include "Utils.h"

#include "ASTNode.h"
#include "ByteStream.h"
#include "Scope.h"
#include <vector>

using namespace decoder;

int decoder::getSizeForRange(int RangeWidth) {
    int Size = 0;
    while(RangeWidth >= (1 << (Size + 1))) Size++;
    return Size;
}

int decoder::getSizeForRange(int Min, int Max) {return getSizeForRange(Max - Min + 1);}

int decoder::getByteSize(int BitSize) {
    return BitSize / ByteSize + (BitSize % ByteSize ? 1 : 0);
}

int decoder::selectInRange(int Min, int Max, int AbsMax, int Seed) {
    AbsMax++;
    return (Seed - Min + AbsMax) % AbsMax % (Max - Min + 1) + Min;
}

int decoder::selectInSet(const std::vector<int> &Set, int AbsMax, int Seed) {
    int CorrectedSeed = Seed;
    AbsMax++;
    for(int E : Set) {
        if(E == Seed % AbsMax) return E;
    }
    return Set[Seed % Set.size()];
}

bool decoder::selectBool(int AllowTrue, int AllowFalse, int Seed) {
    if(!AllowTrue) return false;
    if(!AllowFalse) return true;
    return (Seed & 1);
}

bool decoder::less(int Left, int Right, bool &Result) {
    if(Left == Right) return false;
    Result = Left < Right;
    return true;
}

ASTNode *Decoder::GenerateNode(ByteStream *Stream, Scope *CurrentScope) {
    // Get available kinds list.
    CurrentScope = CurrentScope->changeShortness(!Stream->isAlive());
    ScopeCache Cache;
    std::vector<int> Sizes;
    if(CurrentScope->hasCache())
        Cache = CurrentScope->getCache();
    else {
        for(int I = 0; I < AllKinds.size(); I++) {
            ASTNodeKind *Kind = AllKinds[I];
            Sizes.clear();
            if(Kind->getInfoFields(CurrentScope, Sizes)) {
                Cache.SelectedKinds.push_back(I);
                Cache.InfoFields.push_back(Sizes);
            }
        }
        CurrentScope->setCache(Cache);
    }

    // Select a kind via input. Use min amount of bytes for it, possibly 0.
    int ChoiceSeed;
    ASTNodeKind *CurrentKind;
    if(Cache.Single) {
        ChoiceSeed = 0;
        CurrentKind = Cache.Single;
    } else {
        int KindsCount = AllKinds.size();
        int BitsNeeded = getSizeForRange(KindsCount);
        int BytesNeeded = getByteSize(BitsNeeded);
        // The amount of ast node kinds is definetly less than INT_MAX.
        ChoiceSeed = 0;
        Stream->readBytes(BytesNeeded, &ChoiceSeed);
        ChoiceSeed = selectInSet(Cache.SelectedKinds, KindsCount - 1, ChoiceSeed);
        CurrentKind = AllKinds[ChoiceSeed];
    }
    std::vector<int> &CurrentInfoFields = Cache.InfoFields[ChoiceSeed];

    // Get actual info fields from input.
    std::vector<int> ReadInfoFields;
    int BitsRemained = 0;
    char ReadByte = 0;
    for(int FieldSize : CurrentInfoFields) {
        int Value;
        if(FieldSize > ByteSize) {
            int FieldSizeInBytes = getByteSize(FieldSize);
            Stream->readBytes(FieldSizeInBytes, &Value);
            BitsRemained = 0;
        } else {
            if(FieldSize > BitsRemained) {
                Stream->read(ReadByte);
                BitsRemained = ByteSize;
            }
            Value = ReadByte;
            ReadByte >>= FieldSize;
            BitsRemained -= FieldSize;
        }
        Value &= ((1 << FieldSize) - 1);
        ReadInfoFields.push_back(Value);
    }

    // Get children scopes.
    std::vector<Scope *> ChildScopes;
    CurrentKind->getOperandsScopes(CurrentScope, ReadInfoFields, ChildScopes);

    // Build children nodes.
    std::vector<const ASTNode *> ChildNodes;
    for(Scope *ChildScope : ChildScopes)
        ChildNodes.push_back(GenerateNode(Stream, ChildScope));

    // Build and return the result.
    return new ASTNode(CurrentKind, ChildNodes);
}
