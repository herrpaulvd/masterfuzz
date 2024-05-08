#ifndef IP_FEATUREDPRINTER_H
#define IP_FEATUREDPRINTER_H

#include "Instances/Printers/FlexiblePrinter.h"
#include "Instances/Printers/SimplePrinter.h"
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using namespace decoder;

namespace instances {
    namespace printers {
        struct OperationReplacement {
            const char *Prefix;
            const char *Suffix;
            const char *Infix;

            OperationReplacement() : Prefix(), Suffix(), Infix() {}
        };

        class FeaturedPrinter final : public FlexiblePrinter {
        private:
            std::map<std::string, OperationReplacement> OperationReplacements;
            std::map<std::string, std::string> FunctionReplacements;
            bool SupportSmartPointers = false;

            static std::string ReplacePointerWithSmart(const std::string &Typename) {
                int Count = 0;
                int Last = Typename.size() - 1;
                while(Count <= Last && Typename[Last - Count] == '*') Count++;
                std::string Result;
                int CountPrefix = Count;
                while(CountPrefix--) Result.append("SmartPointer<");
                Result.append(Typename.begin(), Typename.end() - Count);
                while(Count--) Result.push_back('>');
                return Result;
            }
        public:
            FeaturedPrinter(const std::string &Filename, const std::string &Prefix)
                : FlexiblePrinter(Filename, Prefix) {}

            ~FeaturedPrinter() {
                close();
            }

            bool addSmartPointersSupport() {SupportSmartPointers = true;}

            void addOperationReplacement(const std::string &Sign, OperationReplacement Replacement) {
                OperationReplacements[Sign] = Replacement;
            }

            void addFunctionReplacement(const std::string &Original, const std::string &Replacement) {
                FunctionReplacements[Original] = Replacement;
            }

            void emitUnary(const char *Sign, bool Suffix, const std::string &Arg) override final {
                OperationReplacement &Repl = OperationReplacements[Sign];
                const char *ReplacementFunction = Suffix ? Repl.Suffix : Repl.Prefix;
                if(ReplacementFunction) {
                    print(ReplacementFunction);
                    print("<typeof(");
                    print(Arg);
                    print("), typeof(");
                    FlexiblePrinter::emitUnary(Sign, Suffix, Arg);
                    print(")>(");
                    print(Arg);
                    print(')');
                } else FlexiblePrinter::emitUnary(Sign, Suffix, Arg);
            }

            void emitBinary(const char *Sign, const std::string &Left, const std::string &Right) override final {
                OperationReplacement &Repl = OperationReplacements[Sign];
                if(Repl.Infix) {
                    print(Repl.Infix);
                    print("<typeof(");
                    print(Left);
                    print("), typeof(");
                    print(Right);
                    print("), typeof(");
                    FlexiblePrinter::emitBinary(Sign, Left, Right);
                    print(")>(");
                    print(Left);
                    print(", ");
                    print(Right);
                    print(')');
                } else FlexiblePrinter::emitBinary(Sign, Left, Right);
            }

            void emitCall(const CallInfo &Info) override final {
                const CallInfo *ResultCallInfo = &Info;
                std::string &NewName = FunctionReplacements[Info.Name];
                if(!NewName.empty()) {
                    CallInfo NewInfo(NewName);
                    NewInfo.Args = Info.Args;
                    ResultCallInfo = &NewInfo;
                }
                FlexiblePrinter::emitCall(*ResultCallInfo);
            }

            void emitCast(const std::string &Cast, const std::string &Var) override final {
                if(SupportSmartPointers)
                    FlexiblePrinter::emitCast(ReplacePointerWithSmart(Cast), Var);
                else
                    FlexiblePrinter::emitCast(Cast, Var);
            }

            void emitNew(const std::string &Type, const std::string &Size) override final {
                if(SupportSmartPointers) {
                    print("SmartPointer<");
                    print(ReplacePointerWithSmart(Type));
                    print(">::alloc(");
                    print(Size);
                    print(')');
                } else FlexiblePrinter::emitNew(Type, Size);
            }
        };
    }
}

#endif