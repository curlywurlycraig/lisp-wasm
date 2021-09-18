#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <parse.h>
#include <math.h>

TokenFinder* tokenFinders;
unsigned int numTokenFinders;

CharState startState;
CharState endState;

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
    CharState numberState;
    numberState.id = 0;
    numberState.type = DIGIT;

    CharState periodState;
    periodState.id = 1;
    periodState.type = PERIOD;

    CharState fractionState;
    fractionState.id = 2;
    fractionState.type = DIGIT;

    CharState hyphenState;
    hyphenState.id = 3;
    hyphenState.type = HYPHEN;

    // number is
    // 1
    // 1018585
    // 12383.1583
    // -12383.1583
    // NOT 123415. (don't allow trailing period)
    // <some digits><maybe period & more digits>
    TokenFinder numberFinder;
    numberFinder.token = T_NUMBER;
    numberFinder.transitionCount = 9;
    numberFinder.transitions = malloc(sizeof(StateTransition) * numberFinder.transitionCount);

    numberFinder.transitions[0] = (StateTransition) {
        .fromState = startState,
        .toState = numberState
    };
    numberFinder.transitions[1] = (StateTransition) {
        .fromState = numberState,
        .toState = endState
    };
    numberFinder.transitions[2] = (StateTransition) {
        .fromState = numberState,
        .toState = periodState
    };
    numberFinder.transitions[3] = (StateTransition) {
        .fromState = numberState,
        .toState = numberState
    };
    numberFinder.transitions[4] = (StateTransition) {
        .fromState = periodState,
        .toState = fractionState
    };
    numberFinder.transitions[5] = (StateTransition) {
        .fromState = fractionState,
        .toState = endState
    };
    numberFinder.transitions[6] = (StateTransition) {
        .fromState = fractionState,
        .toState = fractionState
    };

    // Neg numbers
    numberFinder.transitions[7] = (StateTransition) {
        .fromState = startState,
        .toState = hyphenState
    };
    numberFinder.transitions[8] = (StateTransition) {
        .fromState = hyphenState,
        .toState = numberState
    };

    return numberFinder;
}

TokenFinder makeSingleCharacterFinder(CharType type, Token token) {
    CharState charState;
    charState.id = 0;
    charState.type = type;

    TokenFinder charFinder;
    charFinder.token = token;
    charFinder.transitionCount = 2;
    charFinder.transitions = malloc(sizeof(StateTransition) * charFinder.transitionCount);

    charFinder.transitions[0] = (StateTransition) {
        .fromState = startState,
        .toState = charState
    };
    charFinder.transitions[1] = (StateTransition) {
        .fromState = charState,
        .toState = endState
    };

    return charFinder;
}

TokenFinder makeRepeatingCharacterFinder(CharType type, Token token) {
    CharState charState;
    charState.id = 0;
    charState.type = type;

    TokenFinder charFinder;
    charFinder.token = token;
    charFinder.transitionCount = 3;
    charFinder.transitions = malloc(sizeof(StateTransition) * charFinder.transitionCount);

    charFinder.transitions[0] = (StateTransition) {
        .fromState = startState,
        .toState = charState
    };
    charFinder.transitions[1] = (StateTransition) {
        .fromState = charState,
        .toState = charState
    };
    charFinder.transitions[2] = (StateTransition) {
        .fromState = charState,
        .toState = endState
    };

    return charFinder;
}

TokenFinder makeIdentifierFinder() {
    CharState charState;
    charState.id = 0;
    charState.type = LETTER;

    CharState hyphenState;
    hyphenState.id = 1;
    hyphenState.type = HYPHEN;

    CharState periodState;
    periodState.id = 2;
    periodState.type = PERIOD;

    CharState numberState;
    numberState.id = 3;
    numberState.type = DIGIT;

    TokenFinder identFinder;
    identFinder.token = T_IDENT;
    identFinder.transitionCount = 14;
    identFinder.transitions = malloc(sizeof(StateTransition) * identFinder.transitionCount);

    identFinder.transitions[0] = (StateTransition) {
        .fromState = startState,
        .toState = charState
    };
    identFinder.transitions[1] = (StateTransition) {
        .fromState = startState,
        .toState = hyphenState
    };
    identFinder.transitions[2] = (StateTransition) {
        .fromState = charState,
        .toState = charState
    };
    identFinder.transitions[3] = (StateTransition) {
        .fromState = charState,
        .toState = numberState
    };
    identFinder.transitions[4] = (StateTransition) {
        .fromState = numberState,
        .toState = periodState
    };
    identFinder.transitions[5] = (StateTransition) {
        .fromState = numberState,
        .toState = hyphenState
    };
    identFinder.transitions[6] = (StateTransition) {
        .fromState = numberState,
        .toState = numberState
    };
    identFinder.transitions[7] = (StateTransition) {
        .fromState = charState,
        .toState = periodState
    };
    identFinder.transitions[8] = (StateTransition) {
        .fromState = periodState,
        .toState = charState
    };
    identFinder.transitions[9] = (StateTransition) {
        .fromState = charState,
        .toState = hyphenState 
    };
    identFinder.transitions[10] = (StateTransition) {
        .fromState = charState,
        .toState = endState
    };
    identFinder.transitions[11] = (StateTransition) {
        .fromState = hyphenState,
        .toState = charState
    };
    identFinder.transitions[12] = (StateTransition) {
        .fromState = hyphenState,
        .toState = endState
    };
    identFinder.transitions[13] = (StateTransition) {
        .fromState = numberState,
        .toState = endState
    };

    return identFinder;
}

TokenFinder makeOpenParenFinder() {
    return makeSingleCharacterFinder(OPEN_PAREN, T_OPEN_PAREN);
}

TokenFinder makeCloseParenFinder() {
    return makeSingleCharacterFinder(CLOSE_PAREN, T_CLOSE_PAREN);
}

TokenFinder makeWhitespaceFinder() {
    return makeRepeatingCharacterFinder(SPACE, T_WHITESPACE);
}

void initTokenFinders() {
    startState.id = -1;
    startState.type = START;

    endState.id = -2;
    endState.type = END;

    numTokenFinders = 0;
    tokenFinders = malloc(sizeof(TokenFinder) * MAX_FINDERS);

    tokenFinders[0] = makeNumberFinder();
    numTokenFinders++;
    tokenFinders[1] = makeWhitespaceFinder();
    numTokenFinders++;
    tokenFinders[2] = makeOpenParenFinder();
    numTokenFinders++;
    tokenFinders[3] = makeCloseParenFinder();
    numTokenFinders++;
    tokenFinders[4] = makeIdentifierFinder();
    numTokenFinders++;
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
