#ifndef IP_FLEXIBLEPRINTER_H
#define IP_FLEXIBLEPRINTER_H

#include "DecoderBase/Utils.h"
#include "Instances/Printers/SimplePrinter.h"
#include <cstring>
#include <stack>
#include <string>
#include <vector>

using namespace decoder;

namespace instances {
    namespace printers {
        struct CallInfo {
            std::string Name;
            std::vector<std::string> Args;

            CallInfo(const std::string &Name)
                : Name(Name), Args() {}
        };

        class FlexiblePrinter : public SimplePrinter {
        private:
            std::stack<int> TempNames;
            int NextTempName = 0;
            std::string Prefix;

            std::string makeTempName(int Suffix) {
                std::string Result = Prefix;
                Result.append(std::to_string(Suffix));
                return Result;
            }

            std::stack<std::string> Casts;
            std::stack<CallInfo *> Calls;

        protected:
            void pushNextTempName() {TempNames.push(NextTempName++);}
            
            std::string getCurrentTempName() {
                if(TempNames.empty())
                    return makeTempName(NextTempName++);
                return makeTempName(TempNames.top());
            }

            std::string extractTempName() {
                int Suffix = TempNames.top();
                TempNames.pop();
                return makeTempName(Suffix);
            }

            void openStatement(const char *Opening) {
                printLine(Opening);
                pusht();
            }

            void emitOperationInit(bool Lvalue) {
                startLine("auto ");
                if(Lvalue) print('&');
                print(getCurrentTempName());
                print(" = ");
            }

            void emitOperationEnd() {
                print(';');
            }

        public:
            FlexiblePrinter(const std::string &Filename, const std::string &Prefix)
                : SimplePrinter(Filename), Prefix(Prefix) {}

            ~FlexiblePrinter() {
                close();
            }

            //*** General ***//
            // ParentInfo:: Expression and Statement aren't used.
            //***//

            //*** Cast ***//

            void startCastTypePart() override final {
                SimplePrinter::startCastTypePart();
            }

            void endCastTypePart() override final {
                SimplePrinter::endCastTypePart();
            }

            void startCastArg() override final {
                pushNextTempName();
            }

            virtual void emitCast(const std::string &Cast, const std::string &Var) {
                SimplePrinter::printCastTypeName(Cast);
                print(Var);
            }

            void endCastArg() override final {
                std::string Arg = extractTempName();
                emitOperationInit(false);
                emitCast(Casts.top(), Arg);
                Casts.pop();
                emitOperationEnd();
            }

            //***//

            //*** Call ***//
            
            void startCallNamePart() override final {
                SimplePrinter::startCallNamePart();
            }
            
            void endCallNamePart() override final {
                clearParentInfo();
                // Do not print anything.
            }

            void startFunctionArg() override final {
                pushNextTempName();
                Calls.top()->Args.push_back(getCurrentTempName());
                setParentInfo(ParentInfo::Expression); // Need for 'stdout'.
            }

            void endFunctionArg() override final {
                clearParentInfo();
            }

            virtual void emitCall(const CallInfo &Info) {
                startLine(Info.Name);
                print('(');
                int Last = Info.Args.size() - 1;
                for(int I = 0; I <= Last; I++) {
                    print(Info.Args[I]);
                    if(I < Last) print(", ");
                    else endLine(");");
                }
            }

            void endCall() override final {
                clearParentInfo();
                CallInfo *Info = Calls.top();
                Calls.pop();
                emitCall(*Info);
                int N = Info->Args.size();
                while(N--) extractTempName();
                delete Info;
            }

            //***//

            //*** Const ***//
            // The job will be done via printConst.

            void startConst() override final {
                SimplePrinter::startConst();
            }

            void endConst() override final {
                SimplePrinter::endConst();
            }

            //***//

            //*** Delete ***//

            void startDelete() override final {
                pushNextTempName();
            }

            virtual void emitDelete(const std::string &Var) {
                startLine("delete [] ");
                print(Var);
                endLine(';');
            }

            void endDelete() override final {
                emitDelete(extractTempName());
            }

            //***//

