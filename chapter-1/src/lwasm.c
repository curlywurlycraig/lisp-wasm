#include <lwasm.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE         4096
#define OVERFLOW_THRESHOLD 256

#define POWER_OF_TWO(v) ((v & (v - 1)) == 0)

static void* check(void* ptr)
{
  if (!ptr) {
    abort();
  }

  return ptr;
}

struct lwasm_block {
  struct lwasm_block* next;
  uint8_t* end;
  uint8_t mem[];
};

struct lwasm_allocator {
  struct lwasm_block* block;
  struct lwasm_big_alloc {
    struct lwasm_big_alloc* next;
    void* mem;
  }* overflow;
};

static_assert(sizeof(struct lwasm_block) + sizeof(struct lwasm_big_alloc) < BLOCK_SIZE,
	      "BLOCK_SIZE must be able to container a minimal headers");

static inline struct lwasm_block* new_block()
{
  struct lwasm_block* ret = check(calloc(1, BLOCK_SIZE));
  ret->end = ret->mem;
  return ret;
}

static inline uint8_t* alignup(uint8_t* p, size_t align)
{
  assert(POWER_OF_TWO(align));
  uintptr_t ptr = (uintptr_t)(void*)p;
  return (uint8_t*)((ptr + align - 1) & ~(align - 1));
}

lwasm_allocator* lwasm_allocator_new()
{
    struct lwasm_allocator* ret = check(calloc(1, sizeof(*ret)));
    ret->block = new_block();
    return ret;
}

void lwasm_allocator_delete(lwasm_allocator* a)
{
    // First free all of the overflows
    for (struct lwasm_big_alloc* o = a->overflow; o != NULL; o = o->next) {
      free(o->mem);
    }

    // Now free all the blocks, taking care to read the next pointer before
    // freeing.
    struct lwasm_block* block = a->block;

    while (block) {
      void* mem = block;
      block = block->next;
      free(mem);
    }

    // Clear the allocator structure so it can't accidently be used.
    *a = (lwasm_allocator){0};
}

void* lwasm_allocate(lwasm_allocator* a, size_t align, size_t size)
{
  assert(a->block);  // Allocator isn't deinitialized

  if (size > OVERFLOW_THRESHOLD) {
    struct lwasm_big_alloc* o = ALLOCATE(a, struct lwasm_big_alloc);
    o->next = a->overflow;
    o->mem = check(aligned_alloc(align, size));
    memset(o->mem, 0, size);
    a->overflow = o;
  }

  uint8_t* base = alignup(a->block->end, align);
  ptrdiff_t height = (base + size) - (uint8_t*)a->block;

  if (height < BLOCK_SIZE) {
    a->block->end = base + size;
    return base;
  } else {
    a->block = new_block();
    return lwasm_allocate(a, align, size);
  }
}
