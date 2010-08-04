%{
	#include <iostream>
	#include <stdio.h>
	#include <string.h>
	#include "ast/nodes.h"

	extern int yylex();
	void yyerror(const char *s) { printf("Syntax error : %s\n", s); }
%}

%union {
	ExprAST *expr;
	FunctionAST *func;
	std::string *string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER
%token <token> TDEF TIF TELSE TEQ
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TPLUS TMINUS TMULT TDIV
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <func> func_decl
%type <expr> expr block
%type <token> comparison bin_operator

%left TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%left TPLUS TMINUS
%left TMULT TDIV

%start program

%%

program : func_stmts;

func_stmts : func_decl { $1->Codegen()->dump(); };
		   | func_stmts func_decl { $2->Codegen()->dump(); };

block : TLBRACE TRBRACE { $$ = new UnimplementedAST(); }
	  | TLBRACE skip_space expr skip_space TRBRACE { $$ = $3; };

expr : TINTEGER { $$ = new IntegerExprAST(atoi($1->c_str())); delete $1; }
	 | func_call { $$ = new UnimplementedAST(); }
	 | expr skip_space comparison skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | expr skip_space bin_operator skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | TLPAREN expr TRPAREN { $$ = $2 };
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

func_call : TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { std::cout << "Call to " << (*$1) << std::endl; };

func_call_args : /*empty*/ {}
			   | expr {}
			   | expr skip_space TCOMMA skip_space func_call_args {};

func_decl : TDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ $$ = new FunctionAST((*$3), std::vector<std::string>(), $8); };

func_decl_args : /* empty */ {}
			| TIDENTIFIER {}
			| func_decl_args skip_space TCOMMA skip_space TIDENTIFIER {};

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV;

%%
