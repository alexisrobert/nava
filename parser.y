%{
	#include <iostream>
	#include <stdio.h>
	extern int yylex();
	void yyerror(const char *s) { printf("Syntax error : %s\n", s); }
%}

%union {
std::string *string;
int token;
}

%token <string> TIDENTIFIER TINTEGER
%token <token> TDEF TJDEF TIF
%token <token> TCEQ
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%start program

%%

program : func_stmts;

func_stmts : func_decl
		   | func_stmts func_decl;

stmts : stmt
	  | stmts stmt;

stmt : if_stmt
	 | func_call
	 | var_decl;

block : TLBRACE TRBRACE {}
	  | TLBRACE stmts TRBRACE {};
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

if_stmt : TIF TSPACE TIDENTIFIER TSPACE TCEQ TSPACE TINTEGER skip_space block
			{ std::cout << "If " << (*$3) << " == " << (*$7) << std::endl; };

func_call : TIDENTIFIER skip_space TLPAREN func_call_args TRPAREN { std::cout << "Call to " << (*$1) << std::endl; };

func_call_args : /*empty*/ {}
			   | func_call_arg {}
			   | func_call_arg skip_space TCOMMA skip_space func_call_args {};

func_call_arg : TIDENTIFIER { std::cout << "With identifier arg " << (*$1) << std::endl; }
			  | TINTEGER { std::cout << "With integer arg " << (*$1) << std::endl; }

func_decl : TDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ std::cout << "Native function definition : " << (*$3) << std::endl; }
			| TJDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
				{ std::cout << "JNI function definition : " << (*$3) << std::endl; }
			;

func_decl_args : /* empty */ {}
			| var_decl {}
			| func_decl_args skip_space TCOMMA skip_space var_decl {};
			
var_decl : TIDENTIFIER TSPACE TIDENTIFIER { std::cout << "Argument " << (*$3) << " of type " << (*$1) << std::endl; };

%%
