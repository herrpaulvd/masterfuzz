#ifndef IS_SINGLESTRINGSCOPE_H
#define IS_SINGLESTRINGSCOPE_H

#include "DecoderBase/Scope.h"
#include "Instances/NodeKinds/SingleStringNK.h"
#include <string>
#include <vector>

using namespace decoder;
using namespace instances::nodekinds;

namespace instances {
    namespace scopes {
        // Pseudo-scope caching pseudo node kind.
        // Saves generated through input data.
        class SingleStringScope : public Scope {
        private:
            static std::vector<SingleStringScope *> &getStash() {
                static std::vector<SingleStringScope *> Stash;
                return Stash;
            }
        public:
            SingleStringScope(const std::string &S, bool Managed) {
                ScopeCache Cache;
                Cache.Single = new SingleStringNK(S);
                Cache.SelectedKinds.push_back(0);
                Cache.InfoFields.emplace_back();
                Cache.Single->getInfoFields(this, Cache.InfoFields[0]);
                setCache(Cache);
                if(Managed)
                    getStash().push_back(this);
            }

            ~SingleStringScope() {delete (SingleStringNK *)getCache().Single;}

            // Shortness does not affect the scope.
            Scope *changeShortness(bool value) override {return this;}

            const std::string &getString() {
                return ((SingleStringNK *)getCache().Single)->getString();
            }

            static void clearManaged() {
                auto &Stash = getStash();
                for(auto Ptr : Stash) delete Ptr;
                Stash.clear();
            }
        };
    }
}

#endif
