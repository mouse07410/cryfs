#include "testutils/DataTreeTest.h"
#include "testutils/TwoLevelDataFixture.h"
#include "blobstore/implementations/onblocks/utils/Math.h"
#include <cpp-utils/data/Data.h>

#include <tuple>

using ::testing::WithParamInterface;
using ::testing::Values;
using ::testing::Combine;
using std::tuple;
using std::get;
using std::function;
using std::mem_fn;
using cpputils::dynamic_pointer_move;

using blobstore::onblocks::datanodestore::DataLeafNode;
using blobstore::onblocks::datanodestore::DataInnerNode;
using blobstore::onblocks::datanodestore::DataNode;
using blobstore::onblocks::datanodestore::DataNodeLayout;
using blobstore::onblocks::datatreestore::DataTree;
using blobstore::onblocks::datatreestore::LeafHandle;
using blobstore::onblocks::utils::ceilDivision;
using blockstore::BlockId;
using cpputils::Data;
using boost::none;

using cpputils::unique_ref;

class DataTreeTest_ResizeByTraversing: public DataTreeTest {
public:
  static constexpr DataNodeLayout LAYOUT = DataNodeLayout(BLOCKSIZE_BYTES);

  unique_ref<DataTree> CreateTree(unique_ref<DataNode> root) {
    BlockId blockId = root->blockId();
    cpputils::destruct(std::move(root));
    return treeStore.load(blockId).value();
  }

  unique_ref<DataTree> CreateLeafTreeWithSize(uint32_t size) {
    return CreateTree(CreateLeafWithSize(size));
  }

  unique_ref<DataTree> CreateTwoLeafTreeWithSecondLeafSize(uint32_t size) {
    return CreateTree(CreateTwoLeafWithSecondLeafSize(size));
  }

  unique_ref<DataTree> CreateFullTwoLevelTreeWithLastLeafSize(uint32_t size) {
    return CreateTree(CreateFullTwoLevelWithLastLeafSize(size));
  }

  unique_ref<DataTree> CreateThreeLevelTreeWithTwoChildrenAndLastLeafSize(uint32_t size) {
    return CreateTree(CreateThreeLevelWithTwoChildrenAndLastLeafSize(size));
  }

  unique_ref<DataTree> CreateThreeLevelTreeWithThreeChildrenAndLastLeafSize(uint32_t size) {
    return CreateTree(CreateThreeLevelWithThreeChildrenAndLastLeafSize(size));
  }

  unique_ref<DataTree> CreateFullThreeLevelTreeWithLastLeafSize(uint32_t size) {
    return CreateTree(CreateFullThreeLevelWithLastLeafSize(size));
  }

  unique_ref<DataTree> CreateFourLevelMinDataTreeWithLastLeafSize(uint32_t size) {
    return CreateTree(CreateFourLevelMinDataWithLastLeafSize(size));
  }

  void EXPECT_IS_LEFTMAXDATA_TREE(const BlockId &blockId) {
    auto root = nodeStore->load(blockId).value();
    DataInnerNode *inner = dynamic_cast<DataInnerNode*>(root.get());
    if (inner != nullptr) {
      for (uint32_t i = 0; i < inner->numChildren()-1; ++i) {
        EXPECT_IS_MAXDATA_TREE(inner->readChild(i).blockId());
      }
      EXPECT_IS_LEFTMAXDATA_TREE(inner->readLastChild().blockId());
    }
  }

  void EXPECT_IS_MAXDATA_TREE(const BlockId &blockId) {
    auto root = nodeStore->load(blockId).value();
    DataInnerNode *inner = dynamic_cast<DataInnerNode*>(root.get());
    if (inner != nullptr) {
      for (uint32_t i = 0; i < inner->numChildren(); ++i) {
        EXPECT_IS_MAXDATA_TREE(inner->readChild(i).blockId());
      }
    } else {
      DataLeafNode *leaf = dynamic_cast<DataLeafNode*>(root.get());
      EXPECT_EQ(nodeStore->layout().maxBytesPerLeaf(), leaf->numBytes());
    }
  }
};
constexpr DataNodeLayout DataTreeTest_ResizeByTraversing::LAYOUT;

