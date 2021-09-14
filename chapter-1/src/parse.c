#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <math.h>

#include <assert.h>
#define BUG(assertion)  // used to temporarily disable an assert that shouldn't fail

#define ARRAY_LENGTH(a) sizeof(a) / sizeof((a)[0])

TokenFinder* tokenFinders;
unsigned int numTokenFinders;

static const CharState startState = { .id = -1, .type = START };
static const CharState endState = { .id = -2, .type = END };

CharType getCharType(char input) {
    if (input >= '0' && input <= '9') {
        return DIGIT;
    }

    if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z')) {
        return LETTER;
    }

    if (input == ' ') {
        return SPACE;
    }

    if (input == '\n') {
        return SPACE;
    }

    if (input == '.') {
        return PERIOD;
    }

    if (input == '-') {
        return HYPHEN;
    }

    // Operators
    if (input == '*') {
        return LETTER;
    }

    if (input == '+') {
        return LETTER;
    }

    if (input == '/') {
        return LETTER;
    }

    // Boolean functions like empty?
    if (input == '?') {
        return LETTER;
    }

    // WASM variables like $a
    if (input == '$') {
        return LETTER;
    }

    if (input == '(') {
        return OPEN_PAREN;
    }

    if (input == ')') {
        return CLOSE_PAREN;
    }

    if (input == ':') {
        return COLON;
    }

    return UNKNOWN;
}

Validity validateRange(char* range, int startIndex, int endIndex, TokenFinder finder) {
    CharState currentCharState = startState;
    for (int i = startIndex; i < endIndex; i++) {
        CharType nextCharType = getCharType(range[i]);
        int hasValidTransition = 0;
        // find matching transition
        for (unsigned int j = 0; j < finder.transitionCount; j++) {
            StateTransition transition = finder.transitions[j];
            if (transition.fromState.id == currentCharState.id && transition.toState.type == nextCharType) {
                hasValidTransition = 1;

                currentCharState = transition.toState;
                break;
            }
        }

        if (!hasValidTransition) {
            return INVALID;
        }
    }

    for (unsigned int i = 0; i < finder.transitionCount; i++) {
        StateTransition transition = finder.transitions[i];
        if (transition.fromState.id == currentCharState.id && transition.toState.id == endState.id) {
            return VALID;
        }
    }

    return PARTIAL;
}

TokenFinder makeNumberFinder() {
    static const CharState numberState   = { .id = 0, .type = DIGIT  };
    static const CharState periodState   = { .id = 1, .type = PERIOD };
    static const CharState fractionState = { .id = 2, .type = DIGIT  };
    static const CharState hyphenState   = { .id = 3, .type = HYPHEN };

    // number is
    // 1
    // 1018585
    // 12383.1583
    // -12383.1583
    // NOT 123415. (don't allow trailing period)
    // <some digits><maybe period & more digits>
    static const StateTransition transitions[] = {
      [0] = { .fromState = startState,    .toState = numberState   },
      [1] = { .fromState = numberState,   .toState = endState      },
      [2] = { .fromState = numberState,   .toState = periodState   },
      [3] = { .fromState = numberState,   .toState = numberState   },
      [4] = { .fromState = periodState,   .toState = fractionState },
      [5] = { .fromState = fractionState, .toState = endState      },
      [6] = { .fromState = fractionState, .toState = fractionState },
      // Neg number,
      [7] = { .fromState = startState,    .toState = hyphenState   },
      [8] = { .fromState = hyphenState,   .toState = numberState   },
    };

    return (TokenFinder){
	.token = T_NUMBER,
	.transitionCount = ARRAY_LENGTH(transitions),
	.transitions = transitions,
    };
}

#define SINGLE_CHAR_TRANSITIONS(ty) {    				   \
	{ .fromState = startState, .toState = { .id = 0, .type = (ty) } }, \
	{ .fromState = { .id = 0, .type = (ty) }, .toState = endState }    \
    }

#define REPEATED_CHAR_TRANSITIONS(ty) {    				                  \
	{ .fromState = startState, .toState = { .id = 0, .type = (ty) } },                \
	{ .fromState = { .id = 0, .type = (ty) }, .toState = { .id = 0, .type = (ty) } }, \
	{ .fromState = { .id = 0, .type = (ty) }, .toState = endState }                   \
    }

#define TOKEN_FINDER_FROM_STATIC_TRANSITION_ARRAY(tok, array) {		\
	.token = (tok), .transitionCount = ARRAY_LENGTH(array),		\
	.transitions = array					        \
	}

static const StateTransition openParenTransitions[] = SINGLE_CHAR_TRANSITIONS(OPEN_PAREN);
static const TokenFinder openParenFinder = TOKEN_FINDER_FROM_STATIC_TRANSITION_ARRAY(T_OPEN_PAREN, openParenTransitions);

static const StateTransition closeParenTransitions[] = SINGLE_CHAR_TRANSITIONS(CLOSE_PAREN);
static const TokenFinder closeParenFinder = TOKEN_FINDER_FROM_STATIC_TRANSITION_ARRAY(T_CLOSE_PAREN, closeParenTransitions);

