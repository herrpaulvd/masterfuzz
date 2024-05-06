#include <cstddef>
#include <iostream>

using namespace std;

#define NOINLINE __attribute__((noinline))
// noipa?

// for guaranteed operations do NOINLINE functions like this:
template<typename A, typename B, typename C> NOINLINE
C pluss(A Left, B Right) {
    return Left + Right;
}

void f() {
    int x = 3;
    long y = 4;
    std::cout << pluss<typeof(x), typeof(y), typeof(x + y)>(x, y);

    // IDEA: make temp variables through auto
    // +:
    // start: push @VL
    // left: emit auto @VL = ...;
    // middle: push @VR
    // right emit auto @VR = ...;
    // end: emit auto @GET_PUSHED() funcall<typeof(@VL), typeof(@VR), typeof(@VL @SIGN @VR)>(@VL, @VR)

    // same for statements.
    // e.g. if: first, emit condition like @COND = ..., then if(@COND)
    // while is a bit different, emit while(true), then break check:
    // start: push @COND; emit while(true) {
    // condition: emit auto @COND = ...;
    // middle: emit break
    // body: emit body
    // end: emit }
    // for do-while and for in the same manner. For maybe unroll to while.
}

// replacing: just check replaced version

// instead of unqiue string: unqiue function call in asm?

// TODO: write full scenario for every SC option.

// for guaranteed ptr optimizations disabling.
template<typename T>
class SmartPointer {
private:
    void *Self;
    NOINLINE SmartPointer(void *Self) : Self(Self) {}
public:
    NOINLINE static SmartPointer<T> capture(T *Ptr) {
        return SmartPointer<T>(Ptr);
    }
    NOINLINE static SmartPointer<T> alloc(long long Count) {
        return SmartPointer<T>(new T[Count]);
    }
    NOINLINE static SmartPointer<T> alloc(unsigned long long Count) {
        return SmartPointer<T>(new T[Count]);
    }
    NOINLINE void destroySingle() {delete (T *)Self;}

    NOINLINE void destroyMany() {delete [] (T *)Self;}

    NOINLINE T &value() {return *(T *)Self;}

    NOINLINE SmartPointer<T> operator+(long long Count) const {
        SmartPointer<T> Result = *this;
        long long *Ptr = (long long *)(&Result.Self);
        *Ptr += Count;
        return Result;
    }

    NOINLINE SmartPointer<T> operator-(long long Count) const {
        SmartPointer<T> Result = *this;
        long long *Ptr = (long long *)(&Result.Self);
        *Ptr -= Count;
        return Result;
    }

    NOINLINE SmartPointer<T> operator+(unsigned long long Count) const {
        SmartPointer<T> Result = *this;
        unsigned long long *Ptr = (unsigned long long *)(&Result.Self);
        *Ptr += Count;
        return Result;
    }

    NOINLINE SmartPointer<T> operator-(unsigned long long Count) const {
        SmartPointer<T> Result = *this;
        unsigned long long *Ptr = (unsigned long long *)(&Result.Self);
        *Ptr -= Count;
        return Result;
    }

    NOINLINE operator bool() const {return Self;}

    NOINLINE explicit operator unsigned long long() const {
        return (unsigned long long)Self;
    }
};

int main() {

}