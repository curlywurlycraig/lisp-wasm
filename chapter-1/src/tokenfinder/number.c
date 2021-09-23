#include <parse.h>
#include <lwasm.h>

#include "tokenfinder.h"

#define numberState   { .id = 0, .type = DIGIT  }
#define periodState   { .id = 1, .type = PERIOD }
#define fractionState { .id = 2, .type = DIGIT  }
#define hyphenState   { .id = 3, .type = HYPHEN }

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

TokenFinder numberTokenFinder = {
  .token = T_NUMBER,
  .transitionCount = ARRAY_LENGTH(transitions),
  .transitions = transitions,
};
