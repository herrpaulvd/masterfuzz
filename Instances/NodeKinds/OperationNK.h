#ifndef INK_OPERATIONNK_H
#define INK_OPERATIONNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/ExpressionScope.h"
#include "Instances/Scopes/StatementScope.h"
#include <algorithm>
#include <cassert>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        enum class OpKind {
            ArithmeticUnary = 0, // + - ~
            LogicalUnary = ArithmeticUnary + 1, // !
            IncDecUnary = LogicalUnary + 1, // ++ --
            ReferenceUnary = IncDecUnary + 1, // &
            DereferenceUnary = ReferenceUnary + 1, // *
            //---
            LastUnary = DereferenceUnary,
            //---
            ArithmeticBinary = LastUnary + 1, // + - * / % & | ^ << >>
            PtrBinary = ArithmeticBinary + 1, // + -
            ComparisonBinary = PtrBinary + 1, // == != < > <= >=
            LogicalBinary = ComparisonBinary + 1, // && ||
            AssignmentBinary = LogicalBinary + 1, // =
        };

        class OperationNK : public ASTNodeKind {
        private:
            const char *Op;
            int AllowFloat : 1;
            int Suffix : 1; // Does matter only when printing.
            OpKind Kind;
        public: 
            OperationNK(const char *Op, int AllowFloat, int Suffix, OpKind Kind)
                : Op(Op), AllowFloat(AllowFloat), Suffix(Suffix), Kind(Kind) {}

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                // Shortness is not allowed for complex nodes.
                if(S->isShort()) return false;
                if(const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(S)) {
                    // Which kinds cannot return ptr.
                    if(ES->getPtrDepthMin() > 0)
                        switch (Kind) {
                        default: break;
                        case OpKind::ArithmeticUnary:
                        case OpKind::LogicalUnary:
                        case OpKind::ArithmeticBinary:
                        case OpKind::ComparisonBinary:
                        case OpKind::LogicalBinary:
                            return false;
                        }
                    // Which kinds cannot return max high-depth ptr.
                    if(ES->getPtrDepthMin() == MaxPtrDepth && Kind == OpKind::DereferenceUnary)
                        return false;
                    // Which kinds cannot return non-ptr.
                    if(ES->getPtrDepthMax() == 0)
                        switch (Kind) {
                        default: break;
                        case OpKind::ReferenceUnary:
                        case OpKind::PtrBinary:
                            return false;
                        }
                    // Any base size exp is allowed in any cases.
                    // Any signess too.
                    // Which kinds cannot return lvalue.
                    if(!ES->getAllowRvalue() && Kind != OpKind::DereferenceUnary)
                        return false;
                    // Can the op return float?
                    if(!ES->getAllowInt() && !AllowFloat) return false;
                } else if(const StatementScope * SS = dynamic_cast<const StatementScope *>(S)) {
                    // StatementScope functions as max-wide ExpressionScope.
                    // But only for few ops.
                    switch(Kind) {
                    default: return false;
                    case OpKind::IncDecUnary:
                    case OpKind::AssignmentBinary:
                        break;
                    }
                } else return false;

                // Get all fields from the input.
                Sizes.push_back(getSizeForRange(0, MaxPtrDepth));
                Sizes.push_back(getSizeForRange(0, MaxBaseSizeExp));
                Sizes.push_back(1); // Float/Int
                Sizes.push_back(1); // Signed/Unsigned
                Sizes.push_back(1); // L/R value
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                // If null, it shall be StatementScope.
                const ExpressionScope *ES = dynamic_cast<const ExpressionScope *>(ResultScope);

                // Get result properties. For SS, they are extremal.
                int PtrDepthMin = ES ? ES->getPtrDepthMin() : 0;
                int PtrDepthMax = ES ? ES->getPtrDepthMax() : MaxPtrDepth;
                int BaseSizeExpMin = ES ? ES->getPtrDepthMin() : 0;
                int BaseSizeExpMax = ES ? ES->getPtrDepthMax() : MaxBaseSizeExp;
                int AllowInt = ES ? ES->getAllowInt() : 1;
                int AllowFloat = ES ? ES->getAllowFloat() : 1;
                int AllowSigned = ES ? ES->getAllowSigned() : 1;
                int AllowUnsigned = ES ? ES->getAllowUnsigned() : 1;
                int AllowRvalue = ES ? ES->getAllowRvalue() : 1;

                // Correct them according to the kind and other options.
                // Disallow float if the op disallows it itself.
                if(!this->AllowFloat) AllowFloat = 0;
                switch (Kind) {
                default: break;
                // Which kinds return only ptrs.
                case OpKind::ReferenceUnary:
                case OpKind::PtrBinary:
                    PtrDepthMin = std::max(PtrDepthMin, 1);
                    break;
                // Which kinds cannot return ptr.
                case OpKind::ArithmeticUnary:
                case OpKind::LogicalUnary:
                case OpKind::ArithmeticBinary:
                case OpKind::ComparisonBinary:
                case OpKind::LogicalBinary:
                    PtrDepthMax = 0;
                    break;
                // Which kinds cannot return too high ptr depth value.
                case OpKind::DereferenceUnary:
                    PtrDepthMax = std::min(PtrDepthMax, MaxPtrDepth - 1);
                    break;
                }
                // Which kinds can return lvalue.
                int AllowLvalue = Kind == OpKind::DereferenceUnary ? 1 : 0;

                // Get the actual result properties.
                int ResPtrDepth = selectInRange(PtrDepthMin, PtrDepthMax, MaxPtrDepth, Values[0]);
                bool ResFloat = selectBool(AllowFloat, AllowInt, Values[2]);
                if(ResFloat) BaseSizeExpMin = std::max(BaseSizeExpMin, 2);
                int ResBaseSizeExp = selectInRange(BaseSizeExpMin, BaseSizeExpMax, MaxBaseSizeExp, Values[1]);
                bool ResSigned = selectBool(AllowSigned, AllowUnsigned, Values[3]);
                bool ResLvalue = selectBool(AllowLvalue, AllowRvalue, Values[4]);

                // For non-ptr values, we want to provide a large scope,
                // ResBaseSizeExp will be actually a max one
                // so we need to define the min one.
                BaseSizeExpMin = ResFloat ? 2 : 0;
                // To pass AllowX, redetermine them.
                AllowSigned = ResSigned ? 1 : 0;
                AllowUnsigned = 1 - AllowSigned;
                AllowFloat = ResFloat ? 1 : 0;
                AllowInt = 1 - AllowFloat;
                // AllowRvalue depends on the ops.

                // Determine left/right scopes:
                Scope *Left = nullptr;
                Scope *Right = nullptr;
                switch (Kind) {
                default: throw "Unknown op kind";
                case OpKind::ArithmeticUnary:
                    // Always non-ptr. Maybe rvalue. Same signess. Same type.
                    Left = ExpressionScope::get(
                        0, 0, 
                        BaseSizeExpMin, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    break;
                case OpKind::LogicalUnary:
                    // Maybe ptr. Maybe rvalue. Any signess. Any type/size.
                    Left = ExpressionScope::get(
                        0, MaxPtrDepth, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 1);
                    break;
                case OpKind::IncDecUnary:
                    // Always lvalue. Same all.
                    Left = ExpressionScope::get(
                        ResPtrDepth, ResPtrDepth, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 0, 
                        AllowInt, AllowFloat);
                    break;
                case OpKind::ReferenceUnary:
                    // Always lvalue. Same all except PtrDepth--.
                    Left = ExpressionScope::get(
                        ResPtrDepth - 1, ResPtrDepth - 1, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 0, 
                        AllowInt, AllowFloat);
                    break;
                case OpKind::DereferenceUnary:
                    // Maybe rvalue. Same all except PtrDepth++.
                    Left = ExpressionScope::get(
                        ResPtrDepth + 1, ResPtrDepth + 1, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    break;
                case OpKind::ArithmeticBinary:
                    // Always non-ptr. Maybe rvalue. Same signess. Same type.
                    Right = Left = ExpressionScope::get(
                        0, 0, 
                        BaseSizeExpMin, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    break;
                case OpKind::PtrBinary:
                    // Same ptrdepth. Maybe rvalue. Same signess. Same type/size.
                    Left = ExpressionScope::get(
                        ResPtrDepth, ResPtrDepth, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    // Any integer. Maybe rvalue.
                    Right = ExpressionScope::get(
                        0, 0, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 0);
                    break;
                case OpKind::ComparisonBinary: {
                    // Any two but equal to each other.
                    // It's impossible in such realization to control whether
                    // right will have the same type as left when left is
                    // a type range instead of a single type.
                    // So reuse input fields to get a single type.
                    int OprndPtrDepth = selectInRange(0, MaxPtrDepth, MaxPtrDepth, Values[0]);
                    bool OprndFloat = selectBool(1, 1, Values[2]);
                    int OprndBaseSizeExp = selectInRange(OprndFloat ? 2 : 0, MaxBaseSizeExp, MaxBaseSizeExp, Values[1]);
                    bool OprndSigned = selectBool(1, 1, Values[3]);
                    // l/r value doesn't matter.
                    AllowSigned = OprndSigned ? 1 : 0;
                    AllowUnsigned = 1 - AllowSigned;
                    AllowFloat = OprndFloat ? 1 : 0;
                    AllowInt = 1 - AllowFloat;
                    Right = Left = ExpressionScope::get(
                        OprndPtrDepth, OprndPtrDepth, 
                        OprndBaseSizeExp, OprndBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    break;
                }
                case OpKind::LogicalBinary:
                    // Anything.
                    Right = Left = ExpressionScope::get(
                        0, MaxPtrDepth, 
                        0, MaxBaseSizeExp, 
                        1, 1, 1, 
                        1, 1);
                    break;
                case OpKind::AssignmentBinary: {
                    // Same types. l/r depends on operand position.
                    // lvalue left.
                    Left = ExpressionScope::get(
                        ResPtrDepth, ResPtrDepth, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 0, 
                        AllowInt, AllowFloat);
                    // rvalue right.
                    Right = ExpressionScope::get(
                        ResPtrDepth, ResPtrDepth, 
                        ResBaseSizeExp, ResBaseSizeExp, 
                        AllowSigned, AllowUnsigned, 1, 
                        AllowInt, AllowFloat);
                    break;
                }
                }
                // Return calculated scopes.
                if(Left) OperandsScopes.push_back(Left);
                if(Right) OperandsScopes.push_back(Right);
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                if(Kind <= OpKind::LastUnary) {
                    switch(Part) {
                    default: throw "Invalid children count";
                    case 0:
                        SP->startUnary(Op, Suffix);
                        break;
                    case 1:
                        SP->endUnary(Op, Suffix);
                        break;
                    }
                } else {
                    switch(Part) {
                    default: throw "Invalid children count";
                    case 0:
                        SP->startBinary(Op);
                        break;
                    case 1:
                        SP->middleBinary(Op);
                        break;
                    case 2:
                        SP->endBinary(Op);
                        break;
                    }
                }
            }
        };
    }
}

#endif
