
#include "overtile/Core/Error.h"
#include "overtile/Parser/SSPParser.h"
#include "SSPParserGenerated.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Twine.h"

using namespace llvm;

extern int SSPparse(void*);

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
    Buf(Buffer) {
  SM.AddNewSourceBuffer(Buffer, SMLoc());
}

SSPParser::~SSPParser() {
}

error_code SSPParser::parseBuffer() {

  // Set the lexer position to the beginning of the stream
  CurPos = 0;
  
  int error = SSPparse(this);

  if (error != 0) {
    StringRef   BufferData = Buf->getBuffer();
    const char *Data       = BufferData.data();
    SrcMgr.PrintMessage(SMLoc::getFromPointer(Data+CurPos),
                        SourceMgr::DK_Error, "Parse error");
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
  } else if (IsAlpha(*CurCh)) {
    const char *Start  = CurCh;
    size_t      Length = 0;
    
    do {
      Length++;
      CurPos++;
      CurCh = Data+CurPos;
    } while (CurPos < BufferData.size() && IsAlphaOrDigit(*CurCh));

    StringRef Str = StringRef(Start, Length);

    // Check for keywords
    if (Str.compare("double")          == 0) {
      return DOUBLE;
    } else if (Str.compare("field")    == 0) {
      return FIELD;
    } else if (Str.compare("float")    == 0) {
      return FLOAT;
    } else if (Str.compare("function") == 0) {
      return FUNCTION;
    } else if (Str.compare("grid")     == 0) {
      return GRID;
    } else if (Str.compare("in")       == 0) {
      return IN;
    } else if (Str.compare("inout")    == 0) {
      return INOUT;
    } else if (Str.compare("is")       == 0) {
      return IS;
    } else if (Str.compare("out")      == 0) {
      return OUT;
    } else if (Str.compare("program")  == 0) {
      return PROGRAM;
    }

    
    // Not a keyword
    InternedStrings.push_back(Str);
    Value->Ident        = &(InternedStrings.back());
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

    if (CurPos < BufferData.size() && *CurCh == 'e') {
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

  // If we get here, then we have no idea how to lex this!
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Data+CurPos),
                      SourceMgr::DK_Error, "Invalid symbol");
  
  return 0;
}

}
