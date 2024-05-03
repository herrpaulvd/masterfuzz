#ifndef DECODERBASE_SCOPE_H
#define DECODERBASE_SCOPE_H

#include <iostream>
#include <utility>
#include <vector>

namespace decoder {
    class ASTNodeKind;

    struct ScopeCache {
        std::vector<int> SelectedKinds;
        std::vector<std::vector<int>> InfoFields;
        ASTNodeKind *Single;

        ScopeCache() : SelectedKinds(), InfoFields(), Single() {}
    };

    class Scope {
    private:
        bool Shortness = false;
        ScopeCache Cache;
        bool HasCache = false;
    protected:
        void setShortness(bool value) {
            if(Shortness == value) return;
            Shortness = value;
            HasCache = false;
        }
    public:
        bool isShort() const {return Shortness;}

        virtual Scope *changeShortness(bool value) = 0;

        bool hasCache() const {return HasCache;}

        const ScopeCache &getCache() const {return Cache;}

        void setCache(const ScopeCache &Cache) {
            this->Cache = Cache;
            HasCache = true;
        }

        void clearCache() {HasCache = false;}

        virtual void printDebugInfo() const {
            std::cout << "No Debug Info" << std::endl;
        }
    };
}

#endif
