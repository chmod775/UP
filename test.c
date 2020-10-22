#include <stdlib.h>
#include <stdio.h>

void main() {
  int abc(int a, int b) {
    return a + b;
  }

  u_int64_t i = 3;
  while (i < 10000000)
    i++;
  printf("%d", i);

  printf("%d", abc(5,6));
}