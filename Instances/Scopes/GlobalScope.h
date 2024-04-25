#ifndef IS_GLOBALSCOPE_H
#define IS_GLOBALSCOPE_H

#include "DecoderBase/Scope.h"

using namespace decoder;

namespace instances {
    namespace scopes {
        class GlobalScope : public Scope {
        private:
            GlobalScope(bool Shortness) {
                setShortness(Shortness);
            }
        public:
            static GlobalScope *getLarge() {
                static GlobalScope Instance(false);
                return &Instance;
            }

            static GlobalScope *getShort() {
                static GlobalScope Instance(true);
                return &Instance;
            }

            static GlobalScope *get(bool Shortness) {
                return Shortness ? getShort() : getLarge();
            }

            Scope *changeShortness(bool value) override {
                return get(value);
            }
        };
    }
}

#endif
