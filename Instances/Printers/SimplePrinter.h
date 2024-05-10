#ifndef IP_SIMPLEPRINTER_H
#define IP_SIMPLEPRINTER_H

#include "DecoderBase/Printer.h"
#include "DecoderBase/Utils.h"
#include "Instances/Printers/UniquePrintGen.h"
#include <cassert>
#include <fstream>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

using namespace decoder;

namespace instances {
    namespace printers {
        enum class ParentInfo {
            Statement, // Next child is statement.
            Expression, // Next child is expression.
            StringFuncall, // Next child must be prepared as funcall.
            StringCast, // Next child is a cast.
            StringConst, // Next child is a const.
            StringVariable, // Next child is a variable.
            StringNew, // Next child is new operator argument.
        };

        class SimplePrinter : public Printer {
        private:
            std::ofstream out;
            int Tabs = 0;
            bool LineEnd = true;
            std::stack<ParentInfo> ParentStack;
            UniquePrintGen *Gen = 0;
        protected:
            void anchor() override {}

            void endLine() {
                if(!LineEnd) {
                    out << '\n';
                    LineEnd = true;
                }
            }

            void startLine() {
                endLine();
                int CurrSpaces = Tabs << 1; // Convert tabs to spaces pairs.
                while(CurrSpaces--) out << ' ';
                LineEnd = false;
            }

            void continueLine() {
                if(LineEnd)
                    startLine();
                else
                    out << ' ';
            }

            template<typename T> void print(T value) {
                out << value;
                LineEnd = false;
            }

            template<typename T> void startLine(T value) {
                startLine();
                print(value);
            }

            template<typename T> void printLine(T value) {
                startLine(value);
                endLine();
            }

            template<typename T> void continueLine(T value) {
                continueLine();
                print(value);
            }

            template<typename T> void finishLine(T value) {
                continueLine(value);
                endLine();
            }

            template<typename T> void endLine(T value) {
                print(value);
                endLine();
            }

            int getParentStackSize() {return ParentStack.size();}

            void printStackSize(const char *Action) {
                return;
                print("/* ");
                print(Action);
                print(": ");
                print(getParentStackSize());
                print("*/");
            }

            void emitUniquePrint() {
                if(Gen) printLine(Gen->make());
            }

            // ParentInfo is some info from parents.
            ParentInfo getParentInfo() {printStackSize("get"); return ParentStack.top();}
            void setParentInfo(ParentInfo value) {ParentStack.push(value); printStackSize("set");}
            void clearParentInfo() {ParentStack.pop(); printStackSize("clear");}

            void startBody() {
                endLine(") {");
                pusht();
                clearParentInfo();
                setParentInfo(ParentInfo::Statement);
                emitUniquePrint();
            }

            void endBody() {
                popt();
                clearParentInfo();
                printLine('}');
            }

