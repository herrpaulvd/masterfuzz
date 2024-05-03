#ifndef IS_EXPRESSIONSCOPE_H
#define IS_EXPRESSIONSCOPE_H

#include "DecoderBase/Scope.h"
#include <map>
#include <tuple>

using namespace decoder;

namespace instances {
    namespace scopes {
        using ES_tuple = std::tuple<int, int, int, int, int, int, int, int, int, int>;

        class ExpressionScope : public Scope {
        private:
            int PtrDepthMin, PtrDepthMax;
            // Exp means log_2(BaseSize).
            // E.g. BaseSize = 1 BYTE => BaseSizeExp = 0;
            // 2 => 1, 4 => 2, 8 => 3.
            int BaseSizeExpMin, BaseSizeExpMax;
            bool AllowSigned : 1;
            bool AllowUnsigned : 1;
            bool AllowRvalue : 1;
            bool AllowInt : 1;
            bool AllowFloat : 1;

            ExpressionScope() {}
            
            static ExpressionScope *get(int PtrDepthMin, int PtrDepthMax,
                int BaseSizeExpMin, int BaseSizeExpMax,
                bool AllowSigned, bool AllowUnsigned, bool AllowRvalue,
                bool AllowInt, bool AllowFloat,
                bool Shortness) {
                static std::map<ES_tuple, ExpressionScope *> Cache;

                ES_tuple Params(PtrDepthMin, PtrDepthMax,
                    BaseSizeExpMin, BaseSizeExpMax,
                    AllowSigned, AllowUnsigned, AllowRvalue,
                    AllowInt, AllowFloat, Shortness);
                ExpressionScope *&Result = Cache[Params];
                if(!Result) {
                    Result = new ExpressionScope();
                    Result->PtrDepthMin = PtrDepthMin;
                    Result->PtrDepthMax = PtrDepthMax;
                    Result->BaseSizeExpMin = BaseSizeExpMin;
                    Result->BaseSizeExpMax = BaseSizeExpMax;
                    Result->AllowSigned = AllowSigned;
                    Result->AllowUnsigned = AllowUnsigned;
                    Result->AllowRvalue = AllowRvalue;
                    Result->AllowInt = AllowInt;
                    Result->AllowFloat = AllowFloat;
                    Result->setShortness(Shortness);
                }
                return Result;
            }
        public:
            static ExpressionScope *get(int PtrDepthMin, int PtrDepthMax,
                int BaseSizeExpMin, int BaseSizeExpMax,
                bool AllowSigned, bool AllowUnsigned, bool AllowRvalue,
                bool AllowInt, bool AllowFloat) {
                return get(PtrDepthMin, PtrDepthMax,
                    BaseSizeExpMin, BaseSizeExpMax,
                    AllowSigned, AllowUnsigned, AllowRvalue,
                    AllowInt, AllowFloat, false);
            }

            int getPtrDepthMin() const {return PtrDepthMin;}
            int getPtrDepthMax() const {return PtrDepthMax;}
            int getBaseSizeExpMin() const {return BaseSizeExpMin;}
            int getBaseSizeExpMax() const {return BaseSizeExpMax;}
            bool getAllowSigned() const {return AllowSigned;}
            bool getAllowUnsigned() const {return AllowUnsigned;}
            bool getAllowRvalue() const {return AllowRvalue;}
            bool getAllowInt() const {return AllowInt;}
            bool getAllowFloat() const {return AllowFloat;}

            Scope *changeShortness(bool value) override {
                return get(PtrDepthMin, PtrDepthMax,
                    BaseSizeExpMin, BaseSizeExpMax,
                    AllowSigned, AllowUnsigned, AllowRvalue,
                    AllowInt, AllowFloat, value);
            }

            void printDebugInfo() const override {
                std::cout << "ES " << getPtrDepthMin() << ' ' << getPtrDepthMax() << std::endl;
                std::cout << getBaseSizeExpMin() << ' ' << getBaseSizeExpMax() << std::endl;
                std::cout << getAllowSigned() << ' ' << getAllowUnsigned() << std::endl;
                std::cout << getAllowRvalue() << std::endl;
                std::cout << getAllowInt() << ' ' << getAllowFloat() << std::endl;
            }
        };
    }
}

#endif
