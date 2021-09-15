#ifndef __PARSE_H__
#define __PARSE_H__

#define MAX_TOKENS 256
#define MAX_FINDERS 16
#define MAX_TOKEN_LENGTH 256

typedef enum CharType {
    START,
    DIGIT,
    HYPHEN,
    PERIOD,
    ASTERISK,
    PLUS,
    SPACE,
    OPEN_PAREN,
    CLOSE_PAREN,
    SLASH,
    END,
    LETTER,
    COLON,
    UNKNOWN
} CharType;

typedef enum Validity {
    INVALID,
    PARTIAL,
    VALID
} Validity;

typedef enum Token {
    T_NO_TOKEN,
    T_NUMBER,
    T_MULT,
    T_NEG,
    T_PLUS,
    T_OPEN_PAREN,
    T_CLOSE_PAREN,
    T_WHITESPACE,
    T_IDENT,
    T_DIV,
    T_CELLREF
} Token;

typedef struct CharState {
    int id;
    CharType type;
} CharState;

typedef struct StateTransition {
    CharState fromState;
    CharState toState;
} StateTransition;

typedef struct TokenFinder {
    StateTransition *transitions;
    unsigned int transitionCount;
    Token token;
} TokenFinder;

CharType getCharType(char input);

Validity validateRange(char* range, int startIndex, int endIndex, TokenFinder finder);

TokenFinder makeCellrefFinder();

TokenFinder makeNumberFinder();

TokenFinder makeSingleCharacterFinder(CharType type, Token token);

TokenFinder makeRepeatingCharacterFinder(CharType type, Token token);

TokenFinder makeIdentifierFinder();

TokenFinder makeOpenParenFinder();

TokenFinder makeCloseParenFinder();

TokenFinder makeWhitespaceFinder();

void initTokenFinders();

typedef struct TokenInfo {
    Validity validity;
    Token token;
    char* raw;
    int startIndex;
    int endIndex;
} TokenInfo;

typedef struct TokenizeResult {
    TokenInfo *tokens;
    int tokenCount;
} TokenizeResult;

void tokenize(TokenizeResult* result, char* formula);

// Parse

typedef struct ParseInfo {
    int tokenIndex;
    int didFail;
    int reachedEnd;
    TokenizeResult *tokenizeResult;
    char* raw;
} ParseInfo;

TokenInfo lookAhead(ParseInfo *info, int ahead);

void error(ParseInfo *info);

void next(ParseInfo *info);

int expectk(ParseInfo *info, Token token, int k);

int expect(ParseInfo *info, Token token);

void consume(ParseInfo *info, Token token);

TokenInfo last(ParseInfo *info);

// left recursive grammar
// ----
// list : ( elem... )

// elem : list
// elem : ident

// ident : func
// ident : number
// ident : string
// ident : cellrange

enum IdentType {
    I_VAR,
    I_NUM,
    I_STR
};

typedef struct Ident {
    enum IdentType type;
    int didFail;
    union {
        char* name;
        double num;
    } val;
} Ident;

enum ElemType {
    E_LIST,
    E_IDENT
};

typedef struct List List;

struct List;

typedef struct Elem {
    enum ElemType type;
    int didFail;
    union {
        List* list;
        Ident ident;
    } val;
} Elem;

typedef struct List {
    int didFail;
    Elem** elems;
    int elemCount;
} List;

char* elemIdentName(Elem* elem);

List* elemList(Elem* elem);

List* list(ParseInfo *info);

Elem* elem(ParseInfo *info);

#endif
