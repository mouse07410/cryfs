// This file is meant to be included by BlockStoreTest.h only

#include "../../utils/Data.h"

class BlockStoreSizeParameterizedTest {
public:
  BlockStoreSizeParameterizedTest(std::unique_ptr<blockstore::BlockStore> blockStore_, size_t size_): blockStore(std::move(blockStore_)), size(size_) {}

  void TestCreatedBlockHasCorrectSize() {
    auto block = CreateBlock();
    EXPECT_EQ(size, block->size());
  }

  void TestLoadingUnchangedBlockHasCorrectSize() {
    blockstore::Key key = CreateBlock()->key();
    auto loaded_block = blockStore->load(key);
    EXPECT_EQ(size, loaded_block->size());
  }

  void TestCreatedBlockData() {
	DataBlockFixture dataFixture(size);
	blockstore::Data data(size);
	std::memcpy(data.data(), dataFixture.data(), size);
	auto block = blockStore->create(data);
	EXPECT_EQ(0, std::memcmp(dataFixture.data(), block->data(), size));

  }

  void TestLoadingUnchangedBlockData() {
	DataBlockFixture dataFixture(size);
	blockstore::Data data(size);
	std::memcpy(data.data(), dataFixture.data(), size);
	blockstore::Key key = blockStore->create(data)->key();
    auto loaded_block = blockStore->load(key);
    EXPECT_EQ(0, std::memcmp(dataFixture.data(), loaded_block->data(), size));
  }

  void TestLoadedBlockIsCorrect() {
    DataBlockFixture randomData(size);
    auto loaded_block = StoreDataToBlockAndLoadIt(randomData);
    EXPECT_EQ(size, loaded_block->size());
    EXPECT_EQ(0, std::memcmp(randomData.data(), loaded_block->data(), size));
  }

  void TestLoadedBlockIsCorrectWhenLoadedDirectlyAfterFlushing() {
    DataBlockFixture randomData(size);
    auto loaded_block = StoreDataToBlockAndLoadItDirectlyAfterFlushing(randomData);
    EXPECT_EQ(size, loaded_block->size());
    EXPECT_EQ(0, std::memcmp(randomData.data(), loaded_block->data(), size));
  }

  void TestAfterCreate_FlushingDoesntChangeBlock() {
    DataBlockFixture randomData(size);
    auto block =  CreateBlock();
    WriteDataToBlock(block.get(), randomData);
    block->flush();

    EXPECT_BLOCK_DATA_CORRECT(*block, randomData);
  }

  void TestAfterLoad_FlushingDoesntChangeBlock() {
    DataBlockFixture randomData(size);
    auto block =  CreateBlockAndLoadIt();
    WriteDataToBlock(block.get(), randomData);
    block->flush();

    EXPECT_BLOCK_DATA_CORRECT(*block, randomData);
  }

  void TestAfterCreate_FlushesWhenDestructed() {
    DataBlockFixture randomData(size);
    blockstore::Key key = key;
    {
      auto block = blockStore->create(blockstore::Data(size));
      key = block->key();
      WriteDataToBlock(block.get(), randomData);
    }
    auto loaded_block = blockStore->load(key);
    EXPECT_BLOCK_DATA_CORRECT(*loaded_block, randomData);
  }

  void TestAfterLoad_FlushesWhenDestructed() {
    DataBlockFixture randomData(size);
    blockstore::Key key = key;
    {
      key = CreateBlock()->key();
      auto block = blockStore->load(key);
      WriteDataToBlock(block.get(), randomData);
    }
    auto loaded_block = blockStore->load(key);
    EXPECT_BLOCK_DATA_CORRECT(*loaded_block, randomData);
  }

  void TestLoadNonExistingBlock() {
    EXPECT_FALSE(
        (bool)blockStore->load(key)
    );
  }

private:
  const blockstore::Key key = blockstore::Key::FromString("1491BB4932A389EE14BC7090AC772972");
  std::unique_ptr<blockstore::BlockStore> blockStore;
  size_t size;

  blockstore::Data ZEROES(size_t size) {
    blockstore::Data ZEROES(size);
    ZEROES.FillWithZeroes();
    return ZEROES;
  }

  std::unique_ptr<blockstore::Block> StoreDataToBlockAndLoadIt(const DataBlockFixture &data) {
    blockstore::Key key = StoreDataToBlockAndGetKey(data);
    return blockStore->load(key);
  }

  blockstore::Key StoreDataToBlockAndGetKey(const DataBlockFixture &dataFixture) {
	blockstore::Data data(dataFixture.size());
	std::memcpy(data.data(), dataFixture.data(), dataFixture.size());
    return blockStore->create(data)->key();
  }

  std::unique_ptr<blockstore::Block> StoreDataToBlockAndLoadItDirectlyAfterFlushing(const DataBlockFixture &dataFixture) {
	blockstore::Data data(dataFixture.size());
	std::memcpy(data.data(), dataFixture.data(), dataFixture.size());
    auto block = blockStore->create(data);
    block->flush();
    return blockStore->load(block->key());
  }

  std::unique_ptr<blockstore::Block> CreateBlockAndLoadIt() {
    blockstore::Key key = CreateBlock()->key();
    return blockStore->load(key);
  }

  std::unique_ptr<blockstore::Block> CreateBlock() {
    return blockStore->create(blockstore::Data(size));
  }

  void WriteDataToBlock(blockstore::Block *block, const DataBlockFixture &randomData) {
    block->write(randomData.data(), 0, randomData.size());
  }

  void EXPECT_BLOCK_DATA_CORRECT(const blockstore::Block &block, const DataBlockFixture &randomData) {
    EXPECT_EQ(randomData.size(), block.size());
    EXPECT_EQ(0, std::memcmp(randomData.data(), block.data(), randomData.size()));
  }
};

constexpr std::initializer_list<size_t> SIZES = {0, 1, 1024, 4096, 10*1024*1024};
#define TYPED_TEST_P_FOR_ALL_SIZES(TestName)                                    \
  TYPED_TEST_P(BlockStoreTest, TestName) {                                      \
    for (auto size: SIZES) {                                                    \
      BlockStoreSizeParameterizedTest(this->fixture.createBlockStore(), size)   \
        .Test##TestName();                                                      \
    }                                                                           \
  }                                                                             \

TYPED_TEST_P_FOR_ALL_SIZES(CreatedBlockHasCorrectSize);
TYPED_TEST_P_FOR_ALL_SIZES(LoadingUnchangedBlockHasCorrectSize);
TYPED_TEST_P_FOR_ALL_SIZES(CreatedBlockData);
TYPED_TEST_P_FOR_ALL_SIZES(LoadingUnchangedBlockData);
TYPED_TEST_P_FOR_ALL_SIZES(LoadedBlockIsCorrect);
//TYPED_TEST_P_FOR_ALL_SIZES(LoadedBlockIsCorrectWhenLoadedDirectlyAfterFlushing);
TYPED_TEST_P_FOR_ALL_SIZES(AfterCreate_FlushingDoesntChangeBlock);
TYPED_TEST_P_FOR_ALL_SIZES(AfterLoad_FlushingDoesntChangeBlock);
TYPED_TEST_P_FOR_ALL_SIZES(AfterCreate_FlushesWhenDestructed);
TYPED_TEST_P_FOR_ALL_SIZES(AfterLoad_FlushesWhenDestructed);
TYPED_TEST_P_FOR_ALL_SIZES(LoadNonExistingBlock);
