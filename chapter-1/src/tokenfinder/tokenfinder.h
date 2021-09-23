#ifndef __TOKEN_FINDER_H_
#define __TOKEN_FINDER_H_

/*
 * Constants used to define TokenFinders.
 */

#define startStateId (-2)
#define endStateId   (-2)
#define startState   { .id = startStateId, .type = START }
#define endState     { .id = endStateId,   .type = END   }

#endif
