#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define NOINLINE __attribute__((noipa))

NOINLINE void print_unique(long long Number) {
    printf("%lld\n", Number);
}
