#pragma once

#include <string>

namespace json
{

struct Token
{
    enum TYPE
    {
        LBRACE = '{',
        RBRACE = '}',
        LBRACKET = '[',
        RBRACKET = ']',
        QUOTE = '"',
        IDENT = 'I',
        NUMBER = 'N',
        TRUE = 'T',
        FALSE = 'F',
        NULL_ = 'n',
        ASSIGN = ':',
        COMMA = ',',
        DOT = '.',
        UNHANDLED = 'X',
        EOF_ = '\0',
    } type;
    std::string_view svLiteral;
};

struct Lexer
{
    std::string m_sFile {};
    size_t m_pos = 0;

    Lexer(std::string_view path) { loadFile(path); }

    void loadFile(std::string_view path);
    void skipWhiteSpace();
    Token number();
    Token stringNoQuotes();
    Token string();
    Token character(enum Token::TYPE type);
    Token next();
};

} /* namespace json */
