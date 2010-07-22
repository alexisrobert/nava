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
	StatementsAST *stmts;
	FunctionAST *func;
	std::string *string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER
%token <token> TDEF TJDEF TIF TELSE TRETURN TEQ
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TPLUS TMINUS TMULT TDIV
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <func> func_decl
%type <expr> expr stmt
%type <stmts> stmts block
%type <token> comparison bin_operator

%left TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%left TPLUS TMINUS
%left TMULT TDIV

%start program

%%

program : func_stmts;

func_stmts : func_decl
		   | stmt
		   | func_stmts func_decl;

stmts : stmt { $$ = new StatementsAST(); $$->statements->push_back($1); }
	  | stmts stmt { $$->statements->push_back($2); };

stmt : if_stmt { $$ = new UnimplementedAST(); }
	 | func_call { $$ = new UnimplementedAST(); }
	 | var_decl { $$ = new UnimplementedAST(); }
	 | var_decl skip_space TEQ skip_space expr { $$ = new UnimplementedAST(); } /* Variable definition with content */
	 | TRETURN TSPACE expr { $$ = new ReturnStmtAST($3); };

block : TLBRACE TRBRACE { $$ = new StatementsAST(); }
	  | TLBRACE stmts TRBRACE { $$ = $2; };

expr : TIDENTIFIER { $$ = new VariableExprAST(*$1); delete $1; }
	 | TINTEGER { $$ = new IntegerExprAST(atoi($1->c_str())); delete $1; }
	 | func_call { $$ = new UnimplementedAST(); }
	 | expr skip_space comparison skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | expr skip_space bin_operator skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | TLPAREN expr TRPAREN { $$ = $2 };
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

if_stmt : TIF TSPACE expr skip_space block { }
		| TIF TSPACE expr skip_space block skip_space TELSE skip_space block { };

func_call : TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { std::cout << "Call to " << (*$1) << std::endl; };

func_call_args : /*empty*/ {}
			   | expr {}
			   | expr skip_space TCOMMA skip_space func_call_args {};

func_decl : TDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ $$ = new FunctionAST((*$5), std::vector<std::string>(), $10); }
			| TJDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
				{ $$ = new FunctionAST((*$5), std::vector<std::string>(), $10); }
			;

func_decl_args : /* empty */ {}
			| var_decl {}
			| func_decl_args skip_space TCOMMA skip_space var_decl {};
			
var_decl : TIDENTIFIER TSPACE TIDENTIFIER { std::cout << "Argument " << (*$3) << " of type " << (*$1) << std::endl; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV;

%%
