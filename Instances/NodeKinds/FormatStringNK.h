#ifndef INK_FORMATSTRINGNK_H
#define INK_FORMATSTRINGNK_H

#include "DecoderBase/ASTNodeKind.h"
#include "DecoderBase/Scope.h"
#include "Instances/Printers/SimplePrinter.h"
#include "Instances/Scopes/FormatStringScope.h"
#include "Instances/Scopes/SingleStringScope.h"
#include <cassert>
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::scopes;
using namespace instances::printers;

namespace instances {
    namespace nodekinds {
        enum class FmtModifier {
            SingleChar = 0,
            String = 1,
            Integer = 2,
            Float = 3,
        };

        class FormatStringNK : public ASTNodeKind {
        private:
            const int EntriesCount = 4;
            FormatStringNK() {}
        public:
            static FormatStringNK *get() {
                static FormatStringNK Instance;
                return &Instance;
            }

            bool getInfoFields(const Scope *S, std::vector<int> &Sizes) const
                override {
                const FormatStringScope *FSS = dynamic_cast<const FormatStringScope *>(S);
                if(!FSS) return false;
                
                // Generate entries.
                for(int I = 0; I < EntriesCount; I++) {
                    Sizes.push_back(1); // Random char or an arg
                    Sizes.push_back(2); // Char class
                    Sizes.push_back(1); // longness
                    Sizes.push_back(4); // Rest for a random char
                }
                return true;
            }

            void getOperandsScopes(const Scope *ResultScope,
                const std::vector<int> &Values,
                std::vector<Scope *> &OperandsScopes) const override {
                const FormatStringScope *FSS = dynamic_cast<const FormatStringScope *>(ResultScope);
                
                std::string Result;
                if(FSS->isWide()) Result.push_back('L');
                Result.push_back('"');
                
                for(int I = 0; I < EntriesCount; I++) {
                    int Offset = I * 4;
                    int Random = Values[Offset] & 1;
                    int Class = Values[Offset + 1] & 0b11;
                    int Long = Values[Offset + 2] & 1;
                    int Rest = Values[Offset + 3] & 0b1111;
                    if(Random) {
                        char C = Class | (Long << 2) | (Rest << 3);
                        // Consider it empty slot if C < 32 (space) and is neither CR nor LF.
                        if(C == '\\') {
                            Result.push_back(C);
                            Result.push_back(C);
                        } else if(C == '"') {
                            Result.push_back('\\');
                            Result.push_back(C);
                        } else if(C >= ' ') {
                            Result.push_back(C);
                        } else if(C == '\r') {
                            Result.push_back('\\');
                            Result.push_back('r');
                        } else if(C == '\n') {
                            Result.push_back('\\');
                            Result.push_back('n');
                        }
                    } else {
                        Result.push_back('%');
                        switch ((FmtModifier)Class) {
                        default: throw "Unknown modifier";
                        case FmtModifier::SingleChar:
                            if(Long)
                                Result.push_back('l');
                            Result.push_back('c');
                            break;
                        case FmtModifier::String:
                            if(Long)
                                Result.push_back('l');
                            Result.push_back('s');
                            break;
                        case FmtModifier::Integer:
                            if(Long) {
                                Result.push_back('l');
                                Result.push_back('l');
                            }
                            Result.push_back('d');
                            break;
                        case FmtModifier::Float:
                            if(Long)
                                Result.push_back('L');
                            Result.push_back('f');
                            break;
                        }
                    }
                }

                Result.push_back('"');
                OperandsScopes.push_back(new SingleStringScope(Result, true));
            }

            void print(Printer *P, int Part, bool Last) const override {
                SimplePrinter *SP = dynamic_cast<SimplePrinter *>(P);
                assert(SP);

                switch(Part) {
                default: throw "Invalid children count";
                case 0:
                    SP->startFormatString();
                    break;
                case 1:
                    SP->endFormatString();
                    break;
                }
            }
        };
    }
}

#endif
