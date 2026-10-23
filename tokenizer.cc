#include "tokenizer.h"
#include <sstream>
#include <iostream>

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word)
    {
        Token token;

        if (word == "|")
            token.type = TOKEN_PIPE;
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else
            {
                if (!tokens.empty())
                {
                    Token &prev = tokens.back();
                    if (prev.type == TOKEN_REDIRECT ||
                        prev.type == TOKEN_APPEND ||
                        prev.type == TOKEN_INPUT ||
                        prev.type == TOKEN_ERROR || 
                        prev.type == TOKEN_REDIRECT_AND_ERROR ||
                        prev.type == TOKEN_COMMAND || 
                        prev.type == TOKEN_ARGUMENT || 
                        word[0] == '-' ||
                        word.find('/') != std::string::npos ||
                        word.find('.') != std::string::npos

                    )
                    {
                        token.type = TOKEN_ARGUMENT;
                    }
                    else
                    {
                        token.type = TOKEN_COMMAND;
                    }
                }
                else
                {
                    token.type = TOKEN_COMMAND;
                }
            }

        token.value = word;
        tokens.push_back(token);
    }

    tokens.push_back({TOKEN_EOF, ""});
    return tokens;
}


/*std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;

    std::string word;
    bool insideQuotes = false;
    bool wordStarted = false;

    for (char c : input)
    {
        // Handle double quotes
        if (c == '"')
        {
            insideQuotes = !insideQuotes;
            wordStarted = true;
        }
        // Split at whitespace only when outside quotes
        else if (std::isspace(static_cast<unsigned char>(c)) && !insideQuotes)
        {
            if (wordStarted)
            {
                Token token;

                if (word == "|")
                    token.type = TOKEN_PIPE;
                else if (word == ">")
                    token.type = TOKEN_REDIRECT;
                else if (word == ">>")
                    token.type = TOKEN_APPEND;
                else if (word == "<")
                    token.type = TOKEN_INPUT;
                else if (word == "&")
                    token.type = TOKEN_BACKGROUND;
                else if (word == "2>")
                    token.type = TOKEN_ERROR;
                else if (word == ">>&")
                    token.type = TOKEN_REDIRECT_AND_ERROR;
                else
                {
                    if (!tokens.empty())
                    {
                        TokenType prev = tokens.back().type;

                        if (prev == TOKEN_REDIRECT ||
                            prev == TOKEN_APPEND ||
                            prev == TOKEN_INPUT ||
                            prev == TOKEN_ERROR ||
                            prev == TOKEN_REDIRECT_AND_ERROR ||
                            prev == TOKEN_COMMAND ||
                            prev == TOKEN_ARGUMENT ||
                            (!word.empty() && word[0] == '-') ||
                            word.find('/') != std::string::npos ||
                            word.find('.') != std::string::npos)
                        {
                            token.type = TOKEN_ARGUMENT;
                        }
                        else
                        {
                            token.type = TOKEN_COMMAND;
                        }
                    }
                    else
                    {
                        token.type = TOKEN_COMMAND;
                    }
                }

                token.value = word;
                tokens.push_back(token);

                word.clear();
                wordStarted = false;
            }
        }
        else
        {
            word += c;
            wordStarted = true;
        }
    }

    // Save the final word
    if (wordStarted)
    {
        Token token;

        if (word == "|")
            token.type = TOKEN_PIPE;
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else
        {
            if (!tokens.empty())
            {
                TokenType prev = tokens.back().type;

                if (prev == TOKEN_REDIRECT ||
                    prev == TOKEN_APPEND ||
                    prev == TOKEN_INPUT ||
                    prev == TOKEN_ERROR ||
                    prev == TOKEN_REDIRECT_AND_ERROR ||
                    prev == TOKEN_COMMAND ||
                    prev == TOKEN_ARGUMENT ||
                    (!word.empty() && word[0] == '-') ||
                    word.find('/') != std::string::npos ||
                    word.find('.') != std::string::npos)
                {
                    token.type = TOKEN_ARGUMENT;
                }
                else
                {
                    token.type = TOKEN_COMMAND;
                }
            }
            else
            {
                token.type = TOKEN_COMMAND;
            }
        }

        token.value = word;
        tokens.push_back(token);
    }

    // Detect unmatched double quotes
    if (insideQuotes)
    {
        std::cerr << "syntax error: unmatched double quote\n";
        tokens.clear();
        tokens.push_back({TOKEN_EOF, ""});
        return tokens;
    }

    tokens.push_back({TOKEN_EOF, ""});

    return tokens;
}