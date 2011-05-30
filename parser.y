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
%token <token> TDOUBLE TINTEGER
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

block : TLBRACE stmts TRBRACE { $$ = $2; }
	  | TLBRACE TRBRACE { $$ = 0; }

stmts : stmt { $$ = new std::vector<ExprAST*>(); $$->push_back($1); }
	  | stmts stmt { $$->push_back($2); };

stmt : expr { $$ = $1; }
	| TIF TSPACE expr TSPACE block TSPACE TELSE TSPACE block
	 	{ $$ = new IfExprAST($3, $5, $9); }
	| TFOR TSPACE TIDENTIFIER TSPACE TEQ TSPACE expr TCOMMA TSPACE expr TCOMMA TSPACE expr TSPACE block
		{ $$ = new ForExprAST($3, $7, $10, $13, $15); };

expr : TLPAREN expr TRPAREN { $$ = $2; } /* Parenthesis */
	 | expr bin_operator expr { $$ = new BinaryExprAST($2, $1, $3); }
	 | TNUMBER { $$ = new NumberExprAST(atof($1->c_str())); delete $1; } /* Number */
	 | TIDENTIFIER TLPAREN func_call_args TRPAREN { $$ = new CallExprAST((*$1), $3); delete $1; } /* Function call */
	 | TIDENTIFIER { $$ = new VariableExprAST((*$1)); delete $1; }
	 | expr comparison expr { $$ = new BinaryExprAST($2, $1, $3); }
	 | var_type TSPACE TIDENTIFIER TVARDEF expr { $$ = new VariableDefAST(new VariableExprAST((*$3)), $5, $1); }
	 | TIDENTIFIER TEQ expr { $$ = new VariableAssignAST(new VariableExprAST((*$1)), $3); }

func_call_args : /*empty*/ { $$ = new std::vector<ExprAST*>(); }
			   | expr { $$ = new std::vector<ExprAST*>(); $$->push_back($1); }
			   | func_call_args TCOMMA expr { $$->push_back($3); };

func_decl : TDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN TSPACE block
		  		{ $$ = new FunctionAST((*$3), $5, $8); }
			| TJDEF TSPACE TIDENTIFIER TLPAREN func_decl_args TRPAREN TSPACE block
				{ $$ = new FunctionAST((*$3), $5, $8); $$->setNative(true); }

func_decl_args : /* empty */ { $$ = new std::vector<FunctionTypeAST*>(); }
			| var_type TSPACE TIDENTIFIER { $$ = new std::vector<FunctionTypeAST*>(); $$->push_back(new FunctionTypeAST(*$3, VariableDefAST::getTypeFromAST($1))); delete $3; }
			| func_decl_args TCOMMA var_type TSPACE TIDENTIFIER { $$->push_back(new FunctionTypeAST(*$5, VariableDefAST::getTypeFromAST($3))); delete $5; };

comparison : TCEQ | TCGEQ | TCGT | TCLEQ | TCLT | TCNEQ;

bin_operator : TPLUS | TMINUS | TMULT | TDIV | TMOD;

var_type : TDOUBLE | TINTEGER;

%%
