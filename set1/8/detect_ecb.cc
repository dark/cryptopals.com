#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "repkey_xor.h"

int char2int(int c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  fprintf(stderr, "%s: unexpected char %d\n", __FUNCTION__, c);
  exit(1);
}

int hex2bin(unsigned char x1, unsigned char x2) {
  int hi = char2int(x1);
  int lo = char2int(x2);

  return (((hi & 0x0f) << 4) | (lo & 0x0f));
}

std::string read_one_line() {
  char *line = NULL;
  size_t length = 0;

  ssize_t retval = getline(&line, &length, stdin);
  if (retval == -1) {
    // EOF
    return "";
  }

  // remove newline
  if (line[retval-1] == '\n') {
    line[retval-1] = '\0';
    --retval;
  }

  // hex 2 binary
  std::string s;
  s.reserve(length / 2);
  for (ssize_t i = 0; i < retval; i += 2) {
    if ((i + 1) == retval) {
      fprintf(stderr, "%s: buffer with odd number of chars\n", __FUNCTION__);
      exit(1);
    }

    s.append(1, hex2bin(line[i], line[i+1]));
  }

  free(line);

  return s;
}

int main (void) {
  while (true) {
    std::string buf = read_one_line();
    if (buf.empty())
      // EOF
      break;

    fprintf(stderr, "ciphertext is %ld bytes long\n", buf.size());
  }

  return 0;
}
