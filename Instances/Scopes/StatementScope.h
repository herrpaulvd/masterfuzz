#ifndef IS_STATEMENTSCOPE_H
#define IS_STATEMENTSCOPE_H

#include "DecoderBase/Scope.h"

using namespace decoder;

namespace instances {
    namespace scopes {
        class StatementScope : public Scope {
        private:
            StatementScope(bool Shortness) {
                setShortness(Shortness);
            }
        public:
            static StatementScope *getLarge() {
                static StatementScope Instance(false);
                return &Instance;
            }

            static StatementScope *getShort() {
                static StatementScope Instance(true);
                return &Instance;
            }

            static StatementScope *get(bool Shortness) {
                return Shortness ? getShort() : getLarge();
            }

            Scope *changeShortness(bool value) override {
                return get(value);
            }
        };
    }
}

#endif
