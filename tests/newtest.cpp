#include <cstdio>
#include <cstdlib>
#include <new>

int main()
{
  // Test 1
  struct C
  {
    int x;
    int y;
  };

  C *c = new C();
  c->x = 4;
  c->y = 14;

  printf("x: %d\n", c->y);
  delete c;

  // Test 2
  int n = 5;
  int *a = new int[n];
  for (int i=0; i < n; i++)
  {
    a[i] = i;
  }
  delete[] a;

  // Test 3
  char *b = new char[0];
  if (!b) exit(-1);

  return 0;
}
