#include <string.h>
#include <wchar.h>
#include <stdio.h>

int main(void) {
  int **ipp = 0;
  int *ip[10];
  int x = 1;
  int y = 0;
  float z = 0.5;
  int *** ippp = 0;
  ++((new float***[1])[0]);
  printf("%lld %d %d %d %d %f", ipp, ip[0], ip[9], x, y, z);
  return 0;
}
