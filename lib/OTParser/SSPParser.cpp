
#include "overtile/Core/Error.h"
#include "overtile/Parser/SSPParser.h"
#include "SSPParserGenerated.h"

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

  if (IsAlpha(*CurCh)) {
    const char *Start  = CurCh;
    size_t      Length = 0;
    
    do {
      Length++;
      CurPos++;
      CurCh = Data+CurPos;
    } while (CurPos < BufferData.size() && IsAlpha(*CurCh));

    StringRef Str = StringRef(Start, Length);

    // Check for keywords
    if (Str.compare("function") == 0) {
      return FUNCTION;
    }
    
    // Not a keyword
    InternedStrings.push_back(Str);
    Value->Ident = &(InternedStrings.back());
    return IDENT;
  }

  // If we get here, then we have no idea how to lex this!
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Data+CurPos),
                      SourceMgr::DK_Error, "Invalid symbol");
  
  return 0;
}

}
