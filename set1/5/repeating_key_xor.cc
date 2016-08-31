#include <stdio.h>
#include <stdlib.h>
#include <string>

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

void print_hex(const std::string &s) {
  for (const unsigned char c: s) {
    printf("%x%x", c >> 4, c & 0x0f);
  }
  printf("\n");
}

std::string repkey_xor(const std::string &key, const std::string &s) {
  std::string result;
  result.reserve(s.size());

  size_t buf_cursor = 0;
  const size_t buf_size = s.size();

  size_t key_cursor = 0;
  const size_t key_size = key.size();
  for (; buf_cursor < buf_size; ++buf_cursor) {
    result.append(1, s[buf_cursor] ^ key[key_cursor]);

    // advance key cursor
    key_cursor = (key_cursor + 1) % key_size;
  }

  return std::move(result);
}

int main(int argc, char *argv[]) {
  std::string key, buf;

  if (argc != 2) {
    fprintf(stderr, "Need 1 argument, got %d instead\n", argc - 1);
    return 1;
  }
  key = std::string(argv[1]);
  if (key.empty()) {
    fprintf(stderr, "Argument #1 is empty\n");
    return 1;
  }
  fprintf(stderr, "Using key: [%s]\n", key.c_str());

  read_buffer(&buf);

  std::string xored = repkey_xor(key, buf);

  print_hex(xored);

  return 0;
}
