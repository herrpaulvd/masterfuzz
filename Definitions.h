#include <vector>
std::vector<FunctionBuilder> NoFunctions = {};

std::vector<FunctionBuilder> Functions2only = {
    {"printf", {ParamType::Format, ParamType::Varargs}, "qprintf"}, 
    {"fprintf", {ParamType::File, ParamType::Format, ParamType::Varargs}, "qfprintf"}, 
    {"wprintf", {ParamType::WFormat, ParamType::Varargs}, "qwprintf"},
    {"fwprintf", {ParamType::File, ParamType::WFormat, ParamType::Varargs}, "qfwprintf"},
};

std::vector<FunctionBuilder> MemoryFunctions = {
    {"memcpy", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}, "qmemcpy"},
    {"memmove", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}, "qmemmove"},
    {"memset", {ParamType::PVoid, ParamType::Int, ParamType::SizeT}, "qmemset"},

    {"wmemcpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwmemcpy"},
    {"wmemmove", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwmemmove"},
    {"wmemset", {ParamType::PWChar, ParamType::Int, ParamType::SizeT}, "qwmemset"},

    {"strcat", {ParamType::PChar, ParamType::PChar}, "qstrcat"},
    {"strncat", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}, "qstrncat"},
    {"strcpy", {ParamType::PChar, ParamType::PChar}, "qstrcpy"},
    {"strncpy", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}, "qstrncpy"},

    {"wcscat", {ParamType::PWChar, ParamType::PWChar}, "qwcscat"},
    {"wcsncat", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwcsncat"},
    {"wcscpy", {ParamType::PWChar, ParamType::PWChar}, "qwcscpy"},
    {"wcsncpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwcsncpy"},
};

std::vector<FunctionBuilder> Functions = {
    {"printf", {ParamType::Format, ParamType::Varargs}, "qprintf"}, 
    {"fprintf", {ParamType::File, ParamType::Format, ParamType::Varargs}, "qfprintf"}, 
    {"sprintf", {ParamType::PChar, ParamType::Format, ParamType::Varargs}, "qsprintf"}, 
    {"snprintf", {ParamType::PChar, ParamType::SizeT, ParamType::Format, ParamType::Varargs}, "qsnprintf"},

    {"wprintf", {ParamType::WFormat, ParamType::Varargs}, "qwprintf"},
    {"fwprintf", {ParamType::File, ParamType::WFormat, ParamType::Varargs}, "qfwprintf"},
    {"swprintf", {ParamType::PWChar, ParamType::SizeT, ParamType::WFormat, ParamType::Varargs}, "qswprintf"},

    {"memcpy", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}, "qmemcpy"},
    {"memmove", {ParamType::PVoid, ParamType::PVoid, ParamType::SizeT}, "qmemmove"},
    {"memset", {ParamType::PVoid, ParamType::Int, ParamType::SizeT}, "qmemset"},

    {"wmemcpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwmemcpy"},
    {"wmemmove", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwmemmove"},
    {"wmemset", {ParamType::PWChar, ParamType::Int, ParamType::SizeT}, "qwmemset"},

    {"strcat", {ParamType::PChar, ParamType::PChar}, "qstrcat"},
    {"strncat", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}, "qstrncat"},
    {"strcpy", {ParamType::PChar, ParamType::PChar}, "qstrcpy"},
    {"strncpy", {ParamType::PChar, ParamType::PChar, ParamType::SizeT}, "qstrncpy"},

    {"wcscat", {ParamType::PWChar, ParamType::PWChar}, "qwcscat"},
    {"wcsncat", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwcsncat"},
    {"wcscpy", {ParamType::PWChar, ParamType::PWChar}, "qwcscpy"},
    {"wcsncpy", {ParamType::PWChar, ParamType::PWChar, ParamType::SizeT}, "qwcsncpy"},
};
