#pragma once
#ifndef MESSMER_CRYFS_FILESYSTEM_CRYDEVICE_H_
#define MESSMER_CRYFS_FILESYSTEM_CRYDEVICE_H_

#include <blockstore/interface/BlockStore.h>
#include <blockstore/interface/BlockStore2.h>
#include "../config/CryConfigFile.h"

#include <boost/filesystem.hpp>
#include <fspp/fs_interface/Device.h>

#include "parallelaccessfsblobstore/ParallelAccessFsBlobStore.h"
#include "parallelaccessfsblobstore/DirBlobRef.h"
#include "parallelaccessfsblobstore/FileBlobRef.h"
#include "parallelaccessfsblobstore/SymlinkBlobRef.h"

namespace cryfs {

class CryDevice final: public fspp::Device {
public:
  CryDevice(CryConfigFile config, cpputils::unique_ref<blockstore::BlockStore2> blockStore, uint32_t myClientId);

  void statfs(const boost::filesystem::path &path, struct ::statvfs *fsstat) override;

  cpputils::unique_ref<parallelaccessfsblobstore::FileBlobRef> CreateFileBlob(const blockstore::Key &parent);
  cpputils::unique_ref<parallelaccessfsblobstore::DirBlobRef> CreateDirBlob(const blockstore::Key &parent);
  cpputils::unique_ref<parallelaccessfsblobstore::SymlinkBlobRef> CreateSymlinkBlob(const boost::filesystem::path &target, const blockstore::Key &parent);
  cpputils::unique_ref<parallelaccessfsblobstore::FsBlobRef> LoadBlob(const blockstore::Key &key);
  struct DirBlobWithParent {
      cpputils::unique_ref<parallelaccessfsblobstore::DirBlobRef> blob;
      boost::optional<cpputils::unique_ref<parallelaccessfsblobstore::DirBlobRef>> parent;
  };
  DirBlobWithParent LoadDirBlobWithParent(const boost::filesystem::path &path);
  void RemoveBlob(const blockstore::Key &key);

  void onFsAction(std::function<void()> callback);

  boost::optional<cpputils::unique_ref<fspp::Node>> Load(const boost::filesystem::path &path) override;
  boost::optional<cpputils::unique_ref<fspp::File>> LoadFile(const boost::filesystem::path &path) override;
  boost::optional<cpputils::unique_ref<fspp::Dir>> LoadDir(const boost::filesystem::path &path) override;
  boost::optional<cpputils::unique_ref<fspp::Symlink>> LoadSymlink(const boost::filesystem::path &path) override;

  void callFsActionCallbacks() const;

  uint64_t numBlocks() const;

private:

  cpputils::unique_ref<parallelaccessfsblobstore::ParallelAccessFsBlobStore> _fsBlobStore;

  blockstore::Key _rootKey;
  std::vector<std::function<void()>> _onFsAction;

  blockstore::Key GetOrCreateRootKey(CryConfigFile *config);
  blockstore::Key CreateRootBlobAndReturnKey();
  static cpputils::unique_ref<parallelaccessfsblobstore::ParallelAccessFsBlobStore> CreateFsBlobStore(cpputils::unique_ref<blockstore::BlockStore2> blockStore, CryConfigFile *configFile, uint32_t myClientId);
#ifndef CRYFS_NO_COMPATIBILITY
  static cpputils::unique_ref<fsblobstore::FsBlobStore> MigrateOrCreateFsBlobStore(cpputils::unique_ref<blobstore::BlobStore> blobStore, CryConfigFile *configFile);
#endif
  static cpputils::unique_ref<blobstore::BlobStore> CreateBlobStore(cpputils::unique_ref<blockstore::BlockStore2> blockStore, CryConfigFile *configFile, uint32_t myClientId);
  static cpputils::unique_ref<blockstore::BlockStore2> CreateVersionCountingEncryptedBlockStore(cpputils::unique_ref<blockstore::BlockStore2> blockStore, CryConfigFile *configFile, uint32_t myClientId);
  static cpputils::unique_ref<blockstore::BlockStore2> CreateEncryptedBlockStore(const CryConfig &config, cpputils::unique_ref<blockstore::BlockStore2> baseBlockStore);

  struct BlobWithParent {
      cpputils::unique_ref<parallelaccessfsblobstore::FsBlobRef> blob;
      boost::optional<cpputils::unique_ref<parallelaccessfsblobstore::DirBlobRef>> parent;
  };
  BlobWithParent LoadBlobWithParent(const boost::filesystem::path &path);

  DISALLOW_COPY_AND_ASSIGN(CryDevice);
};

}

#endif
