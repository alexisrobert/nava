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
%token <token> TDEF TJDEF TIF TELSE TRETURN TEQ
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TPLUS TMINUS TMULT TDIV
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <func> func_decl
%type <expr> expr
%type <token> comparison bin_operator

%left TPLUS TMINUS
%left TMULT TDIV

%start program

%%

program : func_stmts;

func_stmts : func_decl
		   | stmt
		   | func_stmts func_decl;

stmts : stmt
	  | stmts stmt;

stmt : if_stmt
	 | func_call
	 | var_decl
	 | var_decl skip_space TEQ skip_space expr { } /* Variable definition with content */
	 | return_stmt;

block : TLBRACE TRBRACE { }
	  | TLBRACE stmts TRBRACE {};

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

return_stmt : TRETURN TSPACE expr { }

func_call : TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { std::cout << "Call to " << (*$1) << std::endl; };

func_call_args : /*empty*/ {}
			   | expr {}
			   | expr skip_space TCOMMA skip_space func_call_args {};

func_decl : TDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ $$ = new FunctionAST((*$5), std::vector<std::string>(), NULL); }
			| TJDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
				{ $$ = new FunctionAST((*$5), std::vector<std::string>(), NULL); }
			;

func_decl_args : /* empty */ {}
			| var_decl {}
			| func_decl_args skip_space TCOMMA skip_space var_decl {};
			
var_decl : TIDENTIFIER TSPACE TIDENTIFIER { std::cout << "Argument " << (*$3) << " of type " << (*$1) << std::endl; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV;

%%
