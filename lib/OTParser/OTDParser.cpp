
#include "overtile/Core/Error.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Field.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Types.h"
#include "overtile/Parser/OTDParser.h"

#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

using namespace llvm;

namespace overtile {

namespace {
/// OTDParser - Parser implementation for OverTile Descriptor files.
class OTDParser {
public:
  OTDParser(yaml::Stream &S);
  ~OTDParser();

  /// getGrid - Returns the Grid instance created by the parser. If this method
  /// is called, it is assumed that the parse was successful so it becomes the
  /// responsibility of the caller to delete the Grid instance.
  Grid *getGrid() {
    Grid *Ptr = G;
    assert(Ptr != NULL && "G is NULL");
    
    G = NULL;
    return Ptr;
  }

  error_code parse();
  
private:

  enum TokenType {
    TOKEN_OPEN_PAREN = 0,
    TOKEN_CLOSE_PAREN,
    TOKEN_IDENT,
    TOKEN_CONST,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_END
  };

  struct Token {
    std::string Str;
    TokenType Type;
  };

  class Lexer {
  public:
    Lexer(llvm::StringRef Str);

    error_code advance() { return getNextToken(Current); }
    Token getCurrentToken() { return Current; }
    
  private:

    error_code getNextToken(Token &T);
    
    std::string ExprStr;
    size_t Idx;
    Token Current;
  };



  error_code parseTopLevel(yaml::KeyValueNode &N);
  error_code parseFieldSequence(yaml::SequenceNode *N);
  error_code parseFunctionSequence(yaml::SequenceNode *N);
  error_code parseField(yaml::MappingNode *N);
  error_code parseFunction(yaml::MappingNode *N);
  error_code parseExpr(Lexer &L, Expression **Expr, yaml::Node *N);
  
