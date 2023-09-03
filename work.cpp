#include <stdint.h>
#include <iostream>
#include <random>
// https://github.com/BLAKE2/BLAKE2/blob/master/b2sum/b2sum.c shows some usage
// https://github.com/nanocurrency/nano-node/blob/f400b28aa567f43d0cd11e96eeb9f49bce830203/nano/lib/work.cpp#L59C7-L59C7
#include "BLAKE2/blake2.h"

// Work threshold is 64 bits (8 bytes), also, this should always be big endian, I think
uint64_t BANANO_WORK_THRESHOLD = 0xFFFFFE0000000000;

// Work is 64 bits (8 bytes) nonce
// Hash is 256 bits (32 bytes), either block hash of previous, or public key if there is no previous. There is no uint256_t so instead an array of uint8_t is used
// Result hash on the other hand, is 64 bits (8 bytes), for whatever reason

uint64_t gen_work_random(uint8_t hash[32]) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned long long> distrib(
    std::numeric_limits<std::uint64_t>::min(),
    std::numeric_limits<std::uint64_t>::max()
  );
  blake2b_state result_hash;
  while (true) {
    uint64_t nonce = distrib(gen);
    uint64_t result = 0;
    blake2b_init(&result_hash, 8);
    // Change to buffer
    blake2b_update(&result_hash, &nonce, 8);
    blake2b_update(&result_hash, hash, 32);
    blake2b_final(&result_hash, &result, 8);
    if (result > BANANO_WORK_THRESHOLD) {
      return nonce;
    }
  }
}

uint64_t gen_work_deterministic(uint8_t hash[32]) {
  uint64_t nonce = 0;
  blake2b_state result_hash;
  while (true) {
    uint64_t result = 0;
    blake2b_init(&result_hash, 8);
    blake2b_update(&result_hash, &nonce, 8);
    // Not &hash since passing an array as a parameter turns it into a pointer, apparently (arrays are not passed by value)
    blake2b_update(&result_hash, hash, 32);
    blake2b_final(&result_hash, &result, 8);
    if (result > BANANO_WORK_THRESHOLD) {
      return nonce;
    }
    ++nonce;
  }
}

//clang++ -Wall -std=c++20 work.cpp BLAKE2/blake2b.c -o work_test; ./work_test

int main() {
  uint8_t block_hash[32] = {183, 251, 239, 51, 86, 126, 55, 224, 78, 119, 44, 71, 60, 206, 212, 250, 146, 69, 204, 122, 76, 27, 222, 138, 37, 118, 247, 56, 78, 121, 25, 225};
  std::cout << gen_work_deterministic(block_hash) << std::endl;
  std::cout << gen_work_random(block_hash) << std::endl;
  return 0;
}
