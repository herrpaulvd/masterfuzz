#pragma pack(1)
template<typename T>
struct SmartPointer {
    void *Self;
    NOINLINE SmartPointer() : Self(0) {}
    NOINLINE SmartPointer(void *Self) : Self(Self) {}
    NOINLINE static SmartPointer<T> capture(T *Ptr) {
        return SmartPointer<T>(Ptr);
    }
    template<typename INT> NOINLINE static SmartPointer<T> alloc(INT Count) {
        return SmartPointer<T>(new T[Count]());
    }

    NOINLINE void destroySingle() {delete (T *)Self;}

    NOINLINE void destroyMany() {delete [] (T *)Self;}

    NOINLINE T &value() {return *(T *)Self;}

    template<typename INT> NOINLINE SmartPointer<T> operator+(INT Count) const {
        SmartPointer<T> Result = *this;
        long long *Ptr = (long long *)(&Result.Self);
        *Ptr += Count * sizeof(T);
        return Result;
    }

    template<typename INT> NOINLINE SmartPointer<T> operator-(INT Count) const {
        SmartPointer<T> Result = *this;
        long long *Ptr = (long long *)(&Result.Self);
        *Ptr -= Count * sizeof(T);
        return Result;
    }

    // postfix
    NOINLINE SmartPointer<T> operator++(int) {
        SmartPointer<T> Result = *this;
        unsigned long long *Ptr = (unsigned long long *)(Self);
        *Ptr += sizeof(T);
        return Result;
    }

    // prefix
    NOINLINE SmartPointer<T>& operator++() {
        unsigned long long *Ptr = (unsigned long long *)(Self);
        *Ptr += sizeof(T);
        return *this;
    }

    // postfix
    NOINLINE SmartPointer<T> operator--(int) {
        SmartPointer<T> Result = *this;
        unsigned long long *Ptr = (unsigned long long *)(Self);
        *Ptr -= sizeof(T);
        return Result;
    }

    // prefix
    NOINLINE SmartPointer<T>& operator--() {
        unsigned long long *Ptr = (unsigned long long *)(Self);
        *Ptr -= sizeof(T);
        return *this;
    }

    template<typename INT> NOINLINE SmartPointer<T>(INT value) : Self((void *)value) {}

    template<typename INT> NOINLINE T& operator[](INT Index) {
        return ((T*)Self)[Index];
    }

    NOINLINE T& operator*() {
        return *((T*)Self);
    }

    NOINLINE SmartPointer<SmartPointer<T>> operator&() {
        return SmartPointer<SmartPointer<T>>(this);
    }

    NOINLINE operator bool() const {return Self;}

    NOINLINE explicit operator unsigned long long() const {
        return (unsigned long long)Self;
    }

    NOINLINE operator char*() const {
        return (char *)Self;
    }

    NOINLINE operator void*() const {
        return Self;
    }

    NOINLINE operator wchar_t*() const {
        return (wchar_t *)Self;
    }

    #define OPERATOR_COMPARE(op) \
        NOINLINE bool operator op(const SmartPointer<T> &Other) { \
            return Self == Other.Self; \
        }

    OPERATOR_COMPARE(==)
    OPERATOR_COMPARE(!=)
    OPERATOR_COMPARE(<)
    OPERATOR_COMPARE(>)
    OPERATOR_COMPARE(<=)
    OPERATOR_COMPARE(>=)
};
#pragma pack()

template<typename X, typename Z> NOINLINE SmartPointer<X> uref_smart(X& x) {
    return SmartPointer<X>(&x);
}
