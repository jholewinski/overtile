/*
 * SSPParser.cpp: This file is part of the OverTile project.
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
 * @file: SSPParser.cpp
 * @author: Justin Holewinski <justin.holewinski@gmail.com>
 */

#include "overtile/Core/Error.h"
#include "overtile/Core/Expressions.h"
#include "overtile/Core/Function.h"
#include "overtile/Core/Types.h"
#include "overtile/Parser/SSPParser.h"
#include "SSPParserGenerated.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

extern int SSPparse(overtile::SSPParser*);

namespace {
inline bool IsAlpha(char Ch) {
  return ((Ch >= 'A' && Ch <= 'Z') || (Ch >= 'a' && Ch <= 'z'));
}

inline bool IsDigit(char Ch) {
  return (Ch >= '0' && Ch <= '9');
}

inline bool IsAlphaOrDigit(char Ch) {
  return IsAlpha(Ch) || IsDigit(Ch);
}
}

namespace overtile {

SSPParser::SSPParser(MemoryBuffer *Buffer, SourceMgr &SM)
  : SrcMgr(SM),
    Buf(Buffer),
    G(NULL) {
  SM.AddNewSourceBuffer(Buffer, SMLoc());
}

SSPParser::~SSPParser() {
}

error_code SSPParser::parseBuffer() {

  // Set the lexer position to the beginning of the stream
  CurPos = 0;
  
  int error = SSPparse(this);

  if (error != 0) {
    printError("Error(s) occurred during parse!", false);
    return make_error_code(overtile_error::ssp_parse_error);
  }

  for (unsigned i = 0, e = InternedStrings.size(); i != e; ++i) {
    delete InternedStrings[i];
  }  
  InternedStrings.clear();
  
  return make_error_code(overtile_error::success);
}

int SSPParser::getNextToken(void *Val) {
  YYSTYPE *Value = static_cast<YYSTYPE*>(Val);

  StringRef    BufferData = Buf->getBuffer();
  const char  *Data       = BufferData.data();
  const char*  CurCh      = Data+CurPos;

  while (CurPos < BufferData.size()                         &&
         (*CurCh == '\t' || *CurCh == ' ' || *CurCh == '\r' ||
          *CurCh == '\n')) {
    CurPos++;
    CurCh         = Data+CurPos;
  }
  
  if (CurPos >= BufferData.size())
    return 0;                   // EOF

  if (*CurCh == '+') {
    CurPos++;
    return PLUS;
  } else if (*CurCh == '-') {
    CurPos++;
    return MINUS;
  } else if (*CurCh == '*') {
    CurPos++;
    return ASTERISK;
  } else if (*CurCh == '/') {
    CurPos++;
    return SLASH;
  } else if (*CurCh == '$') {
    CurPos++;
    return DOLLAR;
  } else if (*CurCh == '@') {
    CurPos++;
    return AT;
  } else if (IsAlpha(*CurCh)) {
    const char *Start  = CurCh;
    size_t      Length = 0;
    
    do {
      Length++;
      CurPos++;
      CurCh = Data+CurPos;

    } while (CurPos < BufferData.size() && (IsAlphaOrDigit(*CurCh) || *CurCh == '_'));

    StringRef *Str = new StringRef(Start, Length);

    // Check for keywords
    if (Str->compare("double")   == 0) {
      return DOUBLE;
    } else if (Str->compare("field")    == 0) {
      return FIELD;
    } else if (Str->compare("float")    == 0) {
      return FLOAT;
    } else if (Str->compare("grid")     == 0) {
      return GRID;
    } else if (Str->compare("in")       == 0) {
      return IN;
    } else if (Str->compare("inout")    == 0) {
      return INOUT;
    } else if (Str->compare("is")       == 0) {
      return IS;
    } else if (Str->compare("let")      == 0) {
      return LET;
    } else if (Str->compare("out")      == 0) {
      return OUT;
    } else if (Str->compare("param")    == 0) {
      return PARAM;
    } else if (Str->compare("program")  == 0) {
      return PROGRAM;
    }

    // Not a keyword
    InternedStrings.push_back(Str);
    Value->Ident        = Str;
    return IDENT;
  } else if (IsDigit(*CurCh)) {
    const char *Start   = CurCh;
    size_t      Length  = 0;
    bool        IsFloat = false;
      
    do {
      if (*CurCh == '.') IsFloat = true;
      
      Length++;
      CurPos++;
      CurCh = Data+CurPos;
      
    } while (CurPos < BufferData.size() && (IsDigit(*CurCh) || *CurCh == '.'));

    if (CurPos < BufferData.size() && (*CurCh == 'e' || *CurCh == 'E')) {
      // Start of an exponent

      IsFloat = true;
      
      CurPos++;
      CurCh = Data+CurPos;
      Length++;
      
      if (CurPos == BufferData.size() || (!IsDigit(*CurCh) && *CurCh != '-')) {
        SrcMgr.PrintMessage(SMLoc::getFromPointer(Data+CurPos),
                            SourceMgr::DK_Error, "Missing exponent");
        return 0;
      }

      if (*CurCh == '-') {
        Length++;
        CurPos++;
        CurCh = Data+CurPos;

        if (CurPos == BufferData.size() || !IsDigit(*CurCh)) {
          SrcMgr.PrintMessage(SMLoc::getFromPointer(Data+CurPos),
                              SourceMgr::DK_Error, "Missing exponent");
          return 0;
        }
      }

      do {
        Length++;
        CurPos++;
        CurCh = Data+CurPos;
      
      } while (CurPos < BufferData.size() && IsDigit(*CurCh));

    }
    
    StringRef Str = StringRef(Start, Length);

    if (IsFloat) {
      APFloat DoubleValue = APFloat(APFloat::IEEEdouble, Str);
      Value->DoubleConst  = DoubleValue.convertToDouble();
      return DOUBLECONST;
    } else {
      long    IntValue    = atol(Str.data());
      Value->IntConst     = IntValue;
      return INTCONST;
    }
  } else if (*CurCh == '=') {
    CurPos++;
    return EQUALS;
  } else if (*CurCh == '(') {
    CurPos++;
    return OPENPARENS;
  } else if (*CurCh == ')') {
    CurPos++;
    return CLOSEPARENS;
  } else if (*CurCh == '[') {
    CurPos++;
    return OPENBRACE;
  } else if (*CurCh == ']') {
    CurPos++;
    return CLOSEBRACE;
  } else if (*CurCh == ',') {
    CurPos++;
    return COMMA;
  } else if (*CurCh == ':') {
    CurPos++;
    return COLON;
  }

  CurPos++;
  // If we get here, then we have no idea how to lex this!
  printError("Unknown symbol");
  
  return 0;
}

void SSPParser::printError(StringRef Msg, bool PrintLoc) {
  StringRef   BufferData = Buf->getBuffer();
  const char *Data       = BufferData.data();
  unsigned    Pos        = CurPos;

  if (Pos > 0) {
    // The actual error will be one less than the *current* character
    --Pos;
  }
  
  if (Pos >= BufferData.size()) {
    Pos = BufferData.size()-1;
  }

  SMLoc Loc = SMLoc();
  if (PrintLoc) {
    Loc     = SMLoc::getFromPointer(Data+Pos);
  }
  
  SrcMgr.PrintMessage(Loc, SourceMgr::DK_Error, Msg);
}

}
