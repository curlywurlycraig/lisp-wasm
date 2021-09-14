#include <stdlib.h>

#include <parse.h>

/**
   Wat AST generation
 */

typedef enum WatKeyword {
  KW_MODULE,
  KW_FUNC,
  KW_EXPORT,
  KW_PARAM,
  KW_RESULT
} WatKeyword;

typedef enum WatType {
  TYPE_I32
} WatType;

typedef enum WatOp {
  OP_I32_ADD,
  OP_LOCAL_GET
} WatOp;

typedef enum WatElemType {
  WAT_KW,
  WAT_TYPE,
  WAT_OP,
  WAT_LITERAL,
  WAT_LIST
} WatElemType;

typedef enum WatLiteralType {
  L_NUM,
  L_STR
} WatLiteralType;

typedef struct WatLiteral {
  WatLiteralType type;
  union {
    int num; // TODO floats?
    char* str;
  } val;
} WatLiteralType;

typedef struct WatElem {
  WatElemType type;
  union {
    WatKeyword kw;
    WatType type;
    WatOp op;
    WatList list;
  } val;
} WatElem;

typedef struct WatList {
  WatElem** elems;
  unsigned int elemCount;
} WatList;

/**
   A program is an array of s-expressions
 */
void programToWat(WatList* wat, List** program) {
  WatElem** elems = malloc(sizeof(WatElem*) * 1024);
  wat->elems = elems;
  wat->elemCount = 0;

  // All WASM programs have a module keyword
  WatElem* module = malloc(sizeof(WatElem));
  module->type = WAT_KW;
  module->val.kw = KW_MODULE;
  elems[wat->elemCount] = module;
  wat->elemCount++;

  // First steps: add a function as defined in program
  // For now assuming a program is a single function definition.
  // TODO Make constructor functions

  WatElem* funcDef = malloc(sizeof(WatElem));
  funcDef->type = WAT_LIST;
  funcDef->val.list = funcList;
}

/**
   Wat string output generation
 */

void emit(List** program) {
  WatList* wat = malloc(sizeof(WatList));
  programToWat(wat, program);

  // Stringify the wat

  // And print it to stdout
}
