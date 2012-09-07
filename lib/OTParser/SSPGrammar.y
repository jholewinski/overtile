%pure_parser
%lex-param {overtile::SSPParser *Parser}
%parse-param {overtile::SSPParser *Parser}

%{
  
// Stencil Spec (SSP) Grammar

#include "overtile/Parser/SSPParser.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Grid.h"
#include "overtile/Core/Types.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

using namespace overtile;
using namespace llvm;

#define YYERROR_VERBOSE 1

void SSPerror(SSPParser*, const char*);

%}

%union {
  llvm::StringRef *Ident;
  long IntConst;
  double DoubleConst;

  overtile::Expression *Expr;
  overtile::ElementType *Type;
  std::vector<overtile::IntConstant*> *IntList;
  std::pair<unsigned, unsigned> *Bound;
  std::vector<std::pair<unsigned, unsigned> > *BoundList;
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


%type<IntConst> int_constant
%type<IntConst> dim_offset
%type<IntList> offset_list
%type<DoubleConst> double_constant
%type<Expr> unary_expr 
%type<Expr> additive_expr 
%type<Expr> multiplicative_expr 
%type<Expr> expression
%type<Expr> field_ref
%type<Type> type
%type<Bound> application_bound
%type<BoundList> application_bounds

%%

top_level
: PROGRAM IDENT IS grid_def field_list application_list
;

grid_def
: GRID INTCONST {
    Grid *G = new Grid($2);
    Parser->setGrid(G);
  }
;

field_list
: /* empty */
| field_def field_list
;

field_def
: FIELD IDENT type copy_semantic {
    Grid *G = Parser->getGrid();
    Field *F = new Field(G, $3, $2->str());
  }
;

application_list
: /* empty */
| application_def application_list
;

application_def
: IDENT application_bounds EQUALS expression {
    Grid *G = Parser->getGrid();
    Field *Out = G->getFieldByName(*$1);
    Expression *E = $4;
    Function *Func = new Function(Out, E);

    std::vector<std::pair<unsigned, unsigned> > *Bounds = $2;
    for (unsigned i = 0, e = Bounds->size(); i < e; ++i) {
      std::pair<unsigned, unsigned> B = (*Bounds)[i];
      Func->setLowerBound(i, B.first);
      Func->setUpperBound(i, B.second);
    }
    delete Bounds;

    G->appendFunction(Func);
  }
;

application_bounds
: application_bound application_bounds {
    $$ = $2;
    $$->push_back(*$1);
    delete $1;
  }
| application_bound {
    $$ = new std::vector<std::pair<unsigned, unsigned> >();
    $$->push_back(*$1);
    delete $1;
  }
;

application_bound
: OPENBRACE INTCONST COLON INTCONST CLOSEBRACE {
    $$ = new std::pair<unsigned, unsigned>($2, $4);
  }
;

expression
: additive_expr {
    $$ = $1;
  }
;

additive_expr
: additive_expr PLUS multiplicative_expr {
    $$ = new BinaryOp(BinaryOp::ADD, $1, $3);
  }
| additive_expr MINUS multiplicative_expr {
    $$ = new BinaryOp(BinaryOp::SUB, $1, $3);
  }
| multiplicative_expr {
    $$ = $1;
  }
;

multiplicative_expr
: multiplicative_expr ASTERISK unary_expr {
    $$ = new BinaryOp(BinaryOp::MUL, $1, $3);
  }
| multiplicative_expr SLASH unary_expr {
    $$ = new BinaryOp(BinaryOp::DIV, $1, $3);
  }
| unary_expr {
    $$ = $1;
  }
;

unary_expr
: field_ref {
    $$ = $1;
  }
| int_constant {
    $$ = new IntConstant($1);
  }
| double_constant {
    $$ = new FP32Constant($1);
  }
| OPENPARENS expression CLOSEPARENS {
    $$ = $2;
  }
;

field_ref
: IDENT offset_list {
    Grid *G = Parser->getGrid();
    Field *F = G->getFieldByName(*$1);
    $$ = new FieldRef(F, *$2);
    delete $2;
  }
;

offset_list
: dim_offset offset_list {
    $$ = $2;
    $$->push_back(new IntConstant($1));
  }
| dim_offset {
    $$ = new std::vector<IntConstant*>();
    $$->push_back(new IntConstant($1));
  }
;

dim_offset
: OPENBRACE int_constant CLOSEBRACE {
    $$ = $2;
  }
;

int_constant
: MINUS INTCONST {
    $$ = -$2;
  }
| INTCONST {
    $$ = $1;
  }
;

double_constant
: DOUBLECONST {
    $$ = $1;
  }
;

type
: FLOAT {
    $$ = new FP32Type();
  }
| DOUBLE {
    llvm_unreachable("Double is not supported!");
    $$ = NULL;
  }
;

copy_semantic
: INOUT
| IN
| OUT
;

%%

void SSPerror(SSPParser *Parser, const char *Str) {
  std::cerr << Str << "\n";
}
