#include "parser.hh"
#include "../headers/utils.hh"

namespace json
{

Parser::Parser(std::string_view path)
    : m_sName(path), m_l(path)
{
    m_tCurr = m_l.next();
    m_tNext = m_l.next();

    if ((m_tCurr.type != Token::LBRACE) && (m_tCurr.type != Token::LBRACKET))
    {
        CERR("wrong first token\n");
        exit(2);
    }

    m_upHead = std::make_unique<KeyVal>();
}

void
Parser::parse()
{
    parseNode(m_upHead.get());
}

void
Parser::expect(enum Token::TYPE t)
{
    if (m_tCurr.type != t)
    {

        CERR("unexpected token\n");
        exit(2);
    }
}

void
Parser::next()
{
    m_tCurr = m_tNext;
    m_tNext = m_l.next();
}

void
Parser::parseNode(KeyVal* pNode)
{
    switch (m_tCurr.type)
    {
        default:
            next();
            break;

        case Token::IDENT:
            parseIdent(&pNode->tagVal);
            break;

        case Token::NUMBER:
            parseNumber(&pNode->tagVal);
            break;

        case Token::LBRACE:
            next(); /* skip brace */
            parseObject(pNode);
            break;

        case Token::LBRACKET:
            next(); /* skip bracket */
            parseArray(pNode);
            break;

        case Token::NULL_:
            parseNull(&pNode->tagVal);
            break;

        case Token::TRUE:
        case Token::FALSE:
            parseBool(&pNode->tagVal);
            break;
    }
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {TAG::STRING, m_tCurr.svLiteral};
    next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    bool bReal = m_tCurr.svLiteral.find('.') != std::string::npos;

    if (bReal)
        *pTV = {.tag = TAG::DOUBLE, .val = std::atof(m_tCurr.svLiteral.data())};
    else
        *pTV = {.tag = TAG::LONG, .val = std::atoll(m_tCurr.svLiteral.data())};

    next();
}

void
Parser::parseObject(KeyVal* pNode)
{
    pNode->tagVal.tag = TAG::OBJECT;
    pNode->tagVal.val = std::vector<KeyVal>{};
    auto& aObjs = std::get<std::vector<KeyVal>>(pNode->tagVal.val);

    for (; m_tCurr.type != Token::RBRACE; next())
    {
        expect(Token::IDENT);
        aObjs.push_back({.svKey = m_tCurr.svLiteral, .tagVal = {}});

        /* skip identifier and ':' */
        next();
        expect(Token::ASSIGN);
        next();

        parseNode(&aObjs.back());

        if (m_tCurr.type != Token::COMMA)
        {
            next();
            break;
        }
    }

    if (aObjs.empty())
        next();
}

void
Parser::parseArray(KeyVal* pNode)
{
    pNode->tagVal.tag = TAG::ARRAY;
    pNode->tagVal.val = std::vector<TagVal>{};
    auto& aTVs = std::get<std::vector<TagVal>>(pNode->tagVal.val);

    /* collect each key/value pair inside array */
    for (; m_tCurr.type != Token::RBRACKET; next())
    {
        aTVs.push_back({});

        switch (m_tCurr.type)
        {
            default:
            case Token::IDENT:
                parseIdent(&aTVs.back());
                break;

            case Token::NULL_:
                parseNull(&aTVs.back());
                break;

            case Token::TRUE:
            case Token::FALSE:
                parseBool(&aTVs.back());
                break;

            case Token::NUMBER:
                parseNumber(&aTVs.back());
                break;

            case Token::LBRACE:
                next();
                aTVs.back().tag = TAG::OBJECT;
                aTVs.back().val = std::vector<KeyVal>(1);
                auto& obj = std::get<std::vector<KeyVal>>(aTVs.back().val).back();
                parseObject(&obj);
                break;
        }

        if (m_tCurr.type != Token::COMMA)
        {
            next();
            break;
        }
    }

    if (aTVs.empty())
        next();
}

void
Parser::parseNull(TagVal* pTV)
{
    *pTV = {.tag = TAG::NULL_, .val = nullptr};
    next();
}

void
Parser::parseBool(TagVal* pTV)
{
    bool b = m_tCurr.type == Token::TRUE? true : false;
    *pTV = {.tag = TAG::BOOL, .val = b};
    next();
}

void
Parser::print()
{
    printNode(m_upHead.get(), "");
    COUT("\n");
}

void
Parser::printNode(KeyVal* pNode, std::string_view svEnd)
{
    std::string_view key = pNode->svKey;

    switch (pNode->tagVal.tag)
    {
        default:
            break;

        case TAG::OBJECT:
            {
                auto& obj = getObject(pNode->tagVal.val);
                std::string q0, q1, objName0, objName1;

                if (key.size() == 0)
                {
                    q0 = q1 = objName1 = objName0 = "";
                }
                else
                {
                    objName0 = key;
                    objName1 = ": ";
                    q1 = q0 = "\"";
                }

                COUT("{}{}{}{}{{\n", q0, objName0, q1, objName1);
                for (size_t i = 0; i < obj.size(); i++)
                {
                    std::string slE = (i == obj.size() - 1) ? "\n" : ",\n";
                    printNode(&obj[i], slE);
                }
                COUT("}}{}", svEnd);
            }
            break;

        case TAG::ARRAY:
            {
                auto& arr = getArray(pNode->tagVal.val);
                std::string q0, q1, arrName0, arrName1;

                if (key.size() == 0)
                {
                    q0 =  q1 = arrName1 = arrName0 = "";
                }
                else
                {
                    arrName0 = key;
                    arrName1 = ": ";
                    q1 = q0 = "\"";
                }

                COUT("{}{}{}{}[", q0, arrName0, q1, arrName1);
                for (size_t i = 0; i < arr.size(); i++)
                {
                    std::string slE = (i == arr.size() - 1) ? "\n" : ",\n";

                    switch (arr[i].tag)
                    {
                        default:
                        case TAG::STRING:
                            {
                                std::string_view sl = std::get<std::string_view>(arr[i].val);
                                COUT("\"{}\"{}", sl, slE);
                            }
                            break;

                        case TAG::NULL_:
                                COUT("{}{}", "null", slE);
                            break;

                        case TAG::LONG:
                            {
                                long num = std::get<long>(arr[i].val);
                                COUT("{}{}", num, slE);
                            }
                            break;

                        case TAG::DOUBLE:
                            {
                                double dnum = std::get<double>(arr[i].val);
                                COUT("{}{}", dnum, slE);
                            }
                            break;

                        case TAG::BOOL:
                            {
                                bool b = std::get<bool>(arr[i].val);
                                COUT("{}{}", b, slE);
                            }
                            break;

                        case TAG::OBJECT:
                                printNode(&std::get<std::vector<KeyVal>>(arr[i].val).back(), slE);
                            break;
                    }
                }
                COUT("]{}", svEnd);
            }
            break;

        case TAG::DOUBLE:
            {
                /* TODO: add some sort formatting for floats */
                double f = std::get<double>(pNode->tagVal.val);
                COUT("\"{}\": {}{}", key, f, svEnd);
            }
            break;

        case TAG::LONG:
            {
                long i = std::get<long>(pNode->tagVal.val);
                COUT("\"{}\": {}{}", key, i, svEnd);
            }
            break;

        case TAG::NULL_:
                COUT("\"{}\": {}{}", key, "null", svEnd);
            break;

        case TAG::STRING:
            {
                std::string_view sl = std::get<std::string_view>(pNode->tagVal.val);
                COUT("\"{}\": \"{}\"{}", key, sl, svEnd);
            }
            break;

        case TAG::BOOL:
            {
                bool b = std::get<bool>(pNode->tagVal.val);
                COUT("\"{}\": {}{}", key, b, svEnd);
            }
            break;
    }
}

KeyVal*
Parser::searchObject(std::vector<KeyVal>& aObj, std::string_view svKey)
{
    for (auto& node : aObj)
    {
        if (node.svKey == svKey)
            return &node;
    }

    return nullptr;
}

} /* namespace json */
