#include <stdio.h>
int main() {
  int a = 0;
  while(1) {
    printf("%d\t",a);
    printf("a = 0x%x\n",a);
    int temp;
    scanf("%d",&temp);
    if(temp == 0) {
      continue;
    }
    else {
      a = temp;
    }
    printf("%d\t",a);
    printf("a changed 0x%x\n",a);
  }
  return 0;
}
