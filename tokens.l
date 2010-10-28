%{
#include <iostream>
#include <string>
#include "ast/nodes.h"
#include "parser.hpp"

#define SAVE_STRING (yylval.string = new std::string(yytext,yyleng))
#define SAVE_TOKEN(t) (yylval.token = t)
extern "C" int yywrap() {}
extern int yylineno;
%}

%%

" "			return SAVE_TOKEN(TSPACE);
\n			yylineno++;
\t			;
","			return SAVE_TOKEN(TCOMMA);

"def"		return SAVE_TOKEN(TDEF);

"if"		return SAVE_TOKEN(TIF);
"for"		return SAVE_TOKEN(TFOR);
"else"		return SAVE_TOKEN(TELSE);

[a-zA-Z_][a-zA-Z0-9_.]*	SAVE_STRING; return TIDENTIFIER;
[0-9]*		SAVE_STRING; return TINTEGER;

"("			return SAVE_TOKEN(TLPAREN);
")"			return SAVE_TOKEN(TRPAREN);
"{"			return SAVE_TOKEN(TLBRACE);
"}"			return SAVE_TOKEN(TRBRACE);

"="			return SAVE_TOKEN(TEQ);
":="		return SAVE_TOKEN(TVARDEF);

"!="		return SAVE_TOKEN(TCNEQ);
"=="		return SAVE_TOKEN(TCEQ);
">="		return SAVE_TOKEN(TCGEQ);
">"			return SAVE_TOKEN(TCGT);
"<="		return SAVE_TOKEN(TCLEQ);
"<"			return SAVE_TOKEN(TCLT);

"+"			return SAVE_TOKEN(TPLUS);
"-"			return SAVE_TOKEN(TMINUS);
"*"			return SAVE_TOKEN(TMULT);
"/"			return SAVE_TOKEN(TDIV);
"%"			return SAVE_TOKEN(TMOD);

.			printf("Lexical error : unknown token '%s'.\n", yytext); yyterminate();

%%
