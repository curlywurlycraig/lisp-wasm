#ifndef __LWASM_H_
#define __LWASM_H_

#include <stdalign.h>
#include <stddef.h>
#include <assert.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

#define BUG(assertion)  // used to temporarily disable an assert that shouldn't fail

#define ALLOCATE(allocator, ty)					        \
    ((ty*)lwasm_allocate((allocator), alignof(ty), sizeof(ty)))

#define ALLOCATE_ARRAY(allocator, ty, count)				\
    ((ty*)lwasm_allocate((allocator), alignof(ty), (count)* sizeof(ty)))

typedef struct lwasm_allocator lwasm_allocator;

lwasm_allocator* lwasm_allocator_new();
void lwasm_allocator_delete(lwasm_allocator* a);
void* lwasm_allocate(lwasm_allocator* a, size_t align, size_t size);

#endif
