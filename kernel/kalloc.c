// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define NSUPER 16   // number of 2MB super pages we keep; adjust as needed

static struct {
  struct spinlock lock;
  void *list[NSUPER];
  int n;
} superarea;

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
// carve out NSUPER superpages if available
initlock(&superarea.lock, "superarea");
superarea.n = 0;
for(int i = 0; i < NSUPER; i++){
  // try to allocate SUPERPGSIZE bytes as contiguous physical chunk by
  // allocating SUPERPGSIZE/PGSIZE pages and checking alignment.
  // Simplest pragmatic approach:
  char *p = kalloc();
  if(!p) break;
  // try to coalesce: allocate SUPERPGSIZE/PGSIZE - 1 more pages and hope it's contiguous
  // but that is unreliable. Better approach: Require boot-time supply of superpage memory,
  // OR allocate one contiguous chunk by using pages from end of RAM via a build-time constant.
  // For simplicity in labs: attempt to allocate KVA aligned to SUPERPGSIZE by scanning kalloc
  // results is complicated — instead use this simpler pool: allocate SUPERPGSIZE using kalloc multiple times and store starting address only if (uint64)p % SUPERPGSIZE == 0
  if(((uint64)p & SUPERPGMASK) == 0){
    superarea.list[superarea.n++] = p;
  } else {
    kfree(p); // discard non-aligned KVA
    break; // stop if not aligned
  }
}
}
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
void *
superalloc(void)
{
  void *k;
  acquire(&superarea.lock);
  if(superarea.n == 0){
    k = 0;
  } else {
    k = superarea.list[--superarea.n];
    superarea.list[superarea.n] = 0;
  }
  release(&superarea.lock);
  return k;
}

void
superfree(void *k)
{
  acquire(&superarea.lock);
  if(superarea.n < NSUPER){
    superarea.list[superarea.n++] = k;
    release(&superarea.lock);
    return;
  }
  release(&superarea.lock);
  // fallback: free 2MB as 4K pages
  char *p = (char*)k;
  for(int i = 0; i < SUPERPGSIZE/PGSIZE; i++){
    kfree(p + i*PGSIZE);
  }
}

