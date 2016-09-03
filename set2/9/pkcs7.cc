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

std::string pad(const std::string &buf, const size_t blksize) {
  const size_t bufsize = buf.size();

  if (bufsize > blksize) {
    fprintf(stderr, "%s: BUG, %ld > %ld\n", __FUNCTION__, bufsize, blksize);
    exit(1);
  }

  if (bufsize == blksize)
    // already aligned
    return buf;

  // pad and return
  std::string retval(buf);
  retval.append(blksize - bufsize, blksize - bufsize);
  return retval;
}

int main (void) {
  // read input
  std::string buf;
  read_buffer(&buf);
  fprintf(stderr, "Got %ld bytes of input: [%.*s]\n", buf.size(), (int)buf.size(), buf.c_str());

  std::string padded = pad(buf, 20);
  fprintf(stderr, "Padded to %ld bytes: [%.*s]\n", padded.size(), (int)padded.size(), padded.c_str());

  return 0;
}
