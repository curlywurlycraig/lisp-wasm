#include "tokenfinder.h"

#include <lwasm.h>
#include <parse.h>

#define charState   { .id = 0, .type = LETTER }
#define hyphenState { .id = 1, .type = HYPHEN }
#define periodState { .id = 2, .type = PERIOD }
#define numberState { .id = 2, .type = DIGIT  }

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

TokenFinder identifierTokenFinder = {
  .token = T_IDENT,
  .transitionCount = ARRAY_LENGTH(transitions),
  .transitions = transitions,
};
