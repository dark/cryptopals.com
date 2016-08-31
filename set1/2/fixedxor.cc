#include <stdio.h>
#include <stdlib.h>

int char2int(unsigned char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  fprintf(stderr, "%s: unexpected char %d\n", __FUNCTION__, c);
  exit(1);
}

void fixed_xor_and_print(unsigned char left, unsigned char right) {
  int i1 = char2int(left);
  int i2 = char2int(right);

  printf("%x", i1 ^ i2);
}

void fixed_xor_loop(FILE *fp1, FILE *fp2) {
  int left, right;

  while (true) {
    left = fgetc(fp1);
    right = fgetc(fp2);

    if (left == EOF && right == EOF)
      // all done
      return;

    if (left == EOF || right == EOF) {
      fprintf(stderr, "\nOne of the two inputs is longer\n");
      return;
    }

    fixed_xor_and_print(left, right);
  }
}

int main (int argc, char *argv[]) {
  FILE *fp1, *fp2;

  if (argc != 3) {
    fprintf(stderr, "Need 2 arguments, got %d instead\n", argc - 1);
    return 1;
  }

  fp1 = fopen(argv[1], "r");
  if (!fp1) {
    fprintf(stderr, "Cannot open first file: %s\n", argv[1]);
    return 1;
  }

  fp2 = fopen(argv[2], "r");
  if (!fp2) {
    fprintf(stderr, "Cannot open second file: %s\n", argv[2]);
    return 1;
  }

  fixed_xor_loop(fp1, fp2);

  fclose(fp1);
  fclose(fp2);

  return 0;
}
