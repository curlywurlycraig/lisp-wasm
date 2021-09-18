#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <parse.h>
#include <emit.h>

int failCount = 0;
unsigned int testCount = 0;

#define ERROR(...) fprintf(stderr, "ERROR: " __VA_ARGS__)

#define TEST_STR(testName, expected, actual)														\
	testCount++;																													\
	if (!strcmp(actual, expected)) {																			\
		ERROR("Failed test " testName ": expected %s but was %s\n", expected, actual); \
		failCount++;																												\
	} else {																															\
		printf(".");																												\
	}

#define TEST_INT(testName, expected, actual)														\
	testCount++;																													\
	if (actual != expected) {																							\
		ERROR("Failed test " testName ": expected %d but was %d\n", expected, actual); \
		failCount++;																												\
	} else {																															\
		printf(".");																												\
	}

int main() {
	initTokenFinders();

	TokenFinder identifierFinder = makeIdentifierFinder();

	Validity v = validateRange("i32", 0, 3, identifierFinder);
	TEST_INT("validateRange i32 identifier", VALID, v);

	v = validateRange("$myVar", 0, 6, identifierFinder);
	TEST_INT("validateRange $myVar identifier", VALID, v);

	TokenFinder numberFinder = makeNumberFinder();
	v = validateRange("3.", 0, 2, numberFinder);
	TEST_INT("validateRange incomplete number", PARTIAL, v);

  if (!failCount) {
    printf("\n\033[0;32mPASSED %d TESTS\033[0m\n", testCount);
  } else {
    printf("\n\033[0;31mFAILED %d TESTS\033[0m\n", failCount);
  }

  return failCount;
}
