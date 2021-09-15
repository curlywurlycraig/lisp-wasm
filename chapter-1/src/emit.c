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

WatElem* watElem() {
  return malloc(sizeof(WatElem));
}

WatElem* watKeyword(WatType type) {
  WatElem* result = watElem();
  result->type = type;
  return result;
}

WatElem* watType(WatType type) {
  return watKeyword(type);
}

// TODO Intern strings and pass those around, instead of char arrays
WatElem* watStringLiteral(char* name) {
  WatELem* result = watElem();
  result->type = WAT_LITERAL;
  result->val.str = name;
  return result;
}

WatElem* watVar(char* name) {
  WatElem* result = watElem();
  result->type = WAT_VAR;
  result->val.str = name;
  return result;
}

WatElem* watList() {
  WatElem* result = watElem();
  WatList list = (WatList) {
    .elems = malloc(sizeof(WatElem*) * 128); // TODO: Don't hard code this
    .elemCount = 0
  };
  result->type = WAT_LIST;
  result->val.list = list;
  return result;
}

void watListInsert(WatElem* listElem, WatElem* elem) {
  WatList* list = &(listElem->val.list);
  list->elems[list->elemCount] = elem;
  list->elemCount++;
}

void watInsertAllParams(WatElem* result, List* paramList) {
  for (int i = 0; i < paramList.length; i+=2) {
    if (i == paramList.length - 1 && i % 2 == 0) {
      // Return type
      WatElem* resultKW = watKeyword(WAT_KW_RESULT);

      //Elem* returnTypeIdent = paramList.elems[i];
      // TODO Read the type instead of hard-coding i32
      WatElem* returnTypeElem = watType(WAT_TYPE_I32);

      WatElem* resultList = watList();
      watListInsert(resultList, resultKW);
      watListInsert(resultList, returnTypeElem);
      watListInsert(result, resultList);
    } else {
      WatElem* paramKW = watKeyword(WAT_KW_PARAM);

      Elem* paramNameIdent = paramList.elems[i];
      char* paramName = elemIdentName(paramNameIdent);
      WatElem* paramNameElem = watVar(paramName);

      // Elem* paramTypeIdent = paramList.elems[i+1];
      // TODO Read the type instead of hard-coding i32
      WatElem* paramTypeElem = watType(WAT_TYPE_I32);

      WatElem* resultList = watList();
      watListInsert(resultList, paramKW);
      watListInsert(resultList, paramNameElem);
      watListInsert(resultList, paramTypeElem);
      watListInsert(result, resultList);
    }
  }
}

void funcToWat(WatElem* result, List* list) {
  watList(result);

  WatElem* funcKW = watKeyword(WAT_KW_FUNC)
  watListInsert(result, funcKW);

  char* funcNameStr = elemIdentName(list->elems[1]);
  WatElem* funcNameElem = watVar(funcNameStr);
  watListInsert(result, funcNameElem);

  WatElem* exportList = watList();
  WatElem* exportKW = watKeyword(WAT_KW_EXPORT);
  WatElem* funcNameExportLiteral = watStringLiteral(funcNameStr);
  watListInsert(exportList), exportKW);
  watListInsert(exportList), funcNameExportLiteral);

  List* paramList = elemList(list->elems[2]);
  watInsertAllParams(result, paramList);

  // Finally add the body
  // Add all the list elems to the funcList body
  for (int i = 0; i < list->elemCount; i++) {
    // TODO Stopping here for the night.
  }
}

/**
   A program is an array of s-expressions
 */
void programToWat(WatElem* wat, Program* program) {
  // All WASM programs have a module keyword
  WatElem* moduleKW = watKeyword(WAT_KW_MODULE);
  watListInsert(wat, moduleKW);

  for (int i = 0; i < program->listCount; i++) {
    List* list = program->lists[i];
    WatElem* nextElem = watList();
    funcToWat(nextElem, list);
    watListInsert(nextElem);
  }
}

/**
   Wat string output generation
 */

void emit(List** program) {
  WatElem* wat = watList();
  programToWat(wat, program);

  // Stringify the wat

  // And print it to stdout
}
