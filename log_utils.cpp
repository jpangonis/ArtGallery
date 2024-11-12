#include "log_utils.h"
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <iostream>
/* TODO encrypt the log files, do we need to encrypt the token?*/

const std::string encryptionKey = "thisisasecretkey"; // Exactly 16-byte key

// Encrypt the token using AES-CBC with PKCS padding
std::string encryptToken(const std::string& token, const std::string& key) {
    using namespace CryptoPP;
    std::string encrypted;

    // Set up key and IV (Initialization Vector)
    SecByteBlock keyBytes((const byte*)key.data(), key.size());
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero for simplicity

    try {
        CBC_Mode<AES>::Encryption encryptor(keyBytes, keyBytes.size(), iv);
        StringSource ss(token, true,
            new StreamTransformationFilter(encryptor,
                new StringSink(encrypted), StreamTransformationFilter::PKCS_PADDING
            )
        );
    }
    catch (const Exception& e) {
        std::cerr << "Encryption error: " << e.what() << std::endl;
    }

    return encrypted;
}

// Decrypt the token using AES-CBC with PKCS padding
std::string decryptToken(const std::string& encryptedToken, const std::string& key) {
    using namespace CryptoPP;
    std::string decrypted;

    // Set up key and IV
    SecByteBlock keyBytes((const byte*)key.data(), key.size());
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero

    try {
        CBC_Mode<AES>::Decryption decryptor(keyBytes, keyBytes.size(), iv);
        StringSource ss(encryptedToken, true,
            new StreamTransformationFilter(decryptor,
                new StringSink(decrypted), StreamTransformationFilter::PKCS_PADDING
            )
        );
    }
    catch (const Exception& e) {
        std::cerr << "Decryption error: " << e.what() << std::endl;
    }

    return decrypted;
}

// Verify the provided token by decrypting the stored encrypted token and comparing
bool verifyToken(const std::string& providedToken, const std::string& encryptedToken, const std::string& key) {
    return providedToken == decryptToken(encryptedToken, key);
}
