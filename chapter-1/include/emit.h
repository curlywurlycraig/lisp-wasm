#ifndef __EMIT_H__
#define __EMIT_H__

#include <parse.h>

typedef enum WatElemType {
	WAT_VAR,
	WAT_LITERAL,
	WAT_LIST,
	WAT_STRING_LITERAL,
	WAT_NUMBER_LITERAL,

	// Operators
	WAT_OP_I32_ADD,
	WAT_OP_LOCAL_GET,

	// Types
	WAT_TYPE_I32,

	// Keywords
	WAT_KW_MODULE,
	WAT_KW_FUNC,
	WAT_KW_EXPORT,
	WAT_KW_PARAM,
	WAT_KW_RESULT,
	WAT_KW_CALL,
	WAT_KW_I32_CONST,
	WAT_KW_LOCAL_GET,

	// Raw, emitted directly as their content. Useful for contents of (wasm ...)
	// special form, as it avoids having to parse each element inside the special form as
	// WAT.
	// TOOD In the future it would probably be good to parse it though
	WAT_RAW
} WatElemType;

typedef struct WatList WatList;
struct WatList;

typedef struct WatElem {
	WatElemType type;
	union {
		char* str;
		int i32;
		WatList* list;
	} val;
} WatElem;

typedef struct WatList {
	WatElem** elems;
	unsigned int elemCount;
} WatList;

void freeWatElem(WatElem* elem);

WatElem* watElem();

WatElem* watRawStr(char* str);

WatElem* watKeyword(WatElemType type);

WatElem* watType(WatElemType type);

WatElem* watStringLiteral(char* name);

WatElem* watNumberLiteral(int num);

WatElem* watVar(char* name);

WatElem* watList();

void watListInsert(WatElem* listElem, WatElem* elem);

void watInsertAllParams(WatElem* result, List* paramList);

void funcToWat(WatElem* result, List* list);

void programToWat(WatElem* wat, Program* program);

void printWatStrType(WatElem* wat);

void printWatStrLiteral(WatElem* wat);

void printWatNumberLiteral(WatElem* wat);

void printWatList(WatElem* wat);

void printWatElem(WatElem* wat);

void emit(Program* program);


#endif