            //*** Format string ***//
            // The job will be done via printConst.

            void startFormatString() override final {
                SimplePrinter::startFormatString();
            }

            void endFormatString() override final {
                SimplePrinter::endFormatString();
            }

            //***//

            //*** For ***//

            void startFor() override final {
                pushNextTempName(); // T1
            }

            void betweenForInitAndCondition() override final {
                extractTempName(); // T1, unused.
                openStatement("while(true) {");
                pushNextTempName(); // T2
                // keep ParentInfo.
            }

            void betweenForConditionAndStep() override final {
                startLine("if(!");
                print(extractTempName()); // T2
                endLine(") break;");
                // Vars are extracted in the order: T3, LS, Le3
                // So push them vice versa
                pushNextTempName(); // Le3
                std::string Le3 = getCurrentTempName();
                pushNextTempName(); // LS
                startLine("goto ");
                print(getCurrentTempName());
                endLine(';');
                startLine(Le3);
                endLine(':');
                printLine('{'); // emit Block to prevent init crossing CE.
                pusht();
                pushNextTempName(); // T3
                // keep ParentInfo.
            }

            void betweenForStepAndBody() override final {
                extractTempName(); // T3, unused.
                printLine("continue;");
                popt();
                printLine('}'); // end the Block.
                startLine(extractTempName()); // LS
                endLine(':');
                printLine('{'); // start the 2nd.
                pusht();
                emitUniquePrint();
            }

            void endFor() override final {
                startLine("goto ");
                print(extractTempName()); // Le3
                endLine(';');
                popt();
                printLine('}'); // end the 2nd Block.
                popt();
                printLine('}');
            }

            //***//

            //*** If ***//

            void startIf() override final {
                pushNextTempName(); // Condition.
            }

            void betweenIfConditionAndBody() override final {
                startLine("if(");
                print(extractTempName()); // Condition
                endLine(") {");
                pusht();
                emitUniquePrint();
            }

            void endIf() override final {
                popt();
                printLine('}');
            }

            void startElse() override final {
                printLine("else {");
                pusht();
                emitUniquePrint();
            }

            void endElse() override final {
                popt();
                printLine('}');
            }

            //***//

            //*** Index ***//

            void startArray() override final {
                pushNextTempName(); // Array.
            }

            void endArrayStartIndex() override final {
                pushNextTempName(); // Index.
            }

            virtual void emitIndex(const std::string &Array, const std::string &Index) {
                print(Array);
                print('[');
                print(Index);
                print(']');
            }

            void endIndex() override final {
                std::string Index = extractTempName();
                std::string Array = extractTempName();
                emitOperationInit(true);
                emitIndex(Array, Index);
                emitOperationEnd();
            }

            //***//

            //*** New ***//

            void startNew() override final {
                setParentInfo(ParentInfo::StringNew);
            }

            void betweenNewTypePartAndSize() override final {
                clearParentInfo();
                pushNextTempName(); // Size.
            }

            virtual void emitNew(const std::string &Type, const std::string &Size) {
                print("new ");
                print(Type);
                print(" [");
                print(Size);
                print("]()");
            }

            void endNew() override final {
                std::string Arg = extractTempName();
                emitOperationInit(false);
                // Let new type args share casts stack.
                emitNew(Casts.top(), Arg);
                Casts.pop();
                emitOperationEnd();
            }

            //***//

            //*** Operation ***//
            
            void startUnary(const char *Sign, bool Suffix) override final {
                pushNextTempName(); // Arg.
            }

            virtual void emitUnary(const char *Sign, bool Suffix, const std::string &Arg) {
                if(!Suffix) print(Sign);
                print(Arg);
                if(Suffix) print(Sign);
            }

            void endUnary(const char *Sign, bool Suffix) override final {
                std::string Arg = extractTempName();

                static std::string Deref = "*";

                emitOperationInit(Sign == Deref);
                emitUnary(Sign, Suffix, Arg);
                emitOperationEnd();
            }

            void startBinary(const char *Sign) override final {
                pushNextTempName(); // Left.
            }

