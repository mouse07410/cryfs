#pragma once
#ifndef BLOCKSTORE_IMPLEMENTATIONS_ONDISK_DATA_H_
#define BLOCKSTORE_IMPLEMENTATIONS_ONDISK_DATA_H_

#include <cstdlib>
#include <boost/filesystem/path.hpp>
#include <messmer/cpp-utils/macros.h>
#include <memory>

namespace blockstore {

class Data {
public:
  explicit Data(size_t size);
  Data(Data &&rhs); // move constructor
  virtual ~Data();

  Data copy() const;

  void *data();
  const void *data() const;

  size_t size() const;

  Data &FillWithZeroes();

  void StoreToFile(const boost::filesystem::path &filepath) const;
  static Data LoadFromFile(const boost::filesystem::path &filepath);

private:
  size_t _size;
  void *_data;

  static void _assertFileExists(const std::ifstream &file, const boost::filesystem::path &filepath);
  static size_t _getStreamSize(std::istream &stream);
  void _readFromStream(std::istream &stream);

  DISALLOW_COPY_AND_ASSIGN(Data);
};

//TODO Test operator==
bool operator==(const Data &lhs, const Data &rhs);

}

#endif
