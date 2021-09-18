#include <stdlib.h>
#include <string.h>
#include <parse.h>
#include <emit.h>

/**
   Wat AST generation
 */

WatElem* watElem() {
  return malloc(sizeof(WatElem));
}

WatElem* watRawStr(char* str) {
  WatElem* result = watElem();
  result->type = WAT_RAW;
  result->val.str = str;
  return result;
}

WatElem* watKeyword(WatElemType type) {
  WatElem* result = watElem();
  result->type = type;
  return result;
}

WatElem* watType(WatElemType type) {
  return watKeyword(type);
}

// TODO Intern strings and pass those around, instead of char arrays
WatElem* watStringLiteral(char* name) {
  WatElem* result = watElem();
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
  WatList* list = malloc(sizeof(WatList));
  list->elems = malloc(sizeof(WatElem*) * 128); // TODO: Don't hard code this
  list->elemCount = 0;
  result->type = WAT_LIST;
  result->val.list = list;
  return result;
}

void watListInsert(WatElem* listElem, WatElem* elem) {
  WatList* list = listElem->val.list;
  list->elems[list->elemCount] = elem;
  list->elemCount++;
}

void watInsertAllParams(WatElem* result, List* paramList) {
  for (int i = 0; i < paramList->elemCount; i+=2) {
    if (i == paramList->elemCount - 1 && i % 2 == 0) {
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

      Elem* paramNameIdent = paramList->elems[i];
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
  WatElem* funcKW = watKeyword(WAT_KW_FUNC);
  watListInsert(result, funcKW);

  char* funcNameStr = elemIdentName(list->elems[1]);
  WatElem* funcNameElem = watVar(funcNameStr);
  watListInsert(result, funcNameElem);

  WatElem* exportList = watList();
  WatElem* exportKW = watKeyword(WAT_KW_EXPORT);
  WatElem* funcNameExportLiteral = watStringLiteral(funcNameStr);
  watListInsert(exportList, exportKW);
  watListInsert(exportList, funcNameExportLiteral);

  List* paramList = elemList(list->elems[2]);
  watInsertAllParams(result, paramList);

  // Finally add the body
  // Add all the list elems to the funcList body
  for (int i = 3; i < list->elemCount; i++) {
    Elem* expressionElem = list->elems[i];
    List* expressionList = elemList(expressionElem);

    // Two cases initially:
    // 1. macro call ("wasm")
    // 2. function invocation

    // Check the first identifier
    // if the element is "wasm", just straight up copy

    if (strcmp(elemIdentName(expressionList->elems[0]), "wasm")) {
      // add the rest, bypassing any categorisation (just using raw).
      // TODO Consider actually parsing each one. This would be nice
      // because we could show errors for malformed WASM
      for (int i = 1; i < expressionList->elemCount; i++) {
	watListInsert(result, watRawStr(elemIdentName(expressionList->elems[i])));
      }
    }

    // otherwise it is a function invocation.
    // push the arguments onto the stack, and call
    // TODO Handle function invocation
  }
}

/**
   A program is an array of s-expressions
 */
void programToWat(WatElem* wat, Program* program) {
  // All WASM programs have a module keyword
  WatElem* moduleKW = watKeyword(WAT_KW_MODULE);
  watListInsert(wat, moduleKW);

  for (unsigned int i = 0; i < program->listCount; i++) {
    List* list = program->lists[i];
    WatElem* nextFunction = watList();
    funcToWat(nextFunction, list);
    watListInsert(wat, nextFunction);
  }
}

/**
   Wat string output generation
 */

void emit(Program* program) {
  WatElem* wat = watList();
  programToWat(wat, program);

  // Stringify the wat

  // And print it to stdout
}