            void startConditionPart(const char *OpName) {
                startLine(OpName);
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

        public:
            SimplePrinter(const std::string &Filename)
                : out(Filename) {out.tie(0);} // Speed up out.

            void close() {
                out.close();
            }

            ~SimplePrinter() {
                close();
            }

            void setGen(UniquePrintGen *Gen) {this->Gen = Gen;}

            // For nice looking code, need to count tabs.
            void pusht() {Tabs++;}
            void popt() {Tabs--;}

            virtual void startCastTypePart() {
                // Let printer know that the next arg is a cast.
                setParentInfo(ParentInfo::StringCast);
                // Do not print '(' because empty casts are allowed.
            }

            virtual void endCastTypePart() {
                clearParentInfo();
            }

            virtual void startCastArg() {
                print('(');
                // Let the child know it is an expression.
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endCastArg() {
                clearParentInfo();
                print(')');
            }

            virtual void startCallNamePart() {
                // Let printer know that the next child is function name.
                setParentInfo(ParentInfo::StringFuncall);
            }

            virtual void endCallNamePart() {
                clearParentInfo();
                print('(');
            }

            virtual void startFunctionArg() {
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endFunctionArg() {
                clearParentInfo();
                print(", ");
            }

            virtual void endCall() {
                clearParentInfo();
                print(");");
            }

            virtual void startConst() {
                // Let printer know that the next arg is a const.
                setParentInfo(ParentInfo::StringConst);
            }

            virtual void endConst() {
                clearParentInfo();
            }

            virtual void startDelete() {
                startLine("delete [] ");
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endDelete() {
                clearParentInfo();
                endLine(';');
            }

            virtual void startFormatString() {
                // Let printer know that the next arg is a const.
                setParentInfo(ParentInfo::StringConst);
            }

            virtual void endFormatString() {
                clearParentInfo();
            }

            virtual void startFor() {
                startConditionPart("for");
            }

            virtual void betweenForInitAndCondition() {
                print("; ");
            }

            virtual void betweenForConditionAndStep() {
                print("; ");
            }

            virtual void betweenForStepAndBody() {
                startBody();
            }

            virtual void endFor() {
                endBody();
            }

            virtual void startIf() {
                startConditionPart("if");
            }

            virtual void betweenIfConditionAndBody() {
                startBody();
            }

            virtual void endIf() {
                endBody();
            }

            virtual void startElse() {
                printLine("else {");
                setParentInfo(ParentInfo::Statement);
                pusht();
                emitUniquePrint();
            }

            virtual void endElse() {
                endBody();
            }

            virtual void startArray() {
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endArrayStartIndex() {
                print(")[");
                // keep ParentInfo.
            }

            virtual void endIndex() {
                clearParentInfo();
                print(']');
            }

            virtual void startNew() {
                print("new ");
                setParentInfo(ParentInfo::StringNew);
            }

            virtual void betweenNewTypePartAndSize() {
                clearParentInfo();
                // Prevent too big and negative memory allocation.
                print("[(unsigned char)(");
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endNew() {
                clearParentInfo();
                print(")]()");
            }
            
            virtual void startUnary(const char *Sign, bool Suffix) {
                if(getParentInfo() == ParentInfo::Statement)
                    startLine();
                if(!Suffix) print(Sign);
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endUnary(const char *Sign, bool Suffix) {
                clearParentInfo();
                print(')');
                if(Suffix) print(Sign);
                if(getParentInfo() == ParentInfo::Statement)
                    endLine(';');
            }

            // Even if operation sign is sometimes not needed to implement
            // some emition part, let them be a param to let sofisticated
            // printers build alternative emitions.
            virtual void startBinary(const char *Sign) {
                if(getParentInfo() == ParentInfo::Statement)
                    startLine();
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void middleBinary(const char *Sign) {
                print(") ");
                print(Sign);
                print(" (");
                // keep ParentInfo.
            }

            virtual void endBinary(const char *Sign) {
                clearParentInfo();
                print(')');
                if(getParentInfo() == ParentInfo::Statement)
                    endLine(';');
            }

            virtual void startVariable() {
                // Let printer know that the next arg is a const.
                setParentInfo(ParentInfo::StringVariable);
            }

            virtual void endVariable() {
                clearParentInfo();
            }

            virtual void startWhile() {
                startConditionPart("while");
            }

            virtual void betweenWhileConditionAndBody() {
                startBody();
            }

            virtual void endWhile() {
                endBody();
            }

            virtual void startDoWhile() {
                printLine("do {");
                setParentInfo(ParentInfo::Statement);
                pusht();
                emitUniquePrint();
            }

            virtual void betweenDoWhileBodyAndCondition() {
                endBody();
                startConditionPart("while");
            }

            virtual void endDoWhile() {
                clearParentInfo();
                print(");");
            }

            virtual void printCastTypeName(const std::string &Cast) {
                // Empty cast means no cast.
                if(!Cast.empty()) {
                    print('(');
                    print(Cast);
                    print(')'); // Close type part.
                    // To cast anything to anything without CE,
                    // need intermediate cast to ULL. 
                    print("(unsigned long long)");
                }
            }

            virtual void printConst(const std::string &S) {print(S);}
            virtual void printVariable(const std::string &S) {print(S);}
            virtual void printNewArg(const std::string &S) {print(S);}
            virtual void printFunctionName(const std::string &S) {startLine(S);}

            virtual void emitSingleString(const std::string &S) {
                switch(getParentInfo()) {
                default: throw "Invalid string kind";
                case ParentInfo::StringFuncall:
                    printFunctionName(S);
                    break;
                case ParentInfo::StringCast:
                    printCastTypeName(S);
                    break;
                case ParentInfo::StringConst:
                    printConst(S);
                    break;
                case ParentInfo::StringVariable:
                    printVariable(S);
                    break;
                case ParentInfo::StringNew:
                    printNewArg(S);
                    break;
                case ParentInfo::Expression:
                    assert(S == "stdout"); // Single allowed case.
                    printVariable(S);
                    break;
                }
            }

            virtual void printParts(const std::vector<std::string> &PS) {
                for(auto &P : PS)
                    printLine(P);
            }

            virtual void startProgram(const std::vector<std::string> &Header) {
                printParts(Header);
                pusht();
                setParentInfo(ParentInfo::Statement);
            }

            virtual void endProgram(const std::vector<std::string> &Footer) {
                popt();
                clearParentInfo();
                printParts(Footer);
            }

#define PRNTRCHECKSTATE(x) decoder::printAndGet(#x , x)

            virtual bool checkState() {
                return PRNTRCHECKSTATE(Tabs == 0) && PRNTRCHECKSTATE(ParentStack.empty());
            }
        };
    }
}

#endif
