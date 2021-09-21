#include <stdio.h>
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

void freeWatElem(WatElem* elem) {
	if (elem->type == WAT_LIST) {
		for (unsigned int i = 0; i < elem->val.list->elemCount; i++) {
			freeWatElem(elem->val.list->elems[i]);
		}
		free(elem->val.list);
	}

	free(elem);
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
	result->type = WAT_STRING_LITERAL;
	result->val.str = name;
	return result;
}

WatElem* watNumberLiteral(int num) {
	WatElem* result = watElem();
	result->type = WAT_NUMBER_LITERAL;
	result->val.i32 = num;
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

void evaluationToWat(WatElem* result, Elem* elem) {
	if (elem->type == E_LIST) {
		List* expressionList = elemList(elem);
		Elem* functionNameElem = expressionList->elems[0];

		if (!strcmp(elemIdentName(functionNameElem), "wasm")) {
			// WASM special form: just copy contents directly
			for (int i = 1; i < expressionList->elemCount; i++) {
				watListInsert(result, watRawStr(elemIdentName(expressionList->elems[i])));
			}

			return;
		}

		// Function invocation
		for (int i = 1; i < expressionList->elemCount; i++) {
			Elem* argElem = expressionList->elems[i];
			evaluationToWat(result, argElem);
		}

		watListInsert(result, watKeyword(WAT_KW_CALL));
		watListInsert(result, watVar(functionNameElem->val.ident.val.name));
	} else if (elem->val.ident.type == I_NUM) {
		watListInsert(result, watKeyword(WAT_KW_I32_CONST));
		watListInsert(result, watNumberLiteral(elem->val.ident.val.num));
	} else if (elem->val.ident.type == I_VAR) {
		watListInsert(result, watKeyword(WAT_KW_LOCAL_GET));
		watListInsert(result, watVar(elem->val.ident.val.name));
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
	watListInsert(result, exportList);

	List* paramList = elemList(list->elems[2]);
	watInsertAllParams(result, paramList);

	// Finally add the body
	// Add all the list elems to the funcList body
	for (int i = 3; i < list->elemCount; i++) {
		Elem* expressionElem = list->elems[i];
		evaluationToWat(result, expressionElem);

		// TODO If it is not the last item in the list, drop the returned value of the evaluation from the stack
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

void printWatStrType(WatElem* wat) {
	printf("%s", wat->val.str);
}

void printWatStrLiteral(WatElem* wat) {
	printf("\"");
	printWatStrType(wat);
	printf("\"");
}

void printWatNumberLiteral(WatElem* wat) {
	printf("%d", wat->val.i32);
}

void printWatList(WatElem* wat) {
	WatList* watList = wat->val.list;
	printf("(");
	for (unsigned int i = 0; i < watList->elemCount; i++) {
		printWatElem(watList->elems[i]);
		if (i != watList->elemCount - 1) {
			printf(" ");
		}
	}
	printf(")");
}

void printWatElem(WatElem* wat) {
	switch (wat->type) {
	case WAT_VAR:
		printf("$");
		printWatStrType(wat);
		break;
	case WAT_LITERAL:
	case WAT_RAW:
		printWatStrType(wat);
		break;
	case WAT_LIST:
		printWatList(wat);
		break;
	case WAT_STRING_LITERAL:
		printWatStrLiteral(wat);
		break;
	case WAT_NUMBER_LITERAL:
		printWatNumberLiteral(wat);
		break;
	case WAT_OP_I32_ADD:
		printf("i32.add");
		break;
	case WAT_OP_LOCAL_GET:
		printf("local.get");
		break;
	case WAT_TYPE_I32:
		printf("i32");
		break;
	case WAT_KW_MODULE:
		printf("module");
		break;
	case WAT_KW_FUNC:
		printf("func");
		break;
	case WAT_KW_EXPORT:
		printf("export");
		break;
	case WAT_KW_PARAM:
		printf("param");
		break;
	case WAT_KW_RESULT:
		printf("result");
		break;
	case WAT_KW_CALL:
		printf("call");
		break;
	case WAT_KW_I32_CONST:
		printf("i32.const");
		break;
	case WAT_KW_LOCAL_GET:
		printf("local.get");
		break;
	default:
		// TODO Handle error case
		return;
	}
}

void emit(Program* program) {
	WatElem* wat = watList();
	programToWat(wat, program);
	printWatElem(wat);
	printf("\n");
	freeWatElem(wat);
}
