#ifndef IP_SIMPLEPRINTER_H
#define IP_SIMPLEPRINTER_H

#include "DecoderBase/Printer.h"
#include <fstream>
#include <stack>
#include <string>

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
        };

        class SimplePrinter : Printer {
        private:
            std::ofstream out;
            int Tabs = 0;
            bool LineEnd = true;
            std::stack<ParentInfo> ParentStack;
        protected:
            void anchor() override {}

            void startLine() {
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

            void endLine() {
                if(!LineEnd) {
                    out << '\n';
                    LineEnd = true;
                }
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

            virtual void startBody() {
                print(')');
                pusht();
                clearParentInfo();
                setParentInfo(ParentInfo::Statement);
            }

            virtual void endBody() {
                popt();
                clearParentInfo();
            }

            virtual void startConditionPart(const char *OpName) {
                startLine(OpName);
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

        public:
            SimplePrinter(const std::string &Filename)
                : out(Filename) {out.tie(0);} // Speed up out.

            // For nice looking code, need to count tabs.
            void pusht() {Tabs++;}
            void popt() {Tabs--;}

            // ParentInfo is some info from parents.
            ParentInfo getParentInfo() {return ParentStack.top();}
            void setParentInfo(ParentInfo value) {ParentStack.push(value);}
            void clearParentInfo() {ParentStack.pop();}

            // Use virtual methods to let more sofisticated printers override them.
            virtual void startBlock() {
                // Block string usually starts with the same tabs count as
                // the parent, so pop it before starting the block.
                popt();
                finishLine('{');
                // Then push for its children.
                pusht();
                // Let children know they are Statements.
                setParentInfo(ParentInfo::Statement);
            }

            virtual void endBlock() {
                popt();
                printLine('}');
                // Push to let parents set tabs correctly.
                pusht();
                // Clear unused parent info.
                clearParentInfo();
            }

            virtual void startCast(const std::string &Cast) {
                // Empty cast means no cast.
                if(!Cast.empty()) {
                    print('(');
                    print(Cast);
                    print(')'); // Close type part.
                }
                print(')'); // Start arg part.
            }

            virtual void endCast() {
                print(')'); // Close arg part.
            }

            virtual void endArg() {
                print(", ");
            }

            virtual void endStatement() {
                print(';');
            }

            virtual void startCall(const std::string &S) {
                startLine(S);
                print('(');
            }

            virtual void endCall() {
                print(')');
                // If it's a statement call, end statement
                if(getParentInfo() == ParentInfo::Statement)
                    endStatement();
            }

            virtual void startForLoop() {
                startConditionPart("for");
            }

            virtual void endForExpressionPart() {
                print(';');
            }

            virtual void startForBody() {
                startBody();
            }

            virtual void endForBody() {
                endBody();
            }

            virtual void startIf() {
                startConditionPart("if");
            }

            virtual void startIfBody() {
                startBody();
            }

            virtual void endIfBody() {
                endBody();
            }

            virtual void startElse() {
                startLine("else");
                setParentInfo(ParentInfo::Statement);
            }

            virtual void endElse() {
                endBody();
            }

            virtual void startWhile() {
                startConditionPart("while");
            }

            virtual void startWhileBody() {
                startBody();
            }

            virtual void endWhileBody() {
                endBody();
            }

            virtual void startDoWhile() {
                startLine("do");
                setParentInfo(ParentInfo::Statement);
            }

            virtual void startDoWhileCondition() {
                endBody();
                startConditionPart("while");
            }

            virtual void endDoWhileCondition() {
                print(");");
            }

            virtual void startArray() {
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void startIndex() {
                print(")[");
            }

            virtual void endIndex() {
                print(']');
                clearParentInfo();
            }
            
            virtual void startUnary(const char *Sign, bool Suffix) {
                if(!Suffix) print(Sign);
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void endUnary(const char *Sign, bool Suffix) {
                print(')');
                if(Suffix) print(Sign);
                clearParentInfo();
            }

            // Even if operation sign is sometimes not needed to implement
            // some emition part, let them be a param to let sofisticated
            // printers build alternative emitions.
            virtual void startBinary(const char *Sign) {
                print('(');
                setParentInfo(ParentInfo::Expression);
            }

            virtual void middleBinary(const char *Sign) {
                print(") ");
                print(Sign);
                print(" (");
            }

            virtual void endBinary(const char *Sign) {
                print(')');
                clearParentInfo();
            }

            virtual void emitSingleString(const std::string &S) {
                switch(getParentInfo()) {
                default: throw "Invalid string kind";
                case ParentInfo::StringFuncall:
                    startCall(S);
                    break;
                case ParentInfo::StringCast:
                    startCast(S);
                    break;
                case ParentInfo::StringConst:
                case ParentInfo::StringVariable:
                    print(S);
                    break;
                }
            }
        };
    }
}

#endif