#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

bool is_valid(int c) {
  return isprint(c) || isspace(c);
}

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

int char2int(int c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  fprintf(stderr, "%s: unexpected char %d\n", __FUNCTION__, c);
  exit(1);
}

int get_one_hex_v2() {
  int x1 = get_one_v2(false);
  if (x1 == -1)
    // EOF reached
    return -1;
  int x2 = get_one_v2(true);

  int hi = char2int(x1);
  int lo = char2int(x2);

  return (((hi & 0x0f) << 4) | (lo & 0x0f));
}

void read_buffer(std::string *buf) {
  int c;

  while (true) {
    c = get_one_hex_v2();
    if (c == -1)
      // EOF
      break;

    //printf("%d ", c);

    buf->append(1, c);
  }
}

int compute_frequencies(const std::string &s, std::vector<int> *frequencies) {
  bool has_nonprint = false;

  for (const unsigned char c: s) {
    ++(*frequencies)[c];
    if (!is_valid(c))
      has_nonprint = true;
  }

  if (has_nonprint)
    // score is 0 if non-print chars are present
    return 0;

  // try and compute a reasonable score; give 1 base point to all
  // chars, then 1 additional point for all letters or spaces, and 1
  // other additional point for the 5 more common letters in english
  // text (e, t, a, o, i)
  int score = s.size(); // 1 base point
  // ascii is 7 bits
  for (unsigned char c = 0; c < 0x7f; ++c) {
    if (!isalpha(c) && c != ' ')
      continue;

    switch (tolower(c)) {
      case 'e':
      case 't':
      case 'a':
      case 'o':
      case 'i':
        // 2 extra points
        score += (*frequencies)[c] * 2;
        break;

      default:
        // 1 extra point
        score += (*frequencies)[c];
        break;
    }
  }
  return score;
}

void print_frequencies(const std::vector<int> &frequencies) {
  for (unsigned int i = 0; i < frequencies.size(); ++i) {
    if (!frequencies[i])
      continue;

    printf("  %d (", i);
    if (isprint(i))
      printf("%c", i);
    else
      printf("nonprint");
    printf("): %d times\n", frequencies[i]);
  }
}

std::string xor_one(const std::string &buf, int int_mask) {
  std::string result(buf);

  unsigned char mask = int_mask & 0x000000ff;
  const size_t size = result.size();
  for (size_t i = 0; i < size; ++i) {
    result[i] = result[i] ^ mask;
  }

  return std::move(result);
}

int try_all_xors(const std::string &buf) {
  int highest_score = 0;
  int mask_for_highest_score = 0;

  // avoid xoring with 0
  for (int i = 0x01; i <= 0xff; ++i) {
    std::string xord_buffer = xor_one(buf, i);

    std::vector<int> frequencies(256, 0);
    int score = compute_frequencies(xord_buffer, &frequencies);

    if (!score)
      // skip XORs with score 0
      continue;

    printf("XOR with %d (0x%x) has score: %d\n", i, i, score);
    //print_frequencies(frequencies);
    //printf("Result: %.*s\n", (int)xord_buffer.size(), xord_buffer.c_str());

    if (score > highest_score) {
      highest_score = score;
      mask_for_highest_score = i;
    }
  }

  printf("Highest score was %d, obtained with mask %d (0x%x)\n",
         highest_score, mask_for_highest_score, mask_for_highest_score);

  if (!highest_score)
    // print nothing if highest score was 0
    return 1;

  {
    std::string xord_buffer = xor_one(buf, mask_for_highest_score);
    std::vector<int> frequencies(256, 0);
    compute_frequencies(xord_buffer, &frequencies);

    print_frequencies(frequencies);
    printf("Result: %.*s\n", (int)xord_buffer.size(), xord_buffer.c_str());
  }

  return 0;
}

int main (void) {
  std::string buf;

  read_buffer(&buf);
  //printf("\n%.*s\n", (int)buf.size(), buf.c_str());

  std::vector<int> frequencies(256, 0);
  /*int score =*/ compute_frequencies(buf, &frequencies);

  //printf("Original distribution (score %d):\n", score);
  //print_frequencies(frequencies);

  return try_all_xors(buf);
}
