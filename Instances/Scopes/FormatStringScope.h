#ifndef IS_FORMATSTRINGSCOPE_H
#define IS_FORMATSTRINGSCOPE_H

#include "DecoderBase/Scope.h"

using namespace decoder;

namespace instances {
    namespace scopes {
        class FormatStringScope : public Scope {
        private:
            bool Wide;

            FormatStringScope() {}

            static FormatStringScope *get(bool Wideness, bool Shortness) {
                static bool Uninited = true;
                static FormatStringScope Scopes[2][2];
                if(Uninited) {
                    Uninited = false;
                    for(int W = 0; W < 2; W++)
                        for(int S = 0; S < 2; S++) {
                            Scopes[W][S].Wide = W;
                            Scopes[W][S].setShortness(S);
                        }
                }
                return &Scopes[Wideness][Shortness];
            }

        public:
            static FormatStringScope *get(bool Wideness) {
                return get(Wideness, true);
            }

            bool isWide() const {return Wide;}

            Scope *changeShortness(bool value) override {
                return get(Wide, value);
            }
        };
    }
}

#endif
