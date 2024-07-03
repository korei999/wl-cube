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
    const std::string m_sName {};
    std::unique_ptr<Object> m_upHead {};

    Parser(std::string_view path);

    void parse();
    void print();
    void printNode(Object* pNode, std::string_view svEnd);

private:
    Lexer m_l;
    Token m_tCurr;
    Token m_tNext;

    void expect(enum Token::TYPE t);
    void next();
    void parseNode(Object* pNode);
    void parseIdent(TagVal* pTV);
    void parseNumber(TagVal* pTV);
    void parseObject(Object* pNode);
    void parseArray(Object* pNode); /* arrays are same as objects */
    void parseNull(TagVal* pTV);
    void parseBool(TagVal* pTV);
};

/* Interface for getting values from pObject->tagVal.val */
/* maybe extracting values from pObject directly is sufficient idk... */

static inline std::vector<Object>&
getObject(Object& obj)
{
    return std::get<std::vector<Object>>(obj.tagVal.val);
}

/* arrays are axactly like objects but keys are always empty */
static inline std::vector<Object>&
getArray(Object& obj)
{
    return getObject(obj);
}

static inline long
getInteger(Object& obj)
{
    return std::get<long>(obj.tagVal.val);
}

static inline double
getReal(Object& obj)
{
    return std::get<double>(obj.tagVal.val);
}

static inline std::string_view
getStringView(Object& obj)
{
    return std::get<std::string_view>(obj.tagVal.val);
}

static inline Object*
searchObject(std::vector<Object>& aObj, std::string_view svKey)
{
    for (auto& node : aObj)
        if (node.svKey == svKey)
            return &node;

    return nullptr;
}

} /* namespace json */