class DataTreeTest_ResizeByTraversing_P: public DataTreeTest_ResizeByTraversing, public WithParamInterface<tuple<function<unique_ref<DataTree>(DataTreeTest_ResizeByTraversing*, uint32_t)>, uint32_t, uint32_t, std::function<uint32_t (uint32_t oldNumberOfLeaves, uint32_t newNumberOfLeaves)>>> {
public:
  DataTreeTest_ResizeByTraversing_P()
    : oldLastLeafSize(get<1>(GetParam())),
      tree(get<0>(GetParam())(this, oldLastLeafSize)),
      numberOfLeavesToAdd(get<2>(GetParam())),
      newNumberOfLeaves(tree->numLeaves()+numberOfLeavesToAdd),
      traversalBeginIndex(get<3>(GetParam())(tree->numLeaves(), newNumberOfLeaves)),
      ZEROES(LAYOUT.maxBytesPerLeaf())
  {
    ZEROES.FillWithZeroes();
  }

  void GrowTree(const BlockId &blockId) {
    auto tree = treeStore.load(blockId);
    GrowTree(tree.get().get());
  }

  void GrowTree(DataTree *tree, std::function<void (int32_t)> traverse = [] (uint32_t){}) {
    uint64_t maxBytesPerLeaf = tree->maxBytesPerLeaf();
    tree->traverseLeaves(traversalBeginIndex, newNumberOfLeaves, [&traverse] (uint32_t index, bool, LeafHandle){traverse(index);}, [maxBytesPerLeaf, &traverse] (uint32_t index) -> Data { traverse(index); return Data(maxBytesPerLeaf).FillWithZeroes();});
    tree->flush();
  }

  unique_ref<DataLeafNode> LastLeaf(const BlockId &blockId) {
    auto root = nodeStore->load(blockId).value();
    auto leaf = dynamic_pointer_move<DataLeafNode>(root);
    if (leaf != none) {
      return std::move(*leaf);
    }
    auto inner = dynamic_pointer_move<DataInnerNode>(root).value();
    return LastLeaf(inner->readLastChild().blockId());
  }

  uint32_t oldLastLeafSize;
  unique_ref<DataTree> tree;
  uint32_t numberOfLeavesToAdd;
  uint32_t newNumberOfLeaves;
  uint32_t traversalBeginIndex;
  Data ZEROES;
};
INSTANTIATE_TEST_CASE_P(DataTreeTest_ResizeByTraversing_P, DataTreeTest_ResizeByTraversing_P,
  Combine(
    //Tree we're starting with
    Values<function<unique_ref<DataTree>(DataTreeTest_ResizeByTraversing*, uint32_t)>>(
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateLeafTreeWithSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateTwoLeafTreeWithSecondLeafSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateFullTwoLevelTreeWithLastLeafSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateThreeLevelTreeWithTwoChildrenAndLastLeafSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateThreeLevelTreeWithThreeChildrenAndLastLeafSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateFullThreeLevelTreeWithLastLeafSize),
      mem_fn(&DataTreeTest_ResizeByTraversing::CreateFourLevelMinDataTreeWithLastLeafSize)
    ),
    //Last leaf size of the start tree
    Values(
      0u,
      1u,
      10u,
      DataTreeTest_ResizeByTraversing::LAYOUT.maxBytesPerLeaf()
    ),
    //Number of leaves we're adding
    Values(
      1u,
      2u,
      DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode(),  //Full two level tree
      2* DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode(), //Three level tree with two children
      3* DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode(), //Three level tree with three children
      DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode() * DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode(), //Full three level tree
      DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode() * DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode() + 1 //Four level mindata tree
    ),
    //Decide the traversal begin index
    Values(
      [] (uint32_t /*oldNumberOfLeaves*/, uint32_t newNumberOfLeaves)     {return newNumberOfLeaves;}, // Don't traverse any leaves, just resize (begin==end)
      [] (uint32_t /*oldNumberOfLeaves*/, uint32_t newNumberOfLeaves)     {return newNumberOfLeaves-1;}, // Traverse last leaf (begin==end-1)
      [] (uint32_t oldNumberOfLeaves,     uint32_t newNumberOfLeaves)     {return (oldNumberOfLeaves+newNumberOfLeaves)/2;}, // Start traversal in middle of new leaves
      [] (uint32_t oldNumberOfLeaves,     uint32_t /*newNumberOfLeaves*/) {return oldNumberOfLeaves-1;}, // Start traversal with last old leaf
      [] (uint32_t oldNumberOfLeaves,     uint32_t /*newNumberOfLeaves*/) {return oldNumberOfLeaves;}, // Start traversal with first new leaf
      [] (uint32_t /*oldNumberOfLeaves*/, uint32_t /*newNumberOfLeaves*/) {return 0;}, // Traverse full tree
      [] (uint32_t /*oldNumberOfLeaves*/, uint32_t /*newNumberOfLeaves*/) {return 1;} // Traverse full tree except first leaf
    )
  )
);

