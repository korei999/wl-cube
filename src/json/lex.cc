#include <fstream>

#include "lex.hh"
#include "../utils.hh"

namespace json
{

void
Lexer::loadFile(std::string_view path)
{
    std::ifstream file(path.data(), std::ios::in | std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        CERR("failed to open '{}'\n", path);
        exit(1);
    }

    size_t fileSize = (size_t)file.tellg();
    m_sFile = std::string(fileSize + 1, '\0');

    file.seekg(0);
    file.read(m_sFile.data(), fileSize);
    file.close();
}

void
Lexer::skipWhiteSpace()
{
    while (m_pos < m_sFile.size() && (m_sFile[m_pos] == ' ' || m_sFile[m_pos] == '\t' || m_sFile[m_pos] == '\n'))
        m_pos++;
}

Token
Lexer::number()
{
    Token r {};
    size_t start = m_pos;
    size_t i = start;

    while (std::isxdigit(m_sFile[i]) ||
                   m_sFile[i] == '.' ||
                   m_sFile[i] == '-' ||
                   m_sFile[i] == '+')
    {
        i++;
    }

    r.type = Token::NUMBER;
    r.svLiteral = std::string_view(m_sFile).substr(start, i - start);
    
    m_pos = i - 1;
    return r;
}

Token
Lexer::stringNoQuotes()
{
    Token r {};

    size_t start = m_pos;
    size_t i = start;

    while (std::isalpha(m_sFile[i]))
        i++;

    r.svLiteral = std::string_view(m_sFile).substr(start, i - start);

    if ("null" == r.svLiteral)
        r.type = Token::NULL_;
    else if ("false" == r.svLiteral)
        r.type = Token::FALSE;
    else if ("true" == r.svLiteral)
        r.type = Token::TRUE;
    else
        r.type = Token::IDENT;

    m_pos = i - 1;
    return r;
}

Token
Lexer::string()
{
    Token r {};

    size_t start = m_pos;
    size_t i = start + 1;
    bool bEsc = false;

    while (m_sFile[i])
    {
        switch (m_sFile[i])
        {
            default:
                if (bEsc)
                    bEsc = false;
                break;

            case Token::EOF_:
                CERR("unterminated string\n");
                exit(1);

            case '\n':
                CERR("Unexpected newline within string");
                exit(1);

            case '\\':
                bEsc = !bEsc;
                break;

            case '"':
                if (!bEsc)
                    goto done;
                else
                    bEsc = false;
                break;
        }

        i++;
    }

done:

    r.type = Token::IDENT;
    r.svLiteral = std::string_view(m_sFile).substr(start + 1, (i - start) - 1);

    m_pos = i;
    return r;
}

Token
Lexer::character(enum Token::TYPE type)
{
    return {
        .type = type,
        .svLiteral = std::string_view(m_sFile).substr(m_pos, 1)
    };
}

Token
Lexer::next()
{
    Token r {};

    if (m_pos >= m_sFile.size())
            return r;

    skipWhiteSpace();

    switch (m_sFile[m_pos])
    {
        default:
            /* solves bools and nulls */
            r = stringNoQuotes();
            break;

        case '-':
        case '+':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            r = number();
            break;

        case Token::QUOTE:
            r = string();
            break;

        case Token::COMMA:
            r = character(Token::COMMA);
            break;

        case Token::ASSIGN:
            r = character(Token::ASSIGN);
            break;

        case Token::LBRACE:
            r = character(Token::LBRACE);
            break;

        case Token::RBRACE:
            r = character(Token::RBRACE);
            break;

        case Token::RBRACKET:
            r = character(Token::RBRACKET);
            break;

        case Token::LBRACKET:
            r = character(Token::LBRACKET);
            break;

        case Token::EOF_:
            r.type = Token::EOF_;
            break;
    }

    m_pos++;
    return r;
}

} /* namespace json */
