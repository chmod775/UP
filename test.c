#include <stdlib.h>
#include <stdio.h>

void main() {
  int abc(int a, int b) {
    return a + b;
  }

  uint64_t i = 0;
  uint64_t a = 775;
  while (i < 8000000) {
    i++;
    a++;
  }
    printf("%lu", a);
}