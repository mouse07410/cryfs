#include <gtest/gtest.h>

#include "blockstore/implementations/inmemory/InMemoryBlockStore.h"
#include "blockstore/implementations/inmemory/InMemoryBlock.h"
#include "blobstore/implementations/onblocks/BlobStoreOnBlocks.h"
#include "blobstore/implementations/onblocks/impl/DataLeafNode.h"
#include "test/testutils/DataBlockFixture.h"

using ::testing::Test;
using ::testing::WithParamInterface;
using ::testing::Values;
using std::unique_ptr;
using std::make_unique;
using std::string;

using blockstore::BlockStore;
using blockstore::Data;
using blockstore::inmemory::InMemoryBlockStore;
using namespace blobstore;
using namespace blobstore::onblocks;

#define EXPECT_IS_PTR_TYPE(Type, ptr) EXPECT_NE(nullptr, dynamic_cast<Type*>(ptr)) << "Given pointer cannot be cast to the given type"

class DataLeafNodeTest: public Test {
public:
  unique_ptr<BlockStore> blockStore = make_unique<InMemoryBlockStore>();

  DataLeafNodeTest(): randomData(DataLeafNode::MAX_STORED_BYTES) {
    DataBlockFixture dataFixture(DataLeafNode::MAX_STORED_BYTES);

    std::memcpy(randomData.data(), dataFixture.data(), randomData.size());
  }

  Key WriteDataToNewLeafBlockAndReturnKey() {
    auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
    auto leaf = DataNode::createNewLeafNode(std::move(block.block));
    leaf->resize(randomData.size());
    leaf->write(0, randomData.size(), randomData);
    return block.key;
  }

  void ReadDataFromLeafBlock(Key key, Data *data) {
    auto leaf = DataNode::load(blockStore->load(key));
    EXPECT_IS_PTR_TYPE(DataLeafNode, leaf.get());
    leaf->read(0, data->size(), data);
  }

  Data randomData;
};

TEST_F(DataLeafNodeTest, ReadWrittenDataImmediately) {
  auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
  auto leaf = DataNode::createNewLeafNode(std::move(block.block));
  leaf->resize(randomData.size());
  leaf->write(0, randomData.size(), randomData);

  Data read(DataLeafNode::MAX_STORED_BYTES);
  leaf->read(0, read.size(), &read);
  EXPECT_EQ(0, std::memcmp(randomData.data(), read.data(), randomData.size()));
}

TEST_F(DataLeafNodeTest, ReadWrittenDataAfterReloadingBLock) {
  Key key = WriteDataToNewLeafBlockAndReturnKey();

  Data data(DataLeafNode::MAX_STORED_BYTES);
  ReadDataFromLeafBlock(key, &data);

  EXPECT_EQ(0, std::memcmp(randomData.data(), data.data(), randomData.size()));
}

TEST_F(DataLeafNodeTest, NewLeafNodeHasSizeZero) {
  auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
  auto leaf = DataNode::createNewLeafNode(std::move(block.block));
  EXPECT_EQ(0u, leaf->numBytesInThisNode());
}

TEST_F(DataLeafNodeTest, NewLeafNodeHasSizeZero_AfterLoading) {
  auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
  {
    DataNode::createNewLeafNode(std::move(block.block));
  }
  auto leaf = DataNode::load(blockStore->load(block.key));

  EXPECT_EQ(0u, leaf->numBytesInThisNode());
}

class DataLeafNodeSizeTest: public DataLeafNodeTest, public WithParamInterface<unsigned int> {};
INSTANTIATE_TEST_CASE_P(DataLeafNodeSizeTest, DataLeafNodeSizeTest, Values(0, 1, 5, 16, 32, 512, DataLeafNode::MAX_STORED_BYTES));

TEST_P(DataLeafNodeSizeTest, ResizeNode_ReadSizeImmediately) {
  auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
  auto leaf = DataNode::createNewLeafNode(std::move(block.block));
  leaf->resize(GetParam());
  EXPECT_EQ(GetParam(), leaf->numBytesInThisNode());
}

TEST_P(DataLeafNodeSizeTest, ResizeNode_ReadSizeAfterLoading) {
  auto block = blockStore->create(DataNodeView::BLOCKSIZE_BYTES);
  {
    auto leaf = DataNode::createNewLeafNode(std::move(block.block));
    leaf->resize(GetParam());
  }
  auto leaf = DataNode::load(blockStore->load(block.key));
  EXPECT_EQ(GetParam(), leaf->numBytesInThisNode());
}

//TODO Write tests that only read part of the data
//TODO Test numBytesInThisNode()