#pragma once
#ifndef MESSMER_CPPUTILS_CRYPTO_SYMMETRIC_ENCRYPTIONKEY_H_
#define MESSMER_CPPUTILS_CRYPTO_SYMMETRIC_ENCRYPTIONKEY_H_

#include <cpp-utils/data/FixedSizeData.h>
#include <memory>
#include <cpp-utils/system/memory.h>
#include "../cryptopp_byte.h"
#include <cpp-utils/random/RandomGenerator.h>

namespace cpputils {

/**
 * Use this to store an encryption key and keep it safe in memory.
 * It will only keep the key in one memory location, even if the EncryptionKey object is copied or moved.
 * This one memory location will be prevented from swapping to disk.
 * Note: This is a best-effort, but not a guarantee. System hibernation might still write the encryption key to the disk.
 * Also, when (de)serializing the config file or calling a crypto library with the encryption key, it isn't guaranteed
 * that there aren't any copies made to different memory regions. However, these other memory regions should be short-lived
 * and therefore much less likely to swap.
 */
template<size_t KeySize>
class EncryptionKey final {
private:
    explicit EncryptionKey(std::shared_ptr<Data> keyData)
        : _keyData(std::move(keyData)) {
        ASSERT(_keyData->size() == KeySize, "Wrong key data size");
    }
    template<size_t OtherKeySize> friend class EncryptionKey;

public:
    EncryptionKey(const EncryptionKey& rhs) = default;
    EncryptionKey(EncryptionKey&& rhs) = default;
    EncryptionKey& operator=(const EncryptionKey& rhs) = default;
    EncryptionKey& operator=(EncryptionKey&& rhs) = default;

    static constexpr size_t BINARY_LENGTH = KeySize;
    static constexpr size_t STRING_LENGTH = 2 * BINARY_LENGTH;

    static EncryptionKey Null() {
        auto data = std::make_shared<Data>(
            KeySize,
            make_unique_ref<UnswappableAllocator>()
        );
        data->FillWithZeroes();
        return EncryptionKey(std::move(data));
    }

    static EncryptionKey FromString(const std::string& keyData) {
        ASSERT(keyData.size() == STRING_LENGTH, "Wrong input size or EncryptionKey::FromString()");

        auto data = std::make_shared<Data>(
            Data::FromString(keyData, make_unique_ref<UnswappableAllocator>())
        );
        ASSERT(data->size() == KeySize, "Wrong input size for EncryptionKey::FromString()");

        return EncryptionKey(std::move(data));
    }

    std::string ToString() const {
        auto result = _keyData->ToString();
        ASSERT(result.size() == STRING_LENGTH, "Wrong string length");
        return result;
    }

    static EncryptionKey CreateKey(RandomGenerator &randomGenerator) {
        EncryptionKey result(std::make_shared<Data>(
            KeySize,
            make_unique_ref<UnswappableAllocator>() // the allocator makes sure key data is never swapped to disk
        ));
        randomGenerator.write(result._keyData->data(), KeySize);
        return result;
    }

    const void *data() const {
        return _keyData->data();
    }

    void *data() {
        return const_cast<void*>(const_cast<const EncryptionKey*>(this)->data());
    }

    // TODO Test take/drop

    template<size_t NumTaken>
    EncryptionKey<NumTaken> take() const {
        static_assert(NumTaken <= KeySize, "Out of bounds");
        auto result = std::make_shared<Data>(NumTaken, make_unique_ref<UnswappableAllocator>());
        std::memcpy(result->data(), _keyData->data(), NumTaken);
        return EncryptionKey<NumTaken>(std::move(result));
    }

    template<size_t NumDropped>
    EncryptionKey<KeySize - NumDropped> drop() const {
        static_assert(NumDropped <= KeySize, "Out of bounds");
        auto result = std::make_shared<Data>(KeySize - NumDropped, make_unique_ref<UnswappableAllocator>());
        std::memcpy(result->data(), _keyData->dataOffset(NumDropped), KeySize - NumDropped);
        return EncryptionKey<KeySize - NumDropped>(std::move(result));
    }

private:
    std::shared_ptr<Data> _keyData;
};

template<size_t KeySize> constexpr size_t EncryptionKey<KeySize>::BINARY_LENGTH;
template<size_t KeySize> constexpr size_t EncryptionKey<KeySize>::STRING_LENGTH;

}

#endif
