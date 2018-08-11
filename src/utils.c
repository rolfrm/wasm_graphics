#include <stdlib.h>
#include "utils.h"

typedef int (*q2cmp)(const void *, const void*, void * arg);

typedef struct {
  q2cmp cmp;
  void * arg;
}qsort2_data;

static __thread qsort2_data qsort2_arg;


int qsort2_cmp(const void * a, const void * b){
  return qsort2_arg.cmp(a,b,qsort2_arg.arg);
}

void qsort2(void * base, size_t nmemb, size_t size, int (*compare)(const void *, const void*, void * arg), void * arg){
  qsort2_arg.cmp = compare;
  qsort2_arg.arg = arg;
  qsort(base, nmemb, size, qsort2_cmp); 
}

