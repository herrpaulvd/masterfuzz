#ifndef DECODERBASE_UTILS_H
#define DECODERBASE_UTILS_H

#include <string>
#include <vector>
namespace decoder {
    const int ByteSize = 8;
    const int IntSize = ByteSize * 4;

    const int MaxBaseSizeExp = 3;
    const int MaxPtrDepth = 3;

    int getSizeForRange(int RangeWidth);
    int getSizeForRange(int Min, int Max);
    int selectInRange(int Min, int Max, int AbsMax, int Seed);
    int selectInSet(const std::vector<int> &Set, int AbsMax, int Seed);
    bool selectBool(int AllowTrue, int AllowFalse, int Seed);
    int getByteSize(int BitSize);
    bool less(int Left, int Right, bool &Result);

    std::string makeTypeName(int PtrDepth, int BaseSizeExp, bool Float, bool Signed, bool Const);
}

#endif