static const StateTransition whitespaceTransitions[] = REPEATED_CHAR_TRANSITIONS(SPACE);
static const TokenFinder whitespaceFinder = TOKEN_FINDER_FROM_STATIC_TRANSITION_ARRAY(T_WHITESPACE, whitespaceTransitions);

TokenFinder makeIdentifierFinder() {
    static const CharState charState   = { .id = 0, .type = LETTER };
    static const CharState hyphenState = { .id = 1, .type = HYPHEN };
    static const CharState periodState = { .id = 2, .type = PERIOD };
    static const CharState numberState = { .id = 2, .type = DIGIT  };

    static const StateTransition transitions[] = {
      [0]  = { .fromState = startState,  .toState = charState   },
      [1]  = { .fromState = startState,  .toState = hyphenState },
      [2]  = { .fromState = charState,   .toState = charState   },
      [3]  = { .fromState = charState,   .toState = numberState },
      [4]  = { .fromState = numberState, .toState = periodState },
      [5]  = { .fromState = numberState, .toState = hyphenState },
      [6]  = { .fromState = numberState, .toState = numberState },
      [7]  = { .fromState = charState,   .toState = periodState },
      [8]  = { .fromState = periodState, .toState = charState   },
      [9]  = { .fromState = charState,   .toState = hyphenState },
      [10] = { .fromState = charState,   .toState = endState    },
      [11] = { .fromState = hyphenState, .toState = charState   },
      [12] = { .fromState = hyphenState, .toState = endState    },
      [13] = { .fromState = numberState, .toState = endState    },
    };

    return (TokenFinder){
	.token = T_IDENT,
	.transitionCount = ARRAY_LENGTH(transitions),
	.transitions = transitions,
    };
}

void initTokenFinders() {
    TokenFinder finders[] = {
      makeNumberFinder(),
      whitespaceFinder,
      openParenFinder,
      closeParenFinder,
      makeIdentifierFinder(),
      makeCellrefFinder(),
    };

    TokenFinder* copy = malloc(sizeof(finders));
    memcpy(copy, finders, sizeof(finders));

    numTokenFinders = ARRAY_LENGTH(finders);
    tokenFinders = copy;
}

void tokenize(TokenizeResult* result, char* formula) {
    result->tokens = malloc(sizeof(TokenInfo) * MAX_TOKENS);
    result->tokenCount = 0;
    Token bestToken;
    int startIndex = 0;
    int endIndex = 1;
    int bestStartIndex = 0;
    int bestEndIndex = 0;

    int formulaLen = strlen(formula);
    while (startIndex < formulaLen) {
        bestToken = T_NO_TOKEN;

        while (endIndex <= formulaLen) {
            int anyValid = 0;
            for (unsigned int i = 0; i < numTokenFinders; i++) {
                TokenFinder tokenFinder = tokenFinders[i];
                Validity validity = validateRange(formula, startIndex, endIndex, tokenFinder);
                if (validity == VALID) {
                    anyValid = 1;
                    bestToken = tokenFinder.token;
                    bestEndIndex = endIndex;
                    bestStartIndex = startIndex;
                    break;
                } else if (validity == PARTIAL) {
                    anyValid = 1;
                }
            }

            if (anyValid == 0 && bestToken != T_NO_TOKEN) {
                TokenInfo newToken;
                newToken.token = bestToken;
                newToken.validity = VALID;
                newToken.startIndex = startIndex;
                newToken.endIndex = bestEndIndex;
                newToken.raw = malloc(sizeof(char) * MAX_TOKEN_LENGTH);
                strncpy(newToken.raw, formula + startIndex, bestEndIndex - startIndex);
                result->tokens[result->tokenCount] = newToken;
                result->tokenCount++;
                startIndex = bestEndIndex - 1;
                endIndex = startIndex;
                break;
            }

            endIndex++;
        }

        startIndex++;
    }

    // Do one final check
    for (unsigned int i = 0; i < numTokenFinders; i++) {
        TokenFinder tokenFinder = tokenFinders[i];
        Validity validity = validateRange(formula, bestStartIndex, endIndex - 1, tokenFinder);
        if (validity == VALID) {
            TokenInfo newToken;
            newToken.token = tokenFinder.token;
            newToken.validity = VALID;
            newToken.startIndex = bestStartIndex;
            newToken.endIndex = endIndex - 1;
            newToken.raw = malloc(sizeof(char) * MAX_TOKEN_LENGTH);
	    strncpy(newToken.raw, formula + bestStartIndex, endIndex - bestStartIndex - 1);
            result->tokens[result->tokenCount] = newToken;
            result->tokenCount++;
        }
    }
}

// Parse

TokenInfo lookAhead(ParseInfo *info, int ahead) {
    BUG(assert(info->tokenIndex + ahead < info->tokenizeResult->tokenCount));
    return info->tokenizeResult->tokens[info->tokenIndex + ahead];
}

void error(ParseInfo *info) {
    info->didFail = 1;
    info->reachedEnd = 0;
}

