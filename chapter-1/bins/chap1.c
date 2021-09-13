#include <stdio.h>
#include <stdlib.h>

#include <parse.h>

#define MAX_PROGRAM_CHARS 1024

char* formulaInput;

int main(int argc, char** argv) {
  // Read filename from args
  char* filename = argv[1];
  (void)filename;
  (void)argc;

  // Read file
  FILE* fp = fopen("example.wsp", "rb");

  if (!fp) {
    printf("failed to read file.");
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  int length = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* programBuffer = malloc(length);

  if (!programBuffer) {
    printf("couldn't allocate program buffer");
    return 2;
  }

  fread(programBuffer, length, 1, fp);
  fclose(fp);

  // Do simple compilation
  initTokenFinders();
  TokenizeResult* tokens = malloc(sizeof(TokenizeResult));
  tokenize(tokens, programBuffer);

  ParseInfo* parseInfo = malloc(sizeof(ParseInfo));
  parseInfo->tokenIndex = 0;
  parseInfo->didFail = 0;
  parseInfo->reachedEnd = 0;
  parseInfo->tokenizeResult = tokens;
  parseInfo->raw = programBuffer;

  List* result = list(parseInfo);

  if (result->didFail) {
    printf("Failed to parse a list");
    return 3;
  }

  printf("list containing:\n");
  for (int i = 0; i < result->elemCount; i++) {
    Elem* elem = result->elems[i];
    if (elem->type == E_IDENT) {
      if (elem->val.ident.type == I_VAR) {
	printf("\t%s\n", elem->val.ident.val.name);
      } else if (elem->val.ident.type == I_NUM) {
	printf("\t%f\n", elem->val.ident.val.num);
      }
    }
  }

  return 0;
}
