#include <gtest/gtest.h>

#include <cryfs/localstate/LocalStateMetadata.h>
#include <cpp-utils/tempfile/TempDir.h>
#include <fstream>
#include <cpp-utils/data/DataFixture.h>

using cpputils::TempDir;
using cpputils::Data;
using cryfs::LocalStateMetadata;
using cpputils::DataFixture;
using std::ofstream;

class LocalStateMetadataTest : public ::testing::Test {
public:
    TempDir stateDir;
    TempDir stateDir2;
};

TEST_F(LocalStateMetadataTest, myClientId_ValueIsConsistent) {
    LocalStateMetadata metadata1 = LocalStateMetadata::loadOrGenerate(stateDir.path(), Data(0));
    LocalStateMetadata metadata2 = LocalStateMetadata::loadOrGenerate(stateDir.path(), Data(0));
    EXPECT_EQ(metadata1.myClientId(), metadata2.myClientId());
}

TEST_F(LocalStateMetadataTest, myClientId_ValueIsRandomForNewClient) {
    LocalStateMetadata metadata1 = LocalStateMetadata::loadOrGenerate(stateDir.path(), Data(0));
    LocalStateMetadata metadata2 = LocalStateMetadata::loadOrGenerate(stateDir2.path(), Data(0));
    EXPECT_NE(metadata1.myClientId(), metadata2.myClientId());
}

#ifndef CRYFS_NO_COMPATIBILITY
TEST_F(LocalStateMetadataTest, myClientId_TakesLegacyValueIfSpecified) {
  ofstream file((stateDir.path() / "myClientId").native());
  file << 12345u;
  file.close();

  LocalStateMetadata metadata = LocalStateMetadata::loadOrGenerate(stateDir.path(), Data(0));
  EXPECT_EQ(12345u, metadata.myClientId());
}
#endif

TEST_F(LocalStateMetadataTest, encryptionKeyHash_whenLoadingWithSameKey_thenDoesntCrash) {
  LocalStateMetadata::loadOrGenerate(stateDir.path(), DataFixture::generate(1024));
  LocalStateMetadata::loadOrGenerate(stateDir.path(), DataFixture::generate(1024));
}

TEST_F(LocalStateMetadataTest, encryptionKeyHash_whenLoadingWithDifferentKey_thenCrashes) {
  LocalStateMetadata::loadOrGenerate(stateDir.path(), DataFixture::generate(1024, 1));
  EXPECT_THROW(
    LocalStateMetadata::loadOrGenerate(stateDir.path(), DataFixture::generate(1024, 2)),
    std::runtime_error
  );
}
