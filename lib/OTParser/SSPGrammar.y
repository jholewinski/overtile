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

%token FUNCTION
%token<Ident> IDENT
%token<IntConst> INTCONST
%token<DoubleConst> DOUBLECONST
%token EQUALS
%token OPENPARENS CLOSEPARENS
%token OPENBRACE CLOSEBRACE
%token COMMA
%token PLUS MINUS ASTERISK SLASH


%%

top_level
: decl_list
;

decl_list
: /* empty */
| decl decl_list
;

decl
: function

function
: FUNCTION IDENT OPENPARENS param_list CLOSEPARENS EQUALS expression

param_list
: /* empty */
| param other_param_list
;

other_param_list
: /* empty */
| COMMA param other_param_list
;

param
: IDENT
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

%%

void SSPerror(const char *Str) {
  std::cerr << Str << "\n";
}
