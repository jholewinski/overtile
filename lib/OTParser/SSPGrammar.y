/*
 * SSPGrammar.y: This file is part of the OverTile project.
 *
 * OverTile: Research compiler for overlapped tiling on GPU architectures
 *
 * Copyright (C) 2012, University of California Los Angeles
 *
 * This program can be redistributed and/or modified under the terms
 * of the license specified in the LICENSE.txt file at the root of the
 * project.
 *
 * Contact: P Sadayappan <saday@cse.ohio-state.edu>
 */

/**
 * @file: SSPGrammar.y
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

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

  overtile::Expression                *Expr;
  overtile::ElementType               *Type;
  std::vector<overtile::IntConstant*> *IntList;
  std::pair<unsigned, unsigned>       *Bound;
  std::vector<overtile::FunctionBound> *BoundList;
  std::vector<overtile::Expression*>  *ExprList;
  std::list<overtile::BoundedFunction*> *FuncExprList;
  overtile::BoundExpr BExpr;
  overtile::FunctionBound FuncBound;
  overtile::BoundedFunction *FuncExpr;
}

%token AT
%token DOLLAR
%token DOUBLE
%token FIELD
%token FLOAT
%token GRID
%token IN
%token INOUT
%token IS
%token LET
%token OUT
%token PARAM
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
%type<FuncBound> application_bound
%type<BoundList> application_bounds
%type<ExprList> expr_list
%type<BExpr> bound_expr
%type<FuncExprList> func_expr_list;
%type<FuncExpr> func_expr;

%%

top_level
: PROGRAM IDENT IS grid_def param_list field_list application_list {
    Parser->getGrid()->setName(*$2);
  }
;

grid_def
: GRID INTCONST {
    Grid *G = new Grid($2);
    Parser->setGrid(G);
  }
;

param_list
: /* empty */
| param_def param_list
;

param_def
: PARAM IDENT type {
    Grid *G = Parser->getGrid();
    if (G->doesParameterExist(*$2)) {
      std::string        Msg;
      raw_string_ostream MsgStr(Msg);
      MsgStr << "Parameter '" << (*$2) << "' has already been defined";
      MsgStr.flush();
      yyerror(Parser, Msg.c_str());
      YYERROR;
    }

    G->addParameter(*$2, $3);
  }

field_list
: /* empty */
| field_def field_list
;

field_def
: FIELD IDENT type copy_semantic {
    Grid *G = Parser->getGrid();

    if (G->getFieldByName(*$2) != NULL) {
      std::string        Msg;
      raw_string_ostream MsgStr(Msg);
      MsgStr << "Field '" << (*$2) << "' has already been defined";
      MsgStr.flush();
      yyerror(Parser, Msg.c_str());
      YYERROR;
    }
    
    Field *F = new Field(G, $3, $2->str());
  }
;

application_list
: /* empty */
| application_def application_list
;

application_def
: IDENT EQUALS func_expr_list {
    Grid *G = Parser->getGrid();
    Field *Out = G->getFieldByName(*$1);

    if (Out == NULL) {
      std::string        Msg;
      raw_string_ostream MsgStr(Msg);
      MsgStr << "Field '" << (*$1) << "' has not been defined";
      MsgStr.flush();
      yyerror(Parser, Msg.c_str());
      YYERROR;
    }

    Function *Func = new Function(Out);

    for (std::list<overtile::BoundedFunction*>::iterator I = $3->begin(), E = $3->end(); I != E; ++I) {
      Func->addBoundedFunction(*(*I));
    }

    delete $3;

    G->appendFunction(Func);
  }
;

func_expr_list
: func_expr func_expr_list {
    $$ = $2;
    $$->push_front($1);
  }
| func_expr {
    $$ = new std::list<BoundedFunction*>();
    $$->push_front($1);
  }
;

func_expr
: AT application_bounds COLON expression {
    $$ = new BoundedFunction;
    $$->Bounds = *$2;
    $$->Expr = $4;
    delete $2;
  }
;

application_bounds
: application_bound application_bounds {
    $$ = $2;
    $$->push_back($1);
  }
| application_bound {
    $$ = new std::vector<FunctionBound>();
    $$->push_back($1);
  }
;

application_bound
: OPENBRACE bound_expr COLON bound_expr CLOSEBRACE {
    $$.LowerBound = $2;
    $$.UpperBound = $4;
  }
| OPENBRACE bound_expr CLOSEBRACE {
    $$.LowerBound = $2;
    $$.UpperBound = $2;
  }
;

bound_expr
: int_constant {
    $$.Base = 0;
    $$.Constant = $1;
  }
| DOLLAR {
    $$.Base = (unsigned)(-1);
    $$.Constant = 0;
  }
| DOLLAR MINUS int_constant {
    $$.Base = (unsigned)(-1);
    $$.Constant = $3;
  }

expression
: additive_expr {
    $$ = $1;
  }
| LET IDENT EQUALS expression IN expression {
    $$ = $6;
    $$->replacePlaceHolder(*$2, $4);    
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
| IDENT {
    $$ = new PlaceHolderExpr(*$1);
  }
| int_constant {
    $$ = new IntConstant($1);
  }
| double_constant {
    $$ = new FP32Constant($1);
  }
| IDENT OPENPARENS expr_list CLOSEPARENS {
    std::reverse($3->begin(), $3->end());
    $$ = new FunctionCall(*$1, *$3);
    delete $3;
  }
| OPENPARENS expression CLOSEPARENS {
    $$ = $2;
  }
;

expr_list
: expression COMMA expr_list {
    $$ = $3;
    $$->push_back($1);
  }
| expression {
    $$ = new std::vector<Expression*>();
    $$->push_back($1);
  }
;

field_ref
: IDENT offset_list {
    Grid  *G = Parser->getGrid();
    Field *F = G->getFieldByName(*$1);

    if (F == NULL) {
      std::string        Msg;
      raw_string_ostream MsgStr(Msg);
      MsgStr << "Field '" << (*$1) << "' has not been defined";
      MsgStr.flush();
      yyerror(Parser, Msg.c_str());
      YYERROR;
    }

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
    $$ = new FP64Type();
  }
;

copy_semantic
: INOUT
| IN
| OUT
;

%%

void SSPerror(SSPParser *Parser, const char *Str) {
  Parser->printError(Str);
}
