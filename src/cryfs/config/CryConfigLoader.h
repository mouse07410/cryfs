#pragma once
#ifndef MESSMER_CRYFS_SRC_CONFIG_CRYCONFIGLOADER_H_
#define MESSMER_CRYFS_SRC_CONFIG_CRYCONFIGLOADER_H_

#include <cpp-utils/pointer/unique_ref.h>
#include <boost/filesystem.hpp>
#include "CryConfigFile.h"
#include "CryCipher.h"
#include "CryConfigCreator.h"
#include <cpp-utils/crypto/kdf/Scrypt.h>

namespace cryfs {

class CryConfigLoader final {
public:
  CryConfigLoader(std::shared_ptr<cpputils::Console> console, cpputils::RandomGenerator &keyGenerator, LocalStateDir localStateDir, const cpputils::SCryptSettings &scryptSettings, std::function<std::string()> askPasswordForExistingFilesystem, std::function<std::string()> askPasswordForNewFilesystem, const boost::optional<std::string> &cipherFromCommandLine, const boost::optional<uint32_t> &blocksizeBytesFromCommandLine, const boost::optional<bool> &missingBlockIsIntegrityViolationFromCommandLine);
  CryConfigLoader(CryConfigLoader &&rhs) = default;

  struct ConfigLoadResult {
      CryConfigFile configFile;
      uint32_t myClientId;
  };

  boost::optional<ConfigLoadResult> loadOrCreate(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem);

private:
    boost::optional<ConfigLoadResult> _loadConfig(boost::filesystem::path filename, bool allowFilesystemUpgrade, bool allowReplacedFilesystem);
    ConfigLoadResult _createConfig(boost::filesystem::path filename, bool allowReplacedFilesystem);
    void _checkVersion(const CryConfig &config, bool allowFilesystemUpgrade);
    void _checkCipher(const CryConfig &config) const;
    void _checkMissingBlocksAreIntegrityViolations(CryConfigFile *configFile, uint32_t myClientId);

    std::shared_ptr<cpputils::Console> _console;
    CryConfigCreator _creator;
    cpputils::SCryptSettings _scryptSettings;
    std::function<std::string()> _askPasswordForExistingFilesystem;
    std::function<std::string()> _askPasswordForNewFilesystem;
    boost::optional<std::string> _cipherFromCommandLine;
    boost::optional<uint32_t> _blocksizeBytesFromCommandLine;
    boost::optional<bool> _missingBlockIsIntegrityViolationFromCommandLine;
    LocalStateDir _localStateDir;

    DISALLOW_COPY_AND_ASSIGN(CryConfigLoader);
};

}

#endif
