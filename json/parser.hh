#pragma once
#include <memory>

#include "lex.hh"
#include "ast.hh"

namespace json
{

/*using Array = std::vector<TagVal>;*/
/*using Object = std::vector<KeyVal>;*/

struct Parser
{
    const std::string m_sName = {};
    std::unique_ptr<KeyVal> m_upHead = {};

    Parser(std::string_view path);

    void parse();
    void print();
    void printNode(KeyVal* pNode, std::string_view svEnd);

private:
    Lexer m_l;
    Token m_tCurr;
    Token m_tNext;

    void expect(enum Token::TYPE t);
    void next();
    void parseNode(KeyVal* pNode);
    void parseIdent(TagVal* pTV);
    void parseNumber(TagVal* pTV);
    void parseObject(KeyVal* pNode);
    void parseArray(KeyVal* pNode);
    void parseNull(TagVal* pTV);
    void parseBool(TagVal* pTV);
    KeyVal* searchObject(std::vector<KeyVal>& aObj, std::string_view svKey);
};

static inline std::vector<KeyVal>&
getObject(decltype(TagVal::val)& val)
{
    return std::get<std::vector<KeyVal>>(val);
}

static inline std::vector<TagVal>&
getArray(decltype(TagVal::val)& val)
{
    return std::get<std::vector<TagVal>>(val);
}

static inline long
getInteger(decltype(TagVal::val)& val)
{
    return std::get<long>(val);
}

static inline double
getReal(decltype(TagVal::val)& val)
{
    return std::get<double>(val);
}

static inline std::string_view
getStringView(decltype(TagVal::val)& val)
{
    return std::get<std::string_view>(val);
}

} /* namespace json */
