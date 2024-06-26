#define FloatOrInt 1
#define IntOnly 0
#define Prefix 0
#define Infix 0
#define Suffix 1

OPERATION("+", FloatOrInt, Prefix, ArithmeticUnary)
OPERATION("-", FloatOrInt, Prefix, ArithmeticUnary)
OPERATION("~", IntOnly, Prefix, ArithmeticUnary)
OPERATION("!", IntOnly, Prefix, LogicalUnary)
SIDEEFFECT(OPERATION("++", FloatOrInt, Prefix, IncDecUnary))
SIDEEFFECT(OPERATION("++", FloatOrInt, Suffix, IncDecUnary))
SIDEEFFECT(OPERATION("--", FloatOrInt, Prefix, IncDecUnary))
SIDEEFFECT(OPERATION("--", FloatOrInt, Suffix, IncDecUnary))
MEMORY(OPERATION("&", FloatOrInt, Prefix, ReferenceUnary))
MEMORY(OPERATION("*", FloatOrInt, Prefix, DereferenceUnary))
OPERATION("+", FloatOrInt, Infix, ArithmeticBinary)
OPERATION("-", FloatOrInt, Infix, ArithmeticBinary)
OPERATION("*", FloatOrInt, Infix, ArithmeticBinary)
DANGEROUS(OPERATION("/", FloatOrInt, Infix, ArithmeticBinary))
DANGEROUS(OPERATION("%", IntOnly, Infix, ArithmeticBinary))
OPERATION("&", IntOnly, Infix, ArithmeticBinary)
OPERATION("|", IntOnly, Infix, ArithmeticBinary)
OPERATION("^", IntOnly, Infix, ArithmeticBinary)
SHIFT(OPERATION("<<", IntOnly, Infix, ArithmeticBinary))
SHIFT(OPERATION(">>", IntOnly, Infix, ArithmeticBinary))
OPERATION("+", FloatOrInt, Infix, PtrBinary)
OPERATION("-", FloatOrInt, Infix, PtrBinary)
OPERATION("==", FloatOrInt, Infix, ComparisonBinary)
OPERATION("!=", FloatOrInt, Infix, ComparisonBinary)
OPERATION("<", FloatOrInt, Infix, ComparisonBinary)
OPERATION(">", FloatOrInt, Infix, ComparisonBinary)
OPERATION("<=", FloatOrInt, Infix, ComparisonBinary)
OPERATION(">=", FloatOrInt, Infix, ComparisonBinary)
OPERATION("&&", FloatOrInt, Infix, LogicalBinary)
OPERATION("||", FloatOrInt, Infix, LogicalBinary)
SIDEEFFECT(OPERATION("=", FloatOrInt, Infix, AssignmentBinary))

#undef FloatOrInt
#undef IntOnly
#undef Prefix
#undef Infix
#undef Suffix
