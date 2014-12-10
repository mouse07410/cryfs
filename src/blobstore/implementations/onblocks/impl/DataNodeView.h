#pragma once
#ifndef BLOBSTORE_IMPLEMENTATIONS_ONBLOCKS_IMPL_DATANODEVIEW_H_
#define BLOBSTORE_IMPLEMENTATIONS_ONBLOCKS_IMPL_DATANODEVIEW_H_

#include "blockstore/interface/Block.h"
#include "blobstore/implementations/onblocks/BlobStoreOnBlocks.h"

#include "fspp/utils/macros.h"

#include <memory>
#include <cassert>

namespace blobstore {
namespace onblocks {

class DataNodeView {
public:
  DataNodeView(std::unique_ptr<blockstore::Block> block): _block(std::move(block)) {
    assert(_block->size() == BLOCKSIZE_BYTES);
  }
  virtual ~DataNodeView() {}

  DataNodeView(DataNodeView &&rhs) = default;

  //Total size of the header
  static constexpr unsigned int HEADERSIZE_BYTES = 8;
  //Where in the header is the magic number
  static constexpr unsigned int MAGICNUMBER_OFFSET_BYTES = 0;
  //Where in the header is the size field (for inner nodes: number of children, for leafs: content data size)
  static constexpr unsigned int SIZE_OFFSET_BYTES = 4;

  //How big is one blob in total (header + data)
  static constexpr unsigned int BLOCKSIZE_BYTES = BlobStoreOnBlocks::BLOCKSIZE;
  //How much space is there for data
  static constexpr unsigned int DATASIZE_BYTES = BLOCKSIZE_BYTES - HEADERSIZE_BYTES;

  static constexpr unsigned char magicNumberNodeWithChildren = 0x01;
  static constexpr unsigned char magicNumberLeaf = 0x02;
  static constexpr unsigned char magicNumberRootWithChildren = 0x03;
  static constexpr unsigned char magicNumberRootLeaf = 0x04;

  const uint8_t *MagicNumber() const {
    return GetOffset<MAGICNUMBER_OFFSET_BYTES, uint8_t>();
  }

  uint8_t *MagicNumber() {
    return const_cast<uint8_t*>(const_cast<const DataNodeView*>(this)->MagicNumber());
  }

  const uint32_t *Size() const {
    return GetOffset<SIZE_OFFSET_BYTES, uint32_t>();
  }

  uint32_t *Size() {
    return const_cast<uint32_t*>(const_cast<const DataNodeView*>(this)->Size());
  }

  template<typename Entry>
  const Entry *DataBegin() const {
    return GetOffset<HEADERSIZE_BYTES, Entry>();
  }

  template<typename Entry>
  Entry *DataBegin() {
    return const_cast<Entry*>(const_cast<const DataNodeView*>(this)->DataBegin<Entry>());
  }

  template<typename Entry>
  const Entry *DataEnd() const {
    constexpr unsigned int NUM_ENTRIES = DATASIZE_BYTES / sizeof(Entry);
    return DataBegin<Entry>() + NUM_ENTRIES;
  }

  template<typename Entry>
  Entry *DataEnd() {
    return const_cast<Entry*>(const_cast<const DataNodeView*>(this)->DataEnd<Entry>());
  }

private:
  template<int offset, class Type>
  const Type *GetOffset() const {
    return (Type*)(((const int8_t*)_block->data())+offset);
  }

  std::unique_ptr<blockstore::Block> _block;

  DISALLOW_COPY_AND_ASSIGN(DataNodeView);

};

}
}

#endif