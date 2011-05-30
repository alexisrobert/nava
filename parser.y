%{
	#include <iostream>
	#include <stdio.h>
	#include <string.h>
	#include "ast/nodes.h"

	extern int yylex();
	extern int yyget_lineno();

	extern RootAST *rootnode;

	void yyerror(const char *s) { printf("line %d: %s\n", yyget_lineno(), s); }
%}

%union {
	ExprAST *expr;
	FunctionAST *func;
	std::vector<ExprAST*> *stmts;
	std::vector<FunctionTypeAST*> *funcdeclargs;
	std::vector<ExprAST*> *funcargs;
	std::string *string;
	int token;
}

%token <string> TIDENTIFIER TNUMBER
%token <token> TJDEF TDEF TIF TELSE TEQ TFOR
%token <token> TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%token <token> TPLUS TMINUS TMULT TDIV TMOD
%token <token> TDOUBLE
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TSPACE TCOMMA

%type <func> func_decl
%type <funcdeclargs> func_decl_args
%type <funcargs> func_call_args
%type <expr> expr stmt
%type <stmts> stmts block
%type <token> comparison bin_operator var_type

%left TCEQ TCNEQ TCGEQ TCGT TCLEQ TCLT
%left TPLUS TMINUS
%left TMULT TDIV TMOD
%left TEQ TVARDEF

%start program

%%

program : func_stmts;

func_stmts : func_decl { rootnode->children->push_back($1); };
		   | func_stmts func_decl { rootnode->children->push_back($2); };

block : TLBRACE skip_space stmts skip_space TRBRACE { $$ = $3; }
	  | TLBRACE skip_space TRBRACE { $$ = 0; }

stmts : stmt { $$ = new std::vector<ExprAST*>(); $$->push_back($1); }
	  | stmts stmt { $$->push_back($2); };

stmt : expr { $$ = $1; }
	| TIF TSPACE expr skip_space block skip_space TELSE skip_space block
	 	{ $$ = new IfExprAST($3, $5, $9); }
	| TFOR TSPACE TIDENTIFIER TSPACE TEQ TSPACE expr TCOMMA TSPACE expr TCOMMA TSPACE expr skip_space block
		{ $$ = new ForExprAST($3, $7, $10, $13, $15); };

expr : TNUMBER { $$ = new NumberExprAST(atof($1->c_str())); delete $1; }
	 | TIDENTIFIER TLPAREN func_call_args TRPAREN { $$ = new CallExprAST((*$1), $3); delete $1; }
	 | TIDENTIFIER { $$ = new VariableExprAST((*$1)); delete $1; }
	 | expr skip_space comparison skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | var_type TSPACE TIDENTIFIER skip_space TVARDEF skip_space expr { $$ = new VariableDefAST(new VariableExprAST((*$3)), $7, $1); }
	 | TIDENTIFIER skip_space TEQ skip_space expr { $$ = new VariableAssignAST(new VariableExprAST((*$1)), $5); }
	 | expr skip_space bin_operator skip_space expr { $$ = new BinaryExprAST($3, $1, $5); }
	 | TLPAREN expr TRPAREN { $$ = $2; }
		
skip_space : /*empty*/ {}
		   | skip_space TSPACE {}

func_call_args : /*empty*/ { $$ = new std::vector<ExprAST*>(); }
			   | expr { $$ = new std::vector<ExprAST*>(); $$->push_back($1); }
			   | func_call_args skip_space TCOMMA skip_space expr { $$->push_back($5); };

func_decl : TDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
		  		{ $$ = new FunctionAST((*$3), $5, $8); }
			| TJDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN skip_space block
				{ $$ = new FunctionAST((*$3), $5, $8); $$->setNative(true); }

func_decl_args : /* empty */ { $$ = new std::vector<FunctionTypeAST*>(); }
			| var_type TSPACE TIDENTIFIER { $$ = new std::vector<FunctionTypeAST*>(); $$->push_back(new FunctionTypeAST(*$3, VariableDefAST::getTypeFromAST($1))); delete $3; }
			| func_decl_args skip_space TCOMMA skip_space var_type TSPACE TIDENTIFIER { $$->push_back(new FunctionTypeAST(*$7, VariableDefAST::getTypeFromAST($5))); delete $7; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV | TMOD;

var_type : TDOUBLE;

%%
