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
            int AllowSigned : 1;
            int AllowUnsigned : 1;
            int AllowRvalue : 1;
            int AllowInt : 1;
            int AllowFloat : 1;

            ExpressionScope() {}
            
            static ExpressionScope *get(int PtrDepthMin, int PtrDepthMax,
                int BaseSizeExpMin, int BaseSizeExpMax,
                int AllowSigned, int AllowUnsigned, int AllowRvalue,
                int AllowInt, int AllowFloat,
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
                int AllowSigned, int AllowUnsigned, int AllowRvalue,
                int AllowInt, int AllowFloat) {
                return get(PtrDepthMin, PtrDepthMax,
                    BaseSizeExpMin, BaseSizeExpMax,
                    AllowSigned, AllowUnsigned, AllowRvalue,
                    AllowInt, AllowFloat, true);
            }

            int getPtrDepthMin() const {return PtrDepthMin;}
            int getPtrDepthMax() const {return PtrDepthMax;}
            int getBaseSizeExpMin() const {return BaseSizeExpMin;}
            int getBaseSizeExpMax() const {return BaseSizeExpMax;}
            int getAllowSigned() const {return AllowSigned;}
            int getAllowUnsigned() const {return AllowUnsigned;}
            int getAllowRvalue() const {return AllowRvalue;}
            int getAllowInt() const {return AllowInt;}
            int getAllowFloat() const {return AllowFloat;}

            Scope *changeShortness(bool value) override {
                return get(PtrDepthMin, PtrDepthMax,
                    BaseSizeExpMin, BaseSizeExpMax,
                    AllowSigned, AllowUnsigned, AllowRvalue,
                    AllowInt, AllowFloat, value);
            }
        };
    }
}

#endif
