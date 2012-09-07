%pure_parser

%{
  
// Stencil Spec (SSP) Grammar

#include "overtile/Parser/SSPParser.h"
#include "llvm/ADT/StringRef.h"
#include <iostream>

using namespace overtile;
using namespace llvm;

#define YYLEX_PARAM State
#define YYPARSE_PARAM State
#define YYERROR_VERBOSE 1

void SSPerror(const char*);

%}

%union {
  llvm::StringRef *Ident;
  long IntConst;
  double DoubleConst;
}

%token DOUBLE
%token FIELD
%token FLOAT
%token GRID
%token IN
%token INOUT
%token IS
%token OUT
%token PROGRAM
%token<Ident> IDENT
%token<IntConst> INTCONST
%token<DoubleConst> DOUBLECONST
%token EQUALS
%token OPENPARENS CLOSEPARENS
%token OPENBRACE CLOSEBRACE
%token COMMA
%token PLUS MINUS ASTERISK SLASH
%token COLON


%%

top_level
: PROGRAM IDENT IS grid_def field_list application_list
;

grid_def
: GRID INTCONST
;

field_list
: /* empty */
| field_def field_list
;

field_def
: FIELD IDENT type copy_semantic
;

application_list
: /* empty */
| application_def application_list
;

application_def
: IDENT application_bounds EQUALS expression
;

application_bounds
: /* empty */
| application_bound application_bounds
;

application_bound
: OPENBRACE INTCONST COLON INTCONST CLOSEBRACE
;

expression
: additive_expr
;

additive_expr
: additive_expr PLUS multiplicative_expr
| additive_expr MINUS multiplicative_expr
| multiplicative_expr
;

multiplicative_expr
: multiplicative_expr ASTERISK unary_expr
| multiplicative_expr SLASH unary_expr
| unary_expr
;

unary_expr
: field_ref
| IDENT
| int_constant
| double_constant
| OPENPARENS expression CLOSEPARENS
;

field_ref
: IDENT offset_list
;

offset_list
: dim_offset offset_list
| dim_offset
;

dim_offset
: OPENBRACE int_constant CLOSEBRACE
;

int_constant
: MINUS INTCONST
| INTCONST
;

double_constant
: DOUBLECONST
;

type
: FLOAT
| DOUBLE
;

copy_semantic
: INOUT
| IN
| OUT
;

%%

void SSPerror(const char *Str) {
  std::cerr << Str << "\n";
}
