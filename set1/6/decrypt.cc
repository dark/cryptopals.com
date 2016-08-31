#include <string>
#include "base64.h"

int get_one_v2(const bool eof_is_error) {
  int c = getchar();
  if (c == EOF) {
    if (eof_is_error) {
      fprintf(stderr, "Unexpected EOF\n");
      exit(1);
    } else {
      return -1;
    }
  }
  return c;
}

void read_buffer(std::string *buf) {
  int c;

  while (true) {
    c = get_one_v2(false);
    if (c == -1)
      // EOF
      break;

    // printf("%d ", c);

    buf->append(1, c);
  }
}

int count_bits_set(const unsigned char &c) {
  return __builtin_popcount(c);
}

int hamming_distance(const std::string &a, const std::string &b) {
  if (a.size() != b.size())
    // cannot compute distance
    return -1;

  int distance = 0;
  size_t cursor = 0;
  for (; cursor < a.size(); ++cursor) {
    distance += count_bits_set(a[cursor] ^ b[cursor]);
  }

  return distance;
}

int main(int argc, char *argv[]) {
  // test basic preconditions
  const int test_distance = hamming_distance("this is a test", "wokka wokka!!!");
  if (test_distance != 37) {
    fprintf(stderr, "Validation failed: distance should be 37, but it is %d instead\n", test_distance);
    return 1;
  }

  // read input
  std::string buf;
  read_buffer(&buf);
  fprintf(stderr, "Got %ld bytes of input\n", buf.size());

  // decode base64
  std::string decoded_input;
  int result = decodebase64(&decoded_input, buf);
  if (result < 0) {
    fprintf(stderr, "Bad base64 input\n");
    return 1;
  }
  fprintf(stderr, "Decoded %ld bytes of input\n", decoded_input.size());

  return 0;
}
