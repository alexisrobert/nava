%{
	#include <iostream>
	#include <stdio.h>
	#include <string.h>
	extern int yylex();
	void yyerror(const char *s) { printf("Syntax error : %s\n", s); }
%}

%union {
	std::string *expr;
	std::string *string;
	int token;
}

%token <string> TIDENTIFIER TINTEGER
%token <token> TDEF TJDEF TIF TRETURN TEQ
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <expr> expr
%type <token> comparison

%start program

%%

program : func_stmts;

func_stmts : func_decl
		   | func_stmts func_decl;

stmts : stmt
	  | stmts stmt;

stmt : if_stmt
	 | func_call
	 | var_decl
	 | var_decl skip_space TEQ skip_space expr {} /* Variable definition with content */
	 | return_stmt;

block : TLBRACE TRBRACE {}
	  | TLBRACE stmts TRBRACE {};

expr : TIDENTIFIER { $$ = $1; }
	 | TINTEGER { $$ = $1; }
	 | func_call { $$ = new std::string("func call"); }
	 | TIDENTIFIER TSPACE comparison TSPACE TINTEGER { $$ = new std::string("If "+(*$1)+" [comparison] "+(*$5)); };
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

if_stmt : TIF TSPACE expr skip_space block { std::cout << (*$3) << std::endl; };

return_stmt : TRETURN TSPACE expr { std::cout << "Return " << (*$3) << std::endl; }

func_call : TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { std::cout << "Call to " << (*$1) << std::endl; };

func_call_args : /*empty*/ {}
			   | expr {}
			   | expr skip_space TCOMMA skip_space func_call_args {};

func_decl : TDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ std::cout << "Native function definition : " << (*$5) << std::endl; }
			| TJDEF TSPACE TIDENTIFIER TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
				{ std::cout << "JNI function definition : " << (*$5) << std::endl; }
			;

func_decl_args : /* empty */ {}
			| var_decl {}
			| func_decl_args skip_space TCOMMA skip_space var_decl {};
			
var_decl : TIDENTIFIER TSPACE TIDENTIFIER { std::cout << "Argument " << (*$3) << " of type " << (*$1) << std::endl; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

%%
