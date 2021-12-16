#include <stdio.h>
#include <stdlib.h>

struct station {
  char name[20];
  int birthyear;
  struct station *next;
};

int main() {
  int *ptr = (int*) malloc(10 * sizeof(int));
  if(ptr == NULL) exit(1);

  printf("%p\n", ptr);

  for(int i = 0; i < 10; i++) {
    ptr[i] = i * i;
  }
  free(ptr);

  ptr = (int*)malloc(10 * sizeof(int));
  if(ptr == NULL) exit(1);

  printf("%p\n", ptr);

  for(int i = 0; i < 10; i++) {
    printf("%d %d\n", i, ptr[i]);
  }
}