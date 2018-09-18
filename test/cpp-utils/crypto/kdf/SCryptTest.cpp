#include <gtest/gtest.h>
#include "cpp-utils/crypto/kdf/Scrypt.h"

using namespace cpputils;
using std::string;

class SCryptTest : public ::testing::Test {
public:
    unique_ref<SCrypt> scryptForNewKey = SCrypt::forNewKey(SCrypt::TestSettings);
    unique_ref<SCrypt> scryptForExistingKey = SCrypt::forExistingKey(scryptForNewKey->kdfParameters());

    SCryptParameters kdfParameters(const SCrypt &scrypt) {
        SCryptParameters result = SCryptParameters::deserialize(scrypt.kdfParameters());
        return result;
    }

    template<size_t SIZE>
    bool keyEquals(const EncryptionKey<SIZE>& lhs, const EncryptionKey<SIZE>& rhs) {
        return 0 == std::memcmp(lhs.data(), rhs.data(), SIZE);
    }
};

TEST_F(SCryptTest, GeneratedKeyIsReproductible_448) {
    auto derivedKey = scryptForNewKey->deriveKey<56>("mypassword");
    auto rederivedKey = scryptForExistingKey->deriveKey<56>("mypassword");
    EXPECT_TRUE(keyEquals(derivedKey, rederivedKey));
}

TEST_F(SCryptTest, GeneratedKeyIsReproductible_256) {
    auto derivedKey = scryptForNewKey->deriveKey<32>("mypassword");
    auto rederivedKey = scryptForExistingKey->deriveKey<32>("mypassword");
    EXPECT_TRUE(keyEquals(derivedKey, rederivedKey));
}

TEST_F(SCryptTest, GeneratedKeyIsReproductible_128) {
    auto derivedKey = scryptForNewKey->deriveKey<16>("mypassword");
    auto rederivedKey = scryptForExistingKey->deriveKey<16>("mypassword");
    EXPECT_TRUE(keyEquals(derivedKey, rederivedKey));
}

TEST_F(SCryptTest, GeneratedKeyIsReproductible_DefaultSettings) {
    auto derivedKey = scryptForNewKey->deriveKey<16>("mypassword");
    auto rederivedKey = scryptForExistingKey->deriveKey<16>("mypassword");
    EXPECT_TRUE(keyEquals(derivedKey, rederivedKey));
}

TEST_F(SCryptTest, DifferentPasswordResultsInDifferentKey) {
    auto derivedKey = scryptForNewKey->deriveKey<16>("mypassword");
    auto rederivedKey = scryptForExistingKey->deriveKey<16>("mypassword2");
    EXPECT_FALSE(keyEquals(derivedKey, rederivedKey));
}

TEST_F(SCryptTest, UsesCorrectSettings) {
    auto scrypt = SCrypt::forNewKey(SCrypt::TestSettings);
    SCryptParameters parameters = kdfParameters(*scrypt);
    EXPECT_EQ(SCrypt::TestSettings.SALT_LEN, parameters.salt().size());
    EXPECT_EQ(SCrypt::TestSettings.N, parameters.N());
    EXPECT_EQ(SCrypt::TestSettings.r, parameters.r());
    EXPECT_EQ(SCrypt::TestSettings.p, parameters.p());
}

TEST_F(SCryptTest, UsesCorrectDefaultSettings) {
    auto scrypt = SCrypt::forNewKey(SCrypt::DefaultSettings);
    SCryptParameters parameters = kdfParameters(*scrypt);
    EXPECT_EQ(SCrypt::DefaultSettings.SALT_LEN, parameters.salt().size());
    EXPECT_EQ(SCrypt::DefaultSettings.N, parameters.N());
    EXPECT_EQ(SCrypt::DefaultSettings.r, parameters.r());
    EXPECT_EQ(SCrypt::DefaultSettings.p, parameters.p());
}
