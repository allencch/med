#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

int main() {
  int a = 0;
  int b[] = {100, 80, 50, 90, 66, 33, 11};
  int* c = new int[4];
  memset(c, 0, 4 * sizeof(int));
  c[0] = 101;
  c[1] = 81;
  c[2] = 51;
  c[3] = 91;
  while (1) {
    printf("%p\n", b);
    for (int i =  0; i < 7; i++) {
      printf("%d ", b[i]);
    }
    printf("\n");

    uint8_t* ptr = (uint8_t*)c;
    printf("%p\n", ptr);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        printf("%x ", (unsigned int)*(ptr + (i * 4 + j)));
      }
      printf("\n");
    }
    printf("\n");

    printf("%d\t", a);
    printf("a = 0x%x\n", (int)a);
    int temp;
    scanf("%d", &temp);
    if (temp == 0) {
      continue;
    } else {
      a = temp;
    }
    printf("%d\t", a);
    printf("a changed 0x%x\n", (int)a);
  }

  delete[] c;
  return 0;
}
