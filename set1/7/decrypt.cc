#include <string>
#include <openssl/err.h>
#include <openssl/evp.h>
#include "base64.h"

typedef std::basic_string<unsigned char> u_string;

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

// http://stackoverflow.com/q/16560720/1451820
u_string decrypt(const u_string &ciphertext, const u_string &key) {
  EVP_CIPHER_CTX ctx;
  EVP_CIPHER_CTX_init(&ctx);
  EVP_CIPHER_CTX_set_padding(&ctx, false);
  EVP_DecryptInit_ex(&ctx, EVP_aes_128_ecb(), NULL, key.c_str(), NULL);

  unsigned char buffer[10240], *pointer = buffer;
  int outlen;
  EVP_DecryptUpdate(&ctx, pointer, &outlen, ciphertext.c_str(), ciphertext.length());
  pointer += outlen;
  EVP_DecryptFinal_ex(&ctx, pointer, &outlen);
  pointer += outlen;

  EVP_CIPHER_CTX_cleanup(&ctx);

  return u_string(buffer, pointer-buffer);
}

int main(int argc, char *argv[]) {
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

  ERR_load_crypto_strings();
  u_string ciphertext((unsigned char *) decoded_input.c_str(), decoded_input.size());
  u_string cleartext = decrypt(ciphertext, (unsigned char *) "YELLOW SUBMARINE");
  fprintf(stderr, "Decrypted %ld bytes of input\n", cleartext.size());
  fprintf(stderr, "Cleartext: [%.*s]\n", (int)cleartext.size(), cleartext.c_str());

  return 0;
}
