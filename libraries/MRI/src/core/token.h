/* Copyright 2012 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* 'Class' used to parse and tokenize a string based on provided list of separators. */
#ifndef TOKEN_H_
#define TOKEN_H_

#include <stddef.h>
#include <core/try_catch.h>

/* Maximum number of tokens that a string can be separated into. */
#define TOKEN_MAX_TOKENS 10

/* Maximum size of string that can be split into tokens. */
#define TOKEN_MAX_STRING 64

struct Token
{
    const char* tokenPointers[TOKEN_MAX_TOKENS];
    const char* pTokenSeparators;
    size_t      tokenCount;
    char        copyOfString[TOKEN_MAX_STRING + 1];
};
typedef struct Token Token;


/* Real name of functions are in mri namespace. */
         void        mriToken_Init(Token* pToken);
         void        mriToken_InitWith(Token* pToken, const char* pTheseTokenSeparators);
__throws void        mriToken_SplitString(Token* pToken, const char* pStringToSplit);
         size_t      mriToken_GetTokenCount(Token* pToken);
__throws const char* mriToken_GetToken(Token* pToken, size_t tokenIndex);
         const char* mriToken_MatchingString(Token* pToken, const char* pTokenToSearchFor);
         const char* mriToken_MatchingStringPrefix(Token* pToken, const char* pTokenPrefixToSearchFor);
         void        mriToken_Copy(Token* pTokenCopy, Token* pTokenOriginal);

/* Macroes which allow code to drop the mri namespace prefix. */
#define Token_Init                  mriToken_Init
#define Token_InitWith              mriToken_InitWith
#define Token_SplitString           mriToken_SplitString
#define Token_GetTokenCount         mriToken_GetTokenCount
#define Token_GetToken              mriToken_GetToken
#define Token_MatchingString        mriToken_MatchingString
#define Token_MatchingStringPrefix  mriToken_MatchingStringPrefix
#define Token_Copy                  mriToken_Copy

#endif /* TOKEN_H_ */