void next(ParseInfo *info) {
    info->tokenIndex++;
    if (info->tokenIndex >= info->tokenizeResult->tokenCount) {
        info->reachedEnd = 1;
    }
}

void printRemain(ParseInfo *info) {
  printf("There are %d tokens left\n", info->tokenizeResult->tokenCount - info->tokenIndex);
  printf("The next token is %s\n", lookAhead(info, 0).raw);
}

void printToken(Token token) {
  switch (token) {
  case T_NO_TOKEN:
    printf("NO TOKEN");
    break;
  case T_NUMBER:
    printf("NUMBER");
    break;
  case T_MULT:
    printf("MULTIPLY");
    break;
  case T_NEG:
    printf("NEGATIVE");
    break;
  case T_PLUS:
    printf("ADD");
    break;
  case T_OPEN_PAREN:
    printf("OPEN PAREN");
    break;
  case T_CLOSE_PAREN:
    printf("CLOSE PAREN");
    break;
  case T_WHITESPACE:
    printf("WHITESPACE");
    break;
  case T_IDENT:
    printf("IDENTIFIER");
    break;
  case T_DIV:
    printf("DIVISION");
    break;
  default:
    printf("UNKNOWN TOKEN");
  }
}

int expectk(ParseInfo *info, Token token, int k) {
    TokenInfo currToken = lookAhead(info, k-1);
    return currToken.token == token;
}

int expect(ParseInfo *info, Token token) {
    return expectk(info, token, 1);
}

void consume(ParseInfo *info, Token token) {
    if (!expect(info, token)) {
        error(info);
        return;
    }
    next(info);
}

TokenInfo last(ParseInfo *info) {
    return lookAhead(info, -1);
}

// left recursive grammar
// ----
// program : list...
// list : ( elem... )

// elem : list
// elem : ident

// ident : func
// ident : number
// ident : string
// ident : cellrange

List* list(ParseInfo *info) {
    List* result = malloc(sizeof(List));

    consume(info, T_OPEN_PAREN);
    if (info->didFail) return 0;

    // Function token is just for evalling

    // TODO: Don't hard code max elem count.
    // Instead, the List structure should probably be stored
    // as a linked list
    Elem** elems = malloc(sizeof(Elem*) * 128);
    unsigned int argc = 0;
    while (!expect(info, T_CLOSE_PAREN) && !info->didFail) {
        // Obviously a bad way to do this
        if (argc > 20) {
            info->didFail = 1;
            return 0;
        }

        Elem* currElem = elem(info);
        elems[argc] = currElem;
        argc++;
    }

    consume(info, T_CLOSE_PAREN);

    if (expect(info, T_WHITESPACE)) {
        consume(info, T_WHITESPACE);
    }

    result->elems = elems;
    result->elemCount = argc;

    return result;
}

Elem* elem(ParseInfo *info) {
    Elem* result = malloc(sizeof(Elem));

    if (expect(info, T_OPEN_PAREN)) {
        List* listResult = list(info);
        result->type = E_LIST;
        result->val.list = listResult;
        return result;
    }

    if (expect(info, T_WHITESPACE)) {
        consume(info, T_WHITESPACE);
    }

    if (expect(info, T_NUMBER)) {
        TokenInfo currToken = lookAhead(info, 0);
        double a = ctod(currToken.raw);
        consume(info, T_NUMBER);

        if (expect(info, T_WHITESPACE)) {
            consume(info, T_WHITESPACE);
        }

        result->type = E_IDENT;
        result->val.ident = (Ident) {
            .type = I_NUM,
            .val = { .num = a }
        };
        return result;
    }

    if (expect(info, T_IDENT)) {
        TokenInfo currToken = lookAhead(info, 0);
        consume(info, T_IDENT);

        if (expect(info, T_WHITESPACE)) {
            consume(info, T_WHITESPACE);
        }

        result->type = E_IDENT;
        result->val.ident = (Ident) {
            .type = I_VAR,
            .val = { .name = currToken.raw }
        };
        return result;
    }


    return 0;
}

// A program is simply a list of Lists
Program* program(ParseInfo* info) {
  Program* result = malloc(sizeof(Program));
  List** lists = malloc(sizeof(List*) * 1024); // TODO Don't hard code
  unsigned int listCount = 0;

  while (!info->reachedEnd) {
    while (expect(info, T_WHITESPACE)) {
      consume(info, T_WHITESPACE);
    }

    List* resultList = list(info);

    if (!resultList && !info->reachedEnd) {
      info->didFail = 1;
      return 0;
    }

    lists[listCount] = resultList;
    listCount++;

    while (expect(info, T_WHITESPACE)) {
      consume(info, T_WHITESPACE);
    }
  }


  result->lists = lists;
  result->listCount = listCount;
  return result;
}

char* elemIdentName(Elem* elem) {
  // TODO Error handling when trying to get ident name from
  // elem with wrong type.
  return elem->val.ident.val.name;
}

List* elemList(Elem* elem) {
  return elem->val.list;
}
