#include <queue>
#include <string>
#include <vector>
#include "base64.h"
#include "repkey_xor.h"

class KeysizeMetadata {
 public:
  KeysizeMetadata(int keysize, float distance)
      : keysize_(keysize), distance_(distance) {}
  KeysizeMetadata(const KeysizeMetadata &other)
      : keysize_(other.keysize_), distance_(other.distance_) {}

  int keysize_;
  float distance_;
};
class BetterKeysizeComparator {
 public:
  bool operator() (const KeysizeMetadata &lhs, const KeysizeMetadata &rhs) const {
    // We must determine if lhs is 'better' than rhs; comparison is
    // done on the distance.
    //
    // Since the priority_queue calls this class with 'less'
    // semantics, and places 'higher' elements at the beginning (i.e.,
    // elements for which 'less' returns false when compared to
    // everything else), we have to reverse the logic. Hence, return
    // whether lhs is 'worse' than rhs.
    return lhs.distance_ > rhs.distance_;
  }
};

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

std::vector<int> try_find_keysize(const std::string &s, const size_t keysizes_count) {
  std::priority_queue<KeysizeMetadata, std::vector<KeysizeMetadata>,
                      BetterKeysizeComparator> keysizes_data;

  for (int keysize = 2; keysize < 40; ++keysize) {
    std::string chunk1(s, 0, keysize);
    std::string chunk2(s, keysize, keysize);
    std::string chunk3(s, keysize * 2, keysize);
    std::string chunk4(s, keysize * 3, keysize);

    // use more than one sample
    int distance = hamming_distance(chunk1, chunk2) +
        hamming_distance(chunk2, chunk3) +
        hamming_distance(chunk3, chunk4);
    float normalized_distance = (float)distance / keysize;
    fprintf(stderr, "Keysize [%d] generates distance [%d] (normalized %f)\n",
            keysize, distance, normalized_distance);

    keysizes_data.push(KeysizeMetadata(keysize, normalized_distance));
  }

  // return the N best keysizes
  std::vector<int> best_keysizes;
  while (!keysizes_data.empty()) {
    const KeysizeMetadata &top = keysizes_data.top();
    best_keysizes.push_back(top.keysize_);
    keysizes_data.pop();
    if (best_keysizes.size() >= keysizes_count)
      // we have collected enough keysizes
      break;
  }

  return best_keysizes;
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

bool try_decrypt(const std::string &s, const int keysize) {
  // break the ciphertext into blocks of keysize length
  std::vector<std::string> blocks;
  break_blocks(s, keysize, &blocks);
  //fprintf(stderr, "[%ld] blocks created\n", blocks.size());

  // transpose the blocks
  std::vector<std::string> transposed_blocks;
  transpose_blocks(blocks, &transposed_blocks);
  //fprintf(stderr, "[%ld] transposed blocks created\n", transposed_blocks.size());

  for (size_t transposed_block_idx = 0; transposed_block_idx < transposed_blocks.size(); ++transposed_block_idx) {
    int one_byte_key;
    if (!try_all_xors(transposed_blocks[transposed_block_idx], &one_byte_key)) {
      fprintf(stderr, "Keysize [%d]: failed to guess byte [%ld] for the key\n", keysize, transposed_block_idx);
      return false;
    }
  }

  return true;
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

  std::vector<int> keysizes = try_find_keysize(decoded_input, 5);
  fprintf(stderr, "Guessed [%ld] keysizes:\n", keysizes.size());
  for (int keysize: keysizes) {
    fprintf(stderr, "Trying keysize [%d]\n", keysize);
    try_decrypt(decoded_input, keysize);
  }

  return 0;
}
