#include <cstdio>
#include <iostream>
using namespace std;

int main() {
  int a = 0;
  int b[] = {100, 80, 50, 90, 66, 33, 11};
  while (1) {
    for (int i =  0; i < 7; i++) {
      printf("%d ", b[i]);
    }
    printf("\n");

    printf("%d\t", a);
    printf("a = 0x%x\n", a);
    int temp;
    scanf("%d", &temp);
    if (temp == 0) {
      continue;
    } else {
      a = temp;
    }
    printf("%d\t", a);
    printf("a changed 0x%x\n", a);
  }
  return 0;
}
