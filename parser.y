%{
	#include <iostream>
	#include <stdio.h>
	#include <string.h>
	#include "ast/nodes.h"

	extern int yylex();
	extern int yyget_lineno();
	void yyerror(const char *s) { printf("line %d: %s\n", yyget_lineno(), s); }
%}

%union {
	ExprAST *expr;
	FunctionAST *func;
	std::vector<std::string> *funcdeclargs;
	std::vector<ExprAST*> *funcargs;
	std::string *string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER
%token <token> TDEF TIF TELSE TEQ
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TPLUS TMINUS TMULT TDIV
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <func> func_decl
%type <funcdeclargs> func_decl_args
%type <funcargs> func_call_args
%type <expr> expr block
%type <token> comparison bin_operator

%left TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%left TPLUS TMINUS
%left TMULT TDIV

%start program

%%

program : func_stmts;

func_stmts : func_decl { $1->execute(); };
		   | func_stmts func_decl { $2->execute(); };

block : TLBRACE skip_space expr skip_space TRBRACE { $$ = $3; }
	  | TLBRACE skip_space TRBRACE { $$ = 0; }

expr : TINTEGER { $$ = new IntegerExprAST(atoi($1->c_str())); delete $1; }
	 | TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { $$ = new CallExprAST((*$1), $4); delete $1; }
	 | TIDENTIFIER { $$ = new VariableExprAST((*$1)); delete $1; }
	 | expr skip_space comparison skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | expr skip_space bin_operator skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | TLPAREN expr TRPAREN { $$ = $2 };
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

func_call_args : /*empty*/ { $$ = new std::vector<ExprAST*>(); }
			   | expr { $$ = new std::vector<ExprAST*>(); $$->push_back($1); }
			   | func_call_args skip_space TCOMMA skip_space expr { $$->push_back($5); };

func_decl : TDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ $$ = new FunctionAST((*$3), $5, $8); };

func_decl_args : /* empty */ { $$ = new std::vector<std::string>(); }
			| TIDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(*$1); delete $1; }
			| func_decl_args skip_space TCOMMA skip_space TIDENTIFIER { $$->push_back(*$5); delete $5; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV;

%%
