#include "base64.h"

int base64_fetch4_(const std::string &s, size_t *buf_cursor,
                   unsigned char result[4]) {
  const size_t buf_size = s.size();

  // fetch 4 bytes at a time
  int fetched = 0;
  while (fetched < 4) {
    if (*buf_cursor >= buf_size) {
      if (fetched)
        // print warning only if we have incomplete data (otherwise it's legal EOF)
        fprintf(stderr, "%s: we needed data at cursor[%ld] fetched[%d], but no more data available\n",
                __FUNCTION__, *buf_cursor, fetched);
      return fetched;
    }

    unsigned char c = s[*buf_cursor];
    ++(*buf_cursor);
    if (c == '\n') {
      // silently discard this char
    } else {
      result[fetched++] = c;
    }
  }

  // success
  return fetched;
}

int base64_decode_indices_(unsigned char indices[4], const unsigned char input[4]) {
  for (int i = 0; i < 4; ++i) {
    const unsigned char in = input[i];
    unsigned char out;

    if (in >= 'A' && in <= 'Z') {
      out = in - 'A';
    } else if (in >= 'a' && in <= 'z') {
      out = in - 'a' + 26;
    } else if (in >= '0' && in <= '9') {
      out = in - '0' + 52;
    } else if (in == '+') {
      out = 62;
    } else if (in == '/') {
      out = 63;
    } else {
      fprintf(stderr, "%s: bad char [%c](%d) while decoding\n",
              __FUNCTION__, in, in);
      return -1;
    }

    indices[i] = out;
  }
  return 0;
}

int base64_decode4_(std::string *result, unsigned char input[4]) {
  size_t result_length = 3;
  // handle padding
  if (input[3] == '=') {
    input[3] = 'A'; // decay
    if (input[2] == '=') {
      input[2] = 'A'; // decay
      result_length = 1;
    } else {
      result_length = 2;
    }
  }

  unsigned char indices[4];
  int status = base64_decode_indices_(indices, input);
  if (status < 0) {
    // bad decode
    return -1;
  }

  // build the final result
  std::string buf(3, '\0');
  buf[0] = ((indices[0] << 2) & 0xfc) | ((indices[1] >> 4) & 0x3);
  buf[1] = ((indices[1] << 4) & 0xf0) | ((indices[2] >> 2) & 0x0f);
  buf[2] = ((indices[2] << 6) & 0xc0) | (indices[3] & 0x3f);

  // append the right amount of bytes to the result
  result->append(buf, 0, result_length);

  return 0;
}

int decodebase64(std::string *decoded, const std::string &s) {
  // reserve some space for the result (approximate)
  decoded->reserve(s.size() * 3 / 4);

  size_t buf_cursor = 0;
  // fetch 4 bytes at a time
  unsigned char chars[4];
  while (true) {
    int result = base64_fetch4_(s, &buf_cursor, chars);
    if (result == 0) {
      // done decoding
      break;
    } else if (result != 4) {
      // bad data
      return -1;
    }

    // decode
    result = base64_decode4_(decoded, chars);
    if (result < 0)
      // bad decode
      return -1;
  }

  return 0;
}
