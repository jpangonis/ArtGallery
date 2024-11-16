#include "log_utils.h"
#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/secblock.h>
#include <cryptopp/base64.h>

std::string encryptData(const std::string& token, const std::string& key) {
    using namespace CryptoPP;
    std::string encrypted;

    // Set up key and IV (Initialization Vector)
    SecByteBlock keyBytes((const byte*)key.data(), key.size());
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero for simplicity

    try {
        // Encryption using CBC mode and PKCS7 padding
        CBC_Mode<AES>::Encryption encryptor(keyBytes, keyBytes.size(), iv);

        // Encrypt the data with the encryptor
        StringSource ss(token, true,
            new StreamTransformationFilter(encryptor,
                new StringSink(encrypted),
                StreamTransformationFilter::PKCS_PADDING
            )
        );

        // Base64 encode the encrypted binary data for easy transport
        std::string encoded;
        StringSource(encrypted, true, new Base64Encoder(new StringSink(encoded)));
        return encoded;
    }
    catch (const Exception& e) {
        std::cerr << "Encryption error: " << e.what() << std::endl;
        return "";
    }
}

std::string decryptData(const std::string& encryptedToken, const std::string& key) {
    using namespace CryptoPP;
    std::string decrypted;

    // Set up key and IV (Initialization Vector)
    SecByteBlock keyBytes((const byte*)key.data(), key.size());
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero

    try {
        // Decode the Base64 encoded string to get the raw encrypted data
        std::string decoded;
        StringSource(encryptedToken, true, new Base64Decoder(new StringSink(decoded)));

        // Decrypting the data using CBC mode and handling padding
        CBC_Mode<AES>::Decryption decryptor(keyBytes, keyBytes.size(), iv);

        // Decrypt with automatic padding (PKCS7 padding)
        StringSource ss(decoded, true,
            new StreamTransformationFilter(decryptor,
                new StringSink(decrypted),
                StreamTransformationFilter::PKCS_PADDING // Ensure PKCS7 padding is used
            )
        );
    }
    catch (const Exception& e) {
        std::cerr << "Decryption error: " << e.what() << std::endl;
        return "";
    }

    return decrypted;
}


// Verify the provided token by decrypting the stored encrypted token and comparing
bool verifyToken(const std::string& providedToken, const std::string& encryptedToken, const std::string& key) {
    return providedToken == decryptData(encryptedToken, key);
}
