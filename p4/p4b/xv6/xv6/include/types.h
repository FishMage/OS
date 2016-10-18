#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

struct stat;
typedef struct _lock_t{
    unsigned int lock;
}lock_t;
typedef struct _thread{
  uint thread_status; // 0 for proc, 1 for thread
  void* stack_loc; //location of thread stack
  lock_t *lock;
  }thread_t;
#ifndef NULL
#define NULL (0)
#endif

#endif //_TYPES_H_