TEST_P(DataTreeTest_ResizeByTraversing_P, StructureIsValid) {
  GrowTree(tree.get());
  EXPECT_IS_LEFTMAXDATA_TREE(tree->blockId());
}

TEST_P(DataTreeTest_ResizeByTraversing_P, NumLeavesIsCorrect_FromCache) {
  tree->numLeaves(); // fill cache with old value
  GrowTree(tree.get());
  // tree->numLeaves() only goes down the right border nodes and expects the tree to be a left max data tree.
  // This is what the StructureIsValid test case is for.
  EXPECT_EQ(newNumberOfLeaves, tree->numLeaves());
}

TEST_P(DataTreeTest_ResizeByTraversing_P, NumLeavesIsCorrect) {
  GrowTree(tree.get());
  // tree->_forceComputeNumLeaves() only goes down the right border nodes and expects the tree to be a left max data tree.
  // This is what the StructureIsValid test case is for.
  EXPECT_EQ(newNumberOfLeaves, tree->_forceComputeNumLeaves());
}

TEST_P(DataTreeTest_ResizeByTraversing_P, DepthFlagsAreCorrect) {
  GrowTree(tree.get());
  uint32_t depth = ceil(log(newNumberOfLeaves)/log(DataTreeTest_ResizeByTraversing::LAYOUT.maxChildrenPerInnerNode()));
  CHECK_DEPTH(depth, tree->blockId());
}

TEST_P(DataTreeTest_ResizeByTraversing_P, KeyDoesntChange) {
  BlockId blockId = tree->blockId();
  tree->flush();
  GrowTree(tree.get());
  EXPECT_EQ(blockId, tree->blockId());
}

TEST_P(DataTreeTest_ResizeByTraversing_P, DataStaysIntact) {
  uint32_t oldNumberOfLeaves = std::max(UINT64_C(1), ceilDivision(tree->numStoredBytes(), static_cast<uint64_t>(nodeStore->layout().maxBytesPerLeaf())));
  TwoLevelDataFixture data(nodeStore, TwoLevelDataFixture::SizePolicy::Unchanged);
  BlockId blockId = tree->blockId();
  cpputils::destruct(std::move(tree));
  data.FillInto(nodeStore->load(blockId).get().get());

  GrowTree(blockId);

  data.EXPECT_DATA_CORRECT(nodeStore->load(blockId).get().get(), oldNumberOfLeaves, oldLastLeafSize);
}

TEST_P(DataTreeTest_ResizeByTraversing_P, AllLeavesAreTraversed) {
  std::vector<uint32_t> traversedLeaves;
  GrowTree(tree.get(), [&traversedLeaves] (uint32_t index) {traversedLeaves.push_back(index);});

  EXPECT_EQ(newNumberOfLeaves-traversalBeginIndex, traversedLeaves.size());
  for (uint32_t i = traversalBeginIndex; i < newNumberOfLeaves; ++i) {
    EXPECT_NE(traversedLeaves.end(), std::find(traversedLeaves.begin(), traversedLeaves.end(), i));
  }
}
