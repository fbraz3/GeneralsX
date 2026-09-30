#ifndef NGMP_JSON_H
#define NGMP_JSON_H

#pragma push_macro("min")
#pragma push_macro("max")
#pragma push_macro("snprintf")
#pragma push_macro("vsnprintf")
#pragma push_macro("_snprintf")
#pragma push_macro("_vsnprintf")
#undef min
#undef max
#undef snprintf
#undef vsnprintf
#undef _snprintf
#undef _vsnprintf

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#elif __has_include("GameNetwork/GeneralsOnline/json.hpp")
#include "GameNetwork/GeneralsOnline/json.hpp"
#elif __has_include("json.hpp")
#include "json.hpp"
#else
#include "GameNetwork/GeneralsOnline/json.hpp"
#endif

#pragma pop_macro("_vsnprintf")
#pragma pop_macro("_snprintf")
#pragma pop_macro("vsnprintf")
#pragma pop_macro("snprintf")
#pragma pop_macro("max")
#pragma pop_macro("min")

#endif // NGMP_JSON_H
