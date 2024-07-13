#include "parser.hh"
#include "utils.hh"

namespace json
{

Parser::Parser(std::string_view path)
{
    load(path);
}

void
Parser::load(std::string_view path)
{
    this->sName = path;
    this->lex.loadFile(path);

    this->tCurr = this->lex.next();
    this->tNext = this->lex.next();

    if ((this->tCurr.type != Token::LBRACE) && (this->tCurr.type != Token::LBRACKET))
    {
        CERR("wrong first token\n");
        exit(2);
    }

    this->upHead = std::make_unique<Object>();
}

void
Parser::parse()
{
    this->parseNode(this->upHead.get());
}

void
Parser::expect(enum Token::TYPE t)
{
    if (this->tCurr.type != t)
    {
        CERR("({}): unexpected token\n", this->sName);
        exit(2);
    }
}

void
Parser::next()
{
    this->tCurr = this->tNext;
    this->tNext = this->lex.next();
}

void
Parser::parseNode(Object* pNode)
{
    switch (this->tCurr.type)
    {
        default:
            this->next();
            break;

        case Token::IDENT:
            this->parseIdent(&pNode->tagVal);
            break;

        case Token::NUMBER:
            this->parseNumber(&pNode->tagVal);
            break;

        case Token::LBRACE:
            this->next(); /* skip brace */
            this->parseObject(pNode);
            break;

        case Token::LBRACKET:
            this->next(); /* skip bracket */
            this->parseArray(pNode);
            break;

        case Token::NULL_:
            this->parseNull(&pNode->tagVal);
            break;

        case Token::TRUE:
        case Token::FALSE:
            this->parseBool(&pNode->tagVal);
            break;
    }
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {TAG::STRING, this->tCurr.svLiteral};
    this->next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    bool bReal = this->tCurr.svLiteral.find('.') != std::string::npos;

    if (bReal)
        *pTV = {.tag = TAG::DOUBLE, .val = std::atof(this->tCurr.svLiteral.data())};
    else
        *pTV = TagVal{.tag = TAG::LONG, .val = std::atol(this->tCurr.svLiteral.data())};

    next();
}

void
Parser::parseObject(Object* pNode)
{
    pNode->tagVal.tag = TAG::OBJECT;
    pNode->tagVal.val = std::vector<Object>{};
    auto& aObjs = std::get<std::vector<Object>>(pNode->tagVal.val);

    for (; this->tCurr.type != Token::RBRACE; this->next())
    {
        this->expect(Token::IDENT);
        aObjs.push_back({.svKey = this->tCurr.svLiteral, .tagVal = {}});

        /* skip identifier and ':' */
        this->next();
        this->expect(Token::ASSIGN);
        this->next();

        this->parseNode(&aObjs.back());

        if (this->tCurr.type != Token::COMMA)
        {
            this->next();
            break;
        }
    }

    if (aObjs.empty())
        this->next();
}

void
Parser::parseArray(Object* pNode)
{
    pNode->tagVal.tag = TAG::ARRAY;
    pNode->tagVal.val = std::vector<Object>{};
    auto& aTVs = getArray(pNode);

    /* collect each key/value pair inside array */
    for (; this->tCurr.type != Token::RBRACKET; this->next())
    {
        aTVs.push_back({});

        switch (this->tCurr.type)
        {
            default:
            case Token::IDENT:
                this->parseIdent(&aTVs.back().tagVal);
                break;

            case Token::NULL_:
                this->parseNull(&aTVs.back().tagVal);
                break;

            case Token::TRUE:
            case Token::FALSE:
                this->parseBool(&aTVs.back().tagVal);
                break;

            case Token::NUMBER:
                this->parseNumber(&aTVs.back().tagVal);
                break;

            case Token::LBRACE:
                this->next();
                this->parseObject(&aTVs.back());
                break;
        }

        if (this->tCurr.type != Token::COMMA)
        {
            this->next();
            break;
        }
    }

    if (aTVs.empty())
        this->next();
}

void
Parser::parseNull(TagVal* pTV)
{
    *pTV = {.tag = TAG::NULL_, .val = nullptr};
    this->next();
}

void
Parser::parseBool(TagVal* pTV)
{
    bool b = this->tCurr.type == Token::TRUE? true : false;
    *pTV = {.tag = TAG::BOOL, .val = b};
    this->next();
}

void
Parser::print()
{
    this->printNode(upHead.get(), "");
    COUT("\n");
}

void
Parser::printNode(Object* pNode, std::string_view svEnd)
{
    std::string_view key = pNode->svKey;

    switch (pNode->tagVal.tag)
    {
        default:
            break;

        case TAG::OBJECT:
            {
                auto& obj = getObject(pNode);
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
                    this->printNode(&obj[i], slE);
                }
                COUT("}}{}", svEnd);
            }
            break;

        case TAG::ARRAY:
            {
                auto& arr = getArray(pNode);
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

                    switch (arr[i].tagVal.tag)
                    {
                        default:
                        case TAG::STRING:
                            {
                                std::string_view sl = std::get<std::string_view>(arr[i].tagVal.val);
                                COUT("\"{}\"{}", sl, slE);
                            }
                            break;

                        case TAG::NULL_:
                                COUT("{}{}", "null", slE);
                            break;

                        case TAG::LONG:
                            {
                                long num = std::get<long>(arr[i].tagVal.val);
                                COUT("{}{}", num, slE);
                            }
                            break;

                        case TAG::DOUBLE:
                            {
                                double dnum = std::get<double>(arr[i].tagVal.val);
                                COUT("{}{}", dnum, slE);
                            }
                            break;

                        case TAG::BOOL:
                            {
                                bool b = std::get<bool>(arr[i].tagVal.val);
                                COUT("{}{}", b, slE);
                            }
                            break;

                        case TAG::OBJECT:
                                this->printNode(&arr[i], slE);
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

} /* namespace json */