  yaml::Stream &Str;
  Grid         *G;
};

OTDParser::OTDParser(yaml::Stream &S)
  : Str(S), G(new Grid()) {
}

OTDParser::~OTDParser() {
  if (G) delete G;
}

error_code OTDParser::parse() {
  for (yaml::document_iterator I = Str.begin(), E = Str.end(); I != E; ++I) {
    yaml::Node *N = I->getRoot();
    yaml::MappingNode *M = dyn_cast<yaml::MappingNode>(N);
    if (!M) {
      Str.printError(N, "Expected YAML Mapping node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    // Parse the top-level entities
    for (yaml::MappingNode::iterator MI = M->begin(), ME = M->end(); MI != ME;
         ++MI) {
      yaml::KeyValueNode &KV = *MI;
      if (error_code ec = parseTopLevel(KV)) {
        return ec;
      }
    }
  }

  return make_error_code(overtile_error::success);
}

error_code OTDParser::parseTopLevel(yaml::KeyValueNode &N) {
  yaml::ScalarNode *Key = dyn_cast<yaml::ScalarNode>(N.getKey());
  if (!Key) {
    Str.printError(&N, "Expected YAML Scalar node");
    return make_error_code(overtile_error::invalid_yaml);
  }

  StringRef KeyStr = Key->getRawValue();

  if (KeyStr.equals("name")) {
    yaml::ScalarNode *SN = dyn_cast<yaml::ScalarNode>(N.getValue());
    if (!SN) {
      Str.printError(N.getValue(), "Expected YAML Scalar node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    G->setName(SN->getRawValue());
  } else if (KeyStr.equals("dim")) {
    yaml::ScalarNode *SN = dyn_cast<yaml::ScalarNode>(N.getValue());
    if (!SN) {
      Str.printError(N.getValue(), "Expected YAML Scalar node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    int Dim = atoi(SN->getRawValue().data());
    if (Dim < 1) {
      Str.printError(N.getValue(), "Dimensionality must be 1 or greater");
      return make_error_code(overtile_error::invalid_yaml);
    }
    
    G->setNumDimensions(Dim);
  } else if (KeyStr.equals("fields")) {
    yaml::SequenceNode *SN = dyn_cast<yaml::SequenceNode>(N.getValue());
    if (!SN) {
      Str.printError(N.getValue(), "Expected YAML Sequence node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    if (error_code ec = parseFieldSequence(SN)) {
      return ec;
    }
  } else if (KeyStr.equals("functions")) {
    yaml::SequenceNode *SN = dyn_cast<yaml::SequenceNode>(N.getValue());
    if (!SN) {
      Str.printError(N.getValue(), "Expected YAML Sequence node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    if (error_code ec = parseFunctionSequence(SN)) {
      return ec;
    }
  } else {
    Str.printError(N.getKey(), "Unknown top-level key");
    return make_error_code(overtile_error::invalid_yaml);
  }
  
  return make_error_code(overtile_error::success);
}

error_code OTDParser::parseFieldSequence(yaml::SequenceNode *N) {
  for (yaml::SequenceNode::iterator I = N->begin(), E = N->end(); I != E; ++I) {
    yaml::Node &SeqN = *I;
    yaml::MappingNode *Map = dyn_cast<yaml::MappingNode>(&SeqN);
    if (!Map) {
      Str.printError(&SeqN, "Expected YAML Mapping node");
      return make_error_code(overtile_error::success);
    }

    if (error_code ec = parseField(Map)) {
      return ec;
    }
  }
  
  return make_error_code(overtile_error::success);
}

error_code OTDParser::parseFunctionSequence(yaml::SequenceNode *N) {
  for (yaml::SequenceNode::iterator I = N->begin(), E = N->end(); I != E; ++I) {
    yaml::Node &SeqN = *I;
    yaml::MappingNode *Map = dyn_cast<yaml::MappingNode>(&SeqN);
    if (!Map) {
      Str.printError(&SeqN, "Expected YAML Mapping node");
      return make_error_code(overtile_error::success);
    }

    if (error_code ec = parseFunction(Map)) {
      return ec;
    }
  }
  
  return make_error_code(overtile_error::success);
}

error_code OTDParser::parseField(yaml::MappingNode *N) {
  StringRef Name;
  StringRef TypeName;
  
  for (yaml::MappingNode::iterator I = N->begin(), E = N->end(); I != E; ++I) {
    yaml::KeyValueNode &KV = *I;
    yaml::ScalarNode *Key = dyn_cast<yaml::ScalarNode>(KV.getKey());
    if (!Key) {
      Str.printError(KV.getKey(), "Expected YAML Scalar node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    StringRef KeyStr = Key->getRawValue();

    if (KeyStr.equals("name")) {
      yaml::ScalarNode *Value = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }
      Name = Value->getRawValue();
    } else if (KeyStr.equals("type")) {
      yaml::ScalarNode *Value = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }
      TypeName = Value->getRawValue();    
    } else {
      Str.printError(Key, "Unknown field key");
      return make_error_code(overtile_error::invalid_yaml);
    }
  }

  // We've parsed the fields, so create a Field object.
  if (Name.empty()) {
    Str.printError(N, "Missing 'name' field");
    return make_error_code(overtile_error::success);
  }
  if (TypeName.empty()) {
    Str.printError(N, "Missing 'type' field");
    return make_error_code(overtile_error::success);
  }

  // Convert type name to an actual type instance
  ElementType *Type = NULL;
  if (TypeName.equals("float")) {
    Type = new FP32Type();
  } else {
    Str.printError(N, "Bad type: " + TypeName);
    return make_error_code(overtile_error::success);
  }

  Field *F = new Field(G, Type, Name);
  
  return make_error_code(overtile_error::success);
}

error_code OTDParser::parseFunction(yaml::MappingNode *N) {
  StringRef Name;
  StringRef OutFieldName;
  Expression *Expr;
  std::vector<std::pair<unsigned, unsigned> > Bounds;
  
  for (yaml::MappingNode::iterator I = N->begin(), E = N->end(); I != E; ++I) {
    yaml::KeyValueNode &KV = *I;
    yaml::ScalarNode *Key = dyn_cast<yaml::ScalarNode>(KV.getKey());
    if (!Key) {
      Str.printError(KV.getKey(), "Expected YAML Scalar node");
      return make_error_code(overtile_error::invalid_yaml);
    }

    StringRef KeyStr = Key->getRawValue();

    if (KeyStr.equals("name")) {
      yaml::ScalarNode *Value = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }
      Name = Value->getRawValue();
    } else if (KeyStr.equals("output")) {
      yaml::ScalarNode *Value = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }
      OutFieldName = Value->getRawValue();    
    } else if (KeyStr.equals("expr")) {
      yaml::ScalarNode *Value = dyn_cast<yaml::ScalarNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }
      StringRef ExprStr = Value->getRawValue();
      Lexer L(ExprStr);
      if (error_code ec = parseExpr(L, &Expr, Value)) {
        return ec;
      }
      assert(Expr != NULL &&
        "parseExpr was successful but did not supply an Expression");
    } else if (KeyStr.equals("bounds")) {
      yaml::SequenceNode *Value = dyn_cast<yaml::SequenceNode>(KV.getValue());
      if (!Value) {
        Str.printError(KV.getValue(), "Expected YAML Scalar node");
        return make_error_code(overtile_error::invalid_yaml);
      }

      for (yaml::SequenceNode::iterator SI = Value->begin(), SE = Value->end();
           SI != SE; ++SI) {
        unsigned Lower = 0;
        unsigned Upper = 0;
        yaml::Node &Root = *SI;
        yaml::MappingNode *M = dyn_cast<yaml::MappingNode>(&Root);
        if (!M) {
          Str.printError(&Root, "Expected YAML Mapping node");
          return make_error_code(overtile_error::invalid_yaml);
        }
        for (yaml::MappingNode::iterator MI = M->begin(), ME = M->end();
             MI != ME; ++MI) {
          yaml::ScalarNode *BoundKey = dyn_cast<yaml::ScalarNode>(MI->getKey());
          yaml::ScalarNode *BoundValue =
            dyn_cast<yaml::ScalarNode>(MI->getValue());
          if (!BoundKey) {
            Str.printError(MI->getKey(), "Expected YAML Scalar node");
            return make_error_code(overtile_error::success);
          }
          if (!BoundValue) {
            Str.printError(MI->getValue(), "Expected YAML Scalar node");
            return make_error_code(overtile_error::success);
          }
          if (BoundKey->getRawValue().equals("lower")) {
            Lower = atoi(BoundValue->getRawValue().data());
          } else if (BoundKey->getRawValue().equals("upper")) {
            Upper = atoi(BoundValue->getRawValue().data());
          } else {
            Str.printError(BoundKey, "Unknown bounds key");
            return make_error_code(overtile_error::invalid_yaml);
          }
        }

        Bounds.push_back(std::make_pair(Lower, Upper));
      }
    } else {
      Str.printError(Key, "Unknown function key");
      return make_error_code(overtile_error::invalid_yaml);
    }
  }


  if (Name.empty()) {
    Str.printError(N, "Missing 'name' field");
    return make_error_code(overtile_error::success);
  }
  if (OutFieldName.empty()) {
    Str.printError(N, "Missing 'output' field");
    return make_error_code(overtile_error::success);
  }
  if (!Expr) {
    Str.printError(N, "Missing 'expr' field");
    return make_error_code(overtile_error::success);
  }

  Field *OutField = G->getFieldByName(OutFieldName);
  if (!OutField) {
    Str.printError(N, "Unable to locate output field");
    return make_error_code(overtile_error::success);
  }

  Function *F = new Function(OutField, Expr);

  for (unsigned i = 0, e = Bounds.size(); i < e; ++i) {
    F->setLowerBound(i, Bounds[0].first);
    F->setUpperBound(i, Bounds[0].first);
  }

  G->appendFunction(F);
  
  return make_error_code(overtile_error::success);
}


OTDParser::Lexer::Lexer(llvm::StringRef Str)
  : ExprStr(Str), Idx(0) {
  advance();
}

static bool IsAlpha(char Ch) {
  return ((Ch >= 'A' && Ch <= 'Z') || (Ch >= 'a' && Ch <= 'z'));
}

static bool IsDigit(char Ch) {
  return (Ch >= '0' && Ch <= '9');
}

static bool IsAlphaOrDigit(char Ch) {
  return IsAlpha(Ch) || IsDigit(Ch);
}

error_code OTDParser::Lexer::getNextToken(OTDParser::Token &T) {
  // Parse out the next token

  // Eat whitespace
  while (Idx < ExprStr.size() && (ExprStr[Idx] == ' ' || ExprStr[Idx] == '\t'
         || ExprStr[Idx] == '\r' || ExprStr[Idx] == '\n')) {
    ++Idx;
  }
  
  if (Idx == ExprStr.size()) {
    T.Str = "<END>";
    T.Type = TOKEN_END;
    return make_error_code(overtile_error::success);
  } else {
    char Ch = ExprStr[Idx++];

    if (Ch == '(') {
      T.Str = "(";
      T.Type = TOKEN_OPEN_PAREN;
      return make_error_code(overtile_error::success);
    } else if (Ch == ')') {
      T.Str = ")";
      T.Type = TOKEN_CLOSE_PAREN;
      return make_error_code(overtile_error::success);
    } else if (Ch == '+') {
      T.Str = "+";
      T.Type = TOKEN_PLUS;
      return make_error_code(overtile_error::success);
    } else if (Ch == '-') {
      // This could be a literal '-' or the start of a constant
      if (Idx < ExprStr.size() && IsDigit(ExprStr[Idx])) {
        // This is a constant
        std::string Str = "-";
        do {
          Str += ExprStr[Idx++];
        } while (Idx < ExprStr.size() && (IsDigit(ExprStr[Idx])
                 || ExprStr[Idx] == '.'));
        T.Str = Str;
        T.Type = TOKEN_CONST;        
      } else {
        T.Str = "-";
        T.Type = TOKEN_MINUS;
      }
      return make_error_code(overtile_error::success);
    } else if (Ch == '*') {
      T.Str = "*";
      T.Type = TOKEN_ASTERISK;
      return make_error_code(overtile_error::success);
    } else if (Ch == '/') {
      T.Str = "/";
      T.Type = TOKEN_SLASH;
      return make_error_code(overtile_error::success);
    } else if (IsAlpha(Ch)) {
      std::string Str;
      Str += Ch;
      while (Idx < ExprStr.size() && IsAlphaOrDigit(ExprStr[Idx])) {
        Str += ExprStr[Idx++];
      }
      T.Str = Str;
      T.Type = TOKEN_IDENT;
      return make_error_code(overtile_error::success);
    } else if (IsDigit(Ch)) {
      std::string Str;
      Str += Ch;
      while (Idx < ExprStr.size() && (IsDigit(ExprStr[Idx])
             || ExprStr[Idx] == '.')) {
        Str += ExprStr[Idx++];
      }
      T.Str = Str;
      T.Type = TOKEN_CONST;
      return make_error_code(overtile_error::success);
    } else {
      errs() << "Invalid character: " << Ch << "\n";
      return make_error_code(overtile_error::invalid_yaml);
    }
    
    return make_error_code(overtile_error::success);
  }
}


error_code OTDParser::parseExpr(Lexer &L, Expression **Expr,
                                yaml::Node *N) {

  // Check for base constant
  if (L.getCurrentToken().Type == TOKEN_CONST) {
    // @FIXME: This parsing is seriously borked
    std::string Value = L.getCurrentToken().Str;
    if (Value.find(".") != std::string::npos) {
      // This is a float constant
      *Expr = new FP32Constant(atof(Value.c_str()));
    } else {
      // This is an int constant
      *Expr = new IntConstant(atoi(Value.c_str()));
    }
    L.advance();
    return make_error_code(overtile_error::success);
  }
                                
  // An expression must start with an '('
  if (L.getCurrentToken().Type != TOKEN_OPEN_PAREN) {
    Str.printError(N, "Missing '(' at start of expression");
    return make_error_code(overtile_error::invalid_yaml);
  }

  if (error_code ec = L.advance())
    return ec;

  switch (L.getCurrentToken().Type) {
  default:
    Str.printError(N, "Unexpected token: " + L.getCurrentToken().Str);
    return make_error_code(overtile_error::invalid_yaml);
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_ASTERISK:
  case TOKEN_SLASH: {
    // This is a binary expression
    Expression *LHS;
    Expression *RHS;
    TokenType TokenTy = L.getCurrentToken().Type;

    if (error_code ec = L.advance())
        return ec;
        
    if (error_code ec = parseExpr(L, &LHS, N))
      return ec;
    if (error_code ec = parseExpr(L, &RHS, N))
      return ec;

    BinaryOp::Operator Op;
    switch (TokenTy) {
    default: llvm_unreachable("Unhandled operator");
    case TOKEN_PLUS:     Op = BinaryOp::ADD;
    case TOKEN_MINUS:    Op = BinaryOp::SUB;
    case TOKEN_ASTERISK: Op = BinaryOp::MUL;
    case TOKEN_SLASH:    Op = BinaryOp::DIV;
    }

    *Expr = new BinaryOp(Op, LHS, RHS);
    break;
  }
  case TOKEN_IDENT: {
    if (L.getCurrentToken().Str == "fieldref") {
      std::vector<IntConstant*> Offsets;
      std::string Name;
      
      if (error_code ec = L.advance())
        return ec;

      if (L.getCurrentToken().Type != TOKEN_IDENT) {
        Str.printError(N, "Missing field name in fieldref");
        return make_error_code(overtile_error::invalid_yaml);
      }

      Name = L.getCurrentToken().Str;
        
      if (error_code ec = L.advance())
        return ec;

      while (L.getCurrentToken().Type == TOKEN_CONST) {
        Offsets.push_back(
          new IntConstant(atoi(L.getCurrentToken().Str.c_str())));
        if (error_code ec = L.advance())
        return ec;
      }

      Field *F = G->getFieldByName(Name);
      if (!F) {
        Str.printError(N, "Unknown field: " + Name);
        return make_error_code(overtile_error::invalid_yaml);
      }
      
      *Expr = new FieldRef(F, Offsets);
      break;
    } else {
      Str.printError(N, "Unexpected identifier: " + L.getCurrentToken().Str);
      return make_error_code(overtile_error::invalid_yaml);
    }
  }
  }

  // An expression must end with an ')'
  if (L.getCurrentToken().Type != TOKEN_CLOSE_PAREN) {
    Str.printError(N, "Missing ')' at end of expression");
    return make_error_code(overtile_error::invalid_yaml);
  }

  if (error_code ec = L.advance())
    return ec;
  
  return make_error_code(overtile_error::success);
}


}


error_code ParseOTD(MemoryBuffer *Src, OwningPtr<Grid> &G) {

  SourceMgr    SM;
  yaml::Stream Str(Src->getBuffer(), SM);

  OTDParser Parser(Str);
  if (error_code ec = Parser.parse()) {
    return ec;
  }

  G.reset(Parser.getGrid());
  
  return make_error_code(overtile_error::success);
}

}
