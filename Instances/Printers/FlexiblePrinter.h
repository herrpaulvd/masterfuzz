#ifndef IP_FLEXIBLEPRINTER_H
#define IP_FLEXIBLEPRINTER_H

#include "Instances/Printers/SimplePrinter.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using namespace decoder;

namespace instances {
    namespace printers {
        class FlexiblePrinter : public SimplePrinter {
        private:
            std::stack<int> TempNames;
            int NextTempName = 0;
            std::string Prefix;
            std::ostringstream SS;

            std::string makeTempName(int Suffix) {
                SS.clear();
                SS << Prefix << Suffix;
                return SS.str();
            }

        protected:
            void pushNextTempName() {TempNames.push(NextTempName++);}
            
            std::string getCurrentTempName() {
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
        public:
            FlexiblePrinter(const std::string &Filename, const std::string &Prefix)
                : SimplePrinter(Filename), Prefix(Prefix) {}

            ~FlexiblePrinter() {
                close();
            }

            void startFor() override {
                pushNextTempName(); // T1
                setParentInfo(ParentInfo::Expression); // e1
            }

            void betweenForInitAndCondition() override {
                extractTempName(); // T1, unused.
                openStatement("while(true) {");
                pushNextTempName(); // T2
                // keep ParentInfo.
            }

            void betweenForConditionAndStep() override {
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
                pushNextTempName(); // T3
                // keep ParentInfo.
            }

            void betweenForStepAndBody() override {
                printLine("continue;");
                startLine(extractTempName()); // LS
                endLine(':');
                clearParentInfo();
                setParentInfo(ParentInfo::Statement);
            }

            void endFor() override {
                startLine("goto ");
                print(extractTempName()); // Le3
                endLine(';');
                popt();
                printLine('}');
            }
        };
    }
}

#endif
