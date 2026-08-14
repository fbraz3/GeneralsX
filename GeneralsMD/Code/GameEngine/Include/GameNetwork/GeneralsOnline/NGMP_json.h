#ifndef NGMP_JSON_H
#define NGMP_JSON_H

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#elif __has_include("GameNetwork/GeneralsOnline/json.hpp")
#include "GameNetwork/GeneralsOnline/json.hpp"
#elif __has_include("json.hpp")
#include "json.hpp"
#else
#include "GameNetwork/GeneralsOnline/json.hpp"
#endif

#pragma pop_macro("max")
#pragma pop_macro("min")

#endif // NGMP_JSON_H
