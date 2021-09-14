#include <stdlib.h>

#include <parse.h>

/**
   Wat AST generation
 */

typedef enum WatElemType {
  WAT_VAR,
  WAT_KW,
  WAT_TYPE,
  WAT_LITERAL,
  WAT_LIST,
  WAT_STRING_LITERAL,
  WAT_NUMBER_LITERAL,

  // Operators
  WAT_OP_I32_ADD,
  WAT_OP_LOCAL_GET

  // Types
  WAT_TYPE_I32

  // Keywords
  WAT_KW_MODULE,
  WAT_KW_FUNC,
  WAT_KW_EXPORT,
  WAT_KW_PARAM,
  WAT_KW_RESULT
} WatElemType;

typedef struct WatElem {
  WatElemType type;
  union {
    char* str;
    int i32;
    WatList list;
  } val;
} WatElem;

typedef struct WatList {
  WatElem** elems;
  unsigned int elemCount;
} WatList;

void watElem(WatElem* result, WatType type) {
  result->type = type;
}

// TODO Intern strings and pass those around, instead of char arrays
void watStringLiteral(WatElem* result, char* name) {
  result->type = WAT_LITERAL;
  result->val.str = name;
}

void watVar(WatElem* result, char* name) {
  result->type = WAT_VAR;
  result->val.str = name;
}

void watList(WatElem* result) {
  WatList funcList = (WatList) {
    .elems = malloc(sizeof(WatElem*) * 128); // TODO: Don't hard code this
    .elemCount = 0
  };
  result->type = WAT_LIST;
  result->val.list = funcList;
}

void watListInsert(WatList* list, WatElem* elem) {
  list->elems[list->elemCount] = elem;
  list->elemCount++;
}

void funcToWat(WatElem* result, List* list) {
  watList(result);

  WatElem* funcKW = malloc(sizeof(WatElem));
  watElem(funcKW, WAT_KW_FUNC)
  watListInsert(&funcList, funcKW);

  char* funcNameStr = elemIdentName(list->elems[0]);
  WatElem* funcNameElem = malloc(sizeof(WatElem));
  watVar(funcNameElem, funcNameStr);
  watListInsert(&funcList, funcNameElem);

  WatElem* exportList = malloc(sizeof(WatElem));
  watList(exportList);
  WatElem* exportKW = malloc(sizeof(WatElem));
  watElem(exportKW, WAT_KW_EXPORT);
  WatElem* funcNameExportLiteral = malloc(sizeof(WatElem));
  watStringLiteral(funcNameExportLiteral, funcNameStr);
  watListInsert(&(exportList->val.list), exportKW);
  watListInsert(&(exportList->val.list), funcNameExportLiteral);

  // Add param list

  // Add result list

  // Finally add the body
  
  // Add all the list elems to the funcList body
  for (int i = 0; i < list->elemCount; i++) {

  }
}

/**
   A program is an array of s-expressions
 */
void programToWat(WatList* wat, Program* program) {
  WatElem** elems = malloc(sizeof(WatElem*) * 1024);
  wat->elems = elems;
  wat->elemCount = 0;

  // All WASM programs have a module keyword
  WatElem* module = malloc(sizeof(WatElem));
  module->type = WAT_KW;
  module->val.kw = KW_MODULE;
  elems[wat->elemCount] = module;
  wat->elemCounf++;

  // First steps: add a function as defined in program
  // For now assuming only function definitions can be expressed

  for (int i = 0; i < program->listCount; i++) {
    List* list = program->lists[i];
    WatElem* nextElem = malloc(sizeof(WatElem));
    funcToWat
  }

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
