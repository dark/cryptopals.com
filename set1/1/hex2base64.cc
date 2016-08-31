#include <stdio.h>
#include <stdlib.h>

unsigned char get_one(const bool eof_is_error) {
  int c = getchar();
  if (c == EOF) {
    if (eof_is_error) {
      fprintf(stderr, "Unexpected EOF\n");
      exit(1);
    } else {
      exit(0);
    }
  }
  return c;
}

int char2int(unsigned char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  fprintf(stderr, "%s: unexpected char %d\n", __FUNCTION__, c);
  exit(1);
}

unsigned char get_one_hex(const bool eof_is_error) {
  unsigned char x1 = get_one(eof_is_error);
  unsigned char x2 = get_one(true);

  int hi = char2int(x1);
  int lo = char2int(x2);

  return (((hi & 0x0f) << 4) | (lo & 0x0f));
}

void printbase64(unsigned char c) {
  if (c <= 25)
    fprintf(stdout, "%c", c + 'A');
  else if (c <= 51)
    fprintf(stdout, "%c", c - 26 + 'a');
  else if (c <= 61)
    fprintf(stdout, "%c", c - 52 + '0');
  else if (c == 62)
    fprintf(stdout, "+");
  else if (c == 63)
    fprintf(stdout, "/");
  else
    fprintf(stderr, "%s: unexpected char %d\n", __FUNCTION__, c);
}

void hex2base64(unsigned char c1, unsigned char c2, unsigned char c3) {
  printbase64(c1 >> 2);
  printbase64(((c1 & 0x03) << 4) | (c2 >> 4));
  printbase64(((c2 & 0x0f) << 2) | (c3 >> 6));
  printbase64(c3 & 0x3f);
}

int main (void) {
  unsigned char c1, c2, c3;

  while (true) {
    c1 = get_one_hex(false);
    c2 = get_one_hex(true);
    c3 = get_one_hex(true);

    hex2base64(c1, c2, c3);
  }

  return 0;
}
