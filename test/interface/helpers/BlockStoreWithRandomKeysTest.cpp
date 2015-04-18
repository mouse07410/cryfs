#include "../../../interface/helpers/BlockStoreWithRandomKeys.h"
#include "google/gtest/gtest.h"
#include "google/gmock/gmock.h"
#include "../../testutils/DataBlockFixture.h"

using ::testing::Test;
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::Eq;
using ::testing::ByRef;

using std::string;
using std::unique_ptr;
using std::make_unique;

using namespace blockstore;

class BlockStoreWithRandomKeysMock: public BlockStoreWithRandomKeys {
public:
  unique_ptr<Block> tryCreate(const Key &key, Data data) {
    return unique_ptr<Block>(do_create(key, data));
  }
  MOCK_METHOD2(do_create, Block*(const Key &, const Data &data));
  unique_ptr<Block> load(const Key &key) {
    return unique_ptr<Block>(do_load(key));
  }
  MOCK_METHOD1(do_load, Block*(const Key &));
  void remove(unique_ptr<Block> block) {}
  MOCK_CONST_METHOD0(numBlocks, uint64_t());
};

class BlockMock: public Block {
public:
  BlockMock(): Block(Key::CreateRandom()) {}
  MOCK_CONST_METHOD0(data, const void*());
  MOCK_METHOD3(write, void(const void*, uint64_t, uint64_t));
  MOCK_METHOD0(flush, void());
  MOCK_CONST_METHOD0(size, size_t());
  MOCK_CONST_METHOD0(key, const Key&());
};

class BlockStoreWithRandomKeysTest: public Test {
public:
  BlockStoreWithRandomKeysMock blockStoreMock;
  BlockStore &blockStore = blockStoreMock;
  const blockstore::Key key = Key::FromString("1491BB4932A389EE14BC7090AC772972");

  Data createDataWithSize(size_t size) {
	DataBlockFixture fixture(size);
	Data data(size);
	std::memcpy(data.data(), fixture.data(), size);
	return data;
  }
};

TEST_F(BlockStoreWithRandomKeysTest, DataIsPassedThrough0) {
  Data data = createDataWithSize(0);
  EXPECT_CALL(blockStoreMock, do_create(_, Eq(ByRef(data)))).WillOnce(Return(new BlockMock));
  blockStore.create(data);
}

TEST_F(BlockStoreWithRandomKeysTest, DataIsPassedThrough1) {
  Data data = createDataWithSize(1);
  EXPECT_CALL(blockStoreMock, do_create(_, Eq(ByRef(data)))).WillOnce(Return(new BlockMock));
  blockStore.create(data);
}

TEST_F(BlockStoreWithRandomKeysTest, DataIsPassedThrough1024) {
  Data data = createDataWithSize(1024);
  EXPECT_CALL(blockStoreMock, do_create(_, Eq(ByRef(data)))).WillOnce(Return(new BlockMock));
  blockStore.create(data);
}

TEST_F(BlockStoreWithRandomKeysTest, KeyHasCorrectSize) {
  EXPECT_CALL(blockStoreMock, do_create(_, _)).WillOnce(Invoke([](const Key &key, const Data &) {
    EXPECT_EQ(Key::STRING_LENGTH, key.ToString().size());
    return new BlockMock;
  }));

  blockStore.create(createDataWithSize(1024));
}

TEST_F(BlockStoreWithRandomKeysTest, TwoBlocksGetDifferentKeys) {
  Key first_key = key;
  EXPECT_CALL(blockStoreMock, do_create(_, _))
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        first_key = key;
        return new BlockMock;
      }))
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        EXPECT_NE(first_key, key);
        return new BlockMock;
      }));

  Data data = createDataWithSize(1024);
  blockStore.create(data);
  blockStore.create(data);
}

TEST_F(BlockStoreWithRandomKeysTest, WillTryADifferentKeyIfKeyAlreadyExists) {
  Key first_key = key;
  Data data = createDataWithSize(1024);
  EXPECT_CALL(blockStoreMock, do_create(_, Eq(ByRef(data))))
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        first_key = key;
        return nullptr;
      }))
	  //TODO Check that this test case fails when the second do_create call gets different data
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        EXPECT_NE(first_key, key);
        return new BlockMock;
      }));

  blockStore.create(data);
}

TEST_F(BlockStoreWithRandomKeysTest, WillTryADifferentKeyIfKeyAlreadyExistsTwoTimes) {
  Key first_key = key;
  Data data = createDataWithSize(1024);
  EXPECT_CALL(blockStoreMock, do_create(_, Eq(ByRef(data))))
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        first_key = key;
        return nullptr;
      }))
	  //TODO Check that this test case fails when the second/third do_create calls get different data
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        first_key = key;
        return nullptr;
      }))
      .WillOnce(Invoke([&first_key](const Key &key, const Data &) {
        EXPECT_NE(first_key, key);
        return new BlockMock;
      }));

  blockStore.create(data);
}