            void middleBinary(const char *Sign) override final {
                bool OrElse = strcmp(Sign, "||") == 0;
                bool AndAlso = strcmp(Sign, "&&") == 0;
                if(OrElse || AndAlso) {
                    std::string Left = extractTempName();
                    std::string Result = getCurrentTempName();
                    startLine("bool ");
                    print(Result);
                    print(" = (bool)");
                    print(Left);
                    endLine(';');
                    startLine("if(");
                    if(OrElse) print('!');
                    print(Result);
                    endLine(") {");
                    pusht();
                }

                pushNextTempName(); // Right.
            }

            virtual void emitBinary(const char *Sign, const std::string &Left, const std::string &Right) {
                print(Left);
                print(' ');
                print(Sign);
                print(' ');
                print(Right);
            }

            void endBinary(const char *Sign) override final {
                bool OrElse = strcmp(Sign, "||") == 0;
                bool AndAlso = strcmp(Sign, "&&") == 0;
                std::string Right = extractTempName();
                if(OrElse || AndAlso) {
                    std::string Result = getCurrentTempName();
                    startLine(Result);
                    print(" = (bool)");
                    print(Right);
                    endLine(';');
                    popt();
                    printLine('}');
                } else {
                    std::string Left = extractTempName();
                    emitOperationInit(false);
                    emitBinary(Sign, Left, Right);
                    emitOperationEnd();
                }
            }

            //***//

            //*** Variable ***//
            // The job will be done via printVariable.

            void startVariable() override final {
                SimplePrinter::startVariable();
            }

            void endVariable() override final {
                SimplePrinter::endVariable();
            }

            //***//

            //*** While ***//

            void startWhile() override final {
                openStatement("while (true) {");
                pushNextTempName(); // Condition.
                emitUniquePrint();
            }

            void betweenWhileConditionAndBody() override final {
                startLine("if(!");
                print(extractTempName()); // Condition.
                endLine(") break;");
            }

            void endWhile() override final {
                popt();
                printLine('}');
            }

            void startDoWhile() override final {
                // Do-while emition is simpler through while.
                openStatement("while (true) {");
                emitUniquePrint();
            }

            void betweenDoWhileBodyAndCondition() override final {
                pushNextTempName(); // Condition.
            }

            void endDoWhile() override final {
                startLine("if(!");
                print(extractTempName()); // Condition.
                endLine(") break;");
                popt();
                printLine('}');
            }

            //***//

            //*** Stub ***//

            void startStub() override final {
                SimplePrinter::startStub();
            }

            void endStub() override final {
                SimplePrinter::endStub();
            }

            //***//

            //*** Single String emitions ***//

            void printCastTypeName(const std::string &Cast) override final {
                Casts.push(Cast);
            }

            virtual void emitConst(const std::string &Const) {
                print(Const);
            }

            void printConst(const std::string &Const) override final {
                emitOperationInit(endsWith(Const, "[0]"));
                emitConst(Const);
                emitOperationEnd();
            }

            virtual void emitVariable(const std::string &Variable) {
                print(Variable);
            }

            void printVariable(const std::string &Variable) override final {
                emitOperationInit(true);
                emitVariable(Variable);
                emitOperationEnd();
            }

            void printNewArg(const std::string &Type) override final {
                Casts.push(Type);
            }

            void printFunctionName(const std::string &Name) override final {
                Calls.push(new CallInfo(Name));
            }

            virtual void emitStub(const std::string &Stub, bool Lvalue) {
                print(Stub);
            }

            void printStub(const std::string &Stub) override final {
                bool Lvalue = Stub[0] == 'L';
                emitOperationInit(Lvalue);
                emitStub(Stub, Lvalue);
                emitOperationEnd();
            }

            void emitSingleString(const std::string &S) override final {
                SimplePrinter::emitSingleString(S);
            }

            bool checkState() override {
                return PRNTRCHECKSTATE(SimplePrinter::checkState()) 
                    && PRNTRCHECKSTATE(TempNames.empty()) 
                    && PRNTRCHECKSTATE(Casts.empty())
                    && PRNTRCHECKSTATE(Calls.empty());
            }

            //***//

        };
    }
}

#endif
