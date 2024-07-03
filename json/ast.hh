#pragma once

#include <string_view>
#include <variant>
#include <vector>

namespace json
{

enum class TAG
{
    NULL_,
    STRING,
    LONG,
    DOUBLE,
    ARRAY,
    OBJECT,
    BOOL
};

constexpr std::string_view TAGStrings[] {
    "NULL_", "STRING", "LONG", "DOUBLE", "ARRAY", "OBJECT", "BOOL"
};

constexpr std::string_view
getTAGString(enum TAG t)
{
    return TAGStrings[(int)t];
}

struct Object;

struct TagVal
{
    enum TAG tag;
    std::variant<std::string_view,
                 long,
                 double,
                 std::vector<Object>, /* aka JSON object */
                 bool,
                 std::nullptr_t> val;
};

struct Object
{
    std::string_view svKey;
    TagVal tagVal;
};

} /* namespace json */
