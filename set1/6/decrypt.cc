#include <string>
#include <vector>
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

bool is_valid(int c) {
  return isprint(c) || isspace(c);
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

    fprintf(stderr, "  %d (", i);
    if (isprint(i))
      fprintf(stderr, "%c", i);
    else
      fprintf(stderr, "nonprint");
    fprintf(stderr, "): %d times\n", frequencies[i]);
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

    fprintf(stderr, "XOR with %d (0x%x) has score: %d\n", i, i, score);
    //print_frequencies(frequencies);
    //printf("Result: %.*s\n", (int)xord_buffer.size(), xord_buffer.c_str());

    if (score > highest_score) {
      highest_score = score;
      mask_for_highest_score = i;
    }
  }

  fprintf(stderr, "Highest score was %d, obtained with mask %d (0x%x)\n",
          highest_score, mask_for_highest_score, mask_for_highest_score);

  if (!highest_score)
    // print nothing if highest score was 0
    return 1;

  {
    std::string xord_buffer = xor_one(buf, mask_for_highest_score);
    std::vector<int> frequencies(256, 0);
    compute_frequencies(xord_buffer, &frequencies);

    print_frequencies(frequencies);
    fprintf(stderr, "Result: %.*s\n", (int)xord_buffer.size(), xord_buffer.c_str());
  }

  return 0;
}

int try_find_keysize(const std::string &s) {
  int guessed_keysize = -1;
  float distance_for_guessed_keysize = 0; // the goal is to minimize this
  for (int keysize = 2; keysize < 40; ++keysize) {
    std::string chunk1(s, 0, keysize);
    std::string chunk2(s, keysize, keysize);

    int distance = hamming_distance(chunk1, chunk2);
    float normalized_distance = (float)distance / keysize;
    fprintf(stderr, "Keysize [%d] generates distance [%d] (normalized %f)\n",
            keysize, distance, normalized_distance);

    if (guessed_keysize == -1 || normalized_distance < distance_for_guessed_keysize) {
      guessed_keysize = keysize;
      distance_for_guessed_keysize = normalized_distance;
    }
  }

  return guessed_keysize;
}

void break_blocks(const std::string &s, const int keysize, std::vector<std::string> *blocks) {
  size_t cursor = 0;
  const size_t size = s.size();

  while (cursor < size) {
    blocks->push_back(std::string(s, cursor, keysize));
    cursor += keysize;
  }
}

void transpose_blocks(const std::vector<std::string> &blocks, std::vector<std::string> *transposed_blocks) {
  const size_t blocks_count = blocks.size();
  const size_t max_block_length = blocks.front().size();

  for (size_t index_in_block = 0; index_in_block < max_block_length; ++index_in_block) {
    // take the char at index 'index_in_block' from each block and accumulate them
    std::string result;

    for (size_t block_idx = 0; block_idx < blocks_count; ++block_idx) {
      const std::string &block = blocks[block_idx];
      if (index_in_block >= block.size())
        // trailing blocks might be shorter
        break;
      result.append(1, block[index_in_block]);
    }

    transposed_blocks->push_back(result);
  }
}

int try_decrypt(const std::string &s, const int keysize) {
  // break the ciphertext into blocks of keysize length
  std::vector<std::string> blocks;
  break_blocks(s, keysize, &blocks);
  fprintf(stderr, "[%ld] blocks created\n", blocks.size());

  // transpose the blocks
  std::vector<std::string> transposed_blocks;
  transpose_blocks(blocks, &transposed_blocks);
  fprintf(stderr, "[%ld] transposed blocks created\n", transposed_blocks.size());

  for (size_t transposed_block_idx = 0; transposed_block_idx < transposed_blocks.size(); ++transposed_block_idx) {
    try_all_xors(transposed_blocks[transposed_block_idx]);
  }

  return 0;
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

  int keysize = try_find_keysize(decoded_input);
  if (keysize < 0) {
    fprintf(stderr, "Could not guess keysize\n");
    return 1;
  }
  fprintf(stderr, "Guessed keysize of [%d]\n", keysize);

  try_decrypt(decoded_input, keysize);

  return 0;
}
