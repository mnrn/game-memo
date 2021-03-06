#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "matcher.hpp"
#include "secure/hash/sha1.hpp"
#include "secure/hash/sha256.hpp"
#include "secure/hash/sha384.hpp"
#include "secure/hash/sha512.hpp"

TEST_CASE("SHA1-Example") {
  SECTION("One-Block Message") {
    const auto bytes = sha1::hash("abc");
    CHECK_THAT(bytes, expect("a9993e36 4706816a ba3e2571 7850c26c 9cd0d89d"));
  }
  SECTION("Multi-Block Message") {
    const auto bytes =
        sha1::hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    CHECK_THAT(bytes, expect("84983e44 1c3bd26e baae4aa1 f95129e5 e54670f1"));
  }
  SECTION("Long Message") {
    const std::vector<std::uint8_t> msg(1000000, 0x61);
    const auto bytes = sha1::hash(msg);
    CHECK_THAT(bytes, expect("34aa973c d4c4daa4 f61eeb2b dbad2731 6534016f"));
  }
}

TEST_CASE("SHA256-Example") {
  SECTION("One-Block Message") {
    const auto bytes = sha256::hash("abc");
    CHECK_THAT(bytes, expect("ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 "
                             "96177a9c b410ff61 f20015ad"));
  }
  SECTION("Multi-Block Message") {
    const auto bytes = sha256::hash(
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    CHECK_THAT(bytes, expect("248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 "
                             "64ff2167 f6ecedd4 19db06c1"));
  }
  SECTION("Long Message") {
    const std::vector<std::uint8_t> msg(1000000, 0x61);
    const auto bytes = sha256::hash(msg);
    CHECK_THAT(bytes, expect("cdc76e5c 9914fb92 81a1c7e2 84d73e67 f1809a48 "
                             "a497200e 046d39cc c7112cd0"));
  }
}

TEST_CASE("SHA512-Example") {
  SECTION("One-Block Message") {
    const auto bytes = sha512::hash("abc");
    CHECK_THAT(bytes,
               expect("ddaf35a193617aba cc417349ae204131 12e6fa4e89a97ea2 "
                      "0a9eeee64b55d39a 2192992a274fc1a8 36ba3c23a3feebbd "
                      "454d4423643ce80e 2a9ac94fa54ca49f"));
  }
  SECTION("Multi-Block Message") {
    const auto bytes = sha512::hash(
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhi"
        "jklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
    CHECK_THAT(bytes,
               expect("8e959b75dae313da 8cf4f72814fc143f 8f7779c6eb9f7fa1 "
                      "7299aeadb6889018 501d289e4900f7e4 331b99dec4b5433a "
                      "c7d329eeb6dd2654 5e96e55b874be909"));
  }
  SECTION("Long Message") {
    const std::vector<std::uint8_t> msg(1000000, 0x61);
    const auto bytes = sha512::hash(msg);
    CHECK_THAT(bytes,
               expect("e718483d0ce76964 4e2e42c7bc15b463 8e1f98b13b204428 "
                      "5632a803afa973eb de0ff244877ea60a 4cb0432ce577c31b "
                      "eb009c5c2c49aa2e 4eadb217ad8cc09b"));
  }
}

TEST_CASE("SHA384-Example") {
  SECTION("One-Block Message") {
    const auto bytes = sha384::hash("abc");
    CHECK_THAT(bytes,
               expect("cb00753f45a35e8b b5a03d699ac65007 272c32ab0eded163 "
                      "1a8b605a43ff5bed 8086072ba1e7cc23 58baeca134c825a7"));
  }
  SECTION("Multi-Block Message") {
    const auto bytes = sha384::hash(
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhi"
        "jklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");
    CHECK_THAT(bytes,
               expect("09330c33f71147e8 3d192fc782cd1b47 53111b173b3b05d2 "
                      "2fa08086e3b0f712 fcc7c71a557e2db9 66c3e9fa91746039"));
  }
  SECTION("Long Message") {
    const std::vector<std::uint8_t> msg(1000000, 0x61);
    const auto bytes = sha384::hash(msg);
    CHECK_THAT(bytes,
               expect("9d0e1809716474cb 086e834e310a4a1c ed149e9c00f24852 "
                      "7972cec5704c2a5b 07b8b3dc38ecc4eb ae97ddd87f3d8985"));
  }
}
