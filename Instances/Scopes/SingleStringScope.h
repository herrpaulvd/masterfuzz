#ifndef IS_SINGLESTRINGSCOPE_H
#define IS_SINGLESTRINGSCOPE_H

#include "DecoderBase/Scope.h"
#include "Instances/NodeKinds/SingleStringNK.h"

using namespace decoder;
using namespace instances::nodekinds;

namespace instances {
    namespace scopes {
        // Pseudo-scope caching pseudo node kind.
        // Saves generated through input data.
        class SingleStringScope : public Scope {
        public:
            SingleStringScope(const std::string &S) {
                ScopeCache Cache;
                Cache.Single = new SingleStringNK(S);
                Cache.SelectedKinds.push_back(0);
                Cache.InfoFields.emplace_back();
                Cache.Single->getInfoFields(this, Cache.InfoFields[0]);
                setCache(Cache);
            }

            // Shortness does not affect the scope.
            Scope *changeShortness(bool value) override {return this;}
        };
    }
}

#endif
