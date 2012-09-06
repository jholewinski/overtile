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
}

%token FUNCTION
%token<Ident> IDENT

%%

top_level:
  FUNCTION
  ;

%%

void SSPerror(const char *Str) {
  std::cerr << Str << "\n";
}
