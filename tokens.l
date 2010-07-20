%{
#include <iostream>
#include <string>
#include "parser.tab.h"

#define SAVE_STRING (yylval.string = new std::string(yytext,yyleng))
#define SAVE_TOKEN(t) (yylval.token = t)
extern "C" int yywrap() {}
%}

%%

" "			return SAVE_TOKEN(TSPACE);
[\n\t]		;
","			return SAVE_TOKEN(TCOMMA);

"jdef"		return SAVE_TOKEN(TJDEF);
"def"		return SAVE_TOKEN(TDEF);
"if"		return SAVE_TOKEN(TIF);

[a-zA-Z_][a-zA-Z0-9_]*	SAVE_STRING; return TIDENTIFIER;
[0-9]*		SAVE_STRING; return TINTEGER;

"("			return SAVE_TOKEN(TLPAREN);
")"			return SAVE_TOKEN(TRPAREN);
"{"			return SAVE_TOKEN(TLBRACE);
"}"			return SAVE_TOKEN(TRBRACE);

"=="		return SAVE_TOKEN(TCEQ);
.			printf("Lexical error : unknown token '%s'.\n", yytext); yyterminate();

%%
