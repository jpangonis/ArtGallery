#include "log_utils.h"
#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/secblock.h>
#include <cryptopp/base64.h>
#include <fstream>
#include <unordered_map>

std::string encryptData(const std::string& token, const std::string& key) {
    using namespace CryptoPP;
    std::string encrypted;

    // Set up key and IV (Initialization Vector)
    SecByteBlock keyBytes((const byte*)key.data(), std::min(key.size(), static_cast<size_t>(AES::DEFAULT_KEYLENGTH)));
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero for simplicity

    try {
        // Encryption using CBC mode and PKCS7 padding
        CBC_Mode<AES>::Encryption encryptor;
        encryptor.SetKeyWithIV(keyBytes, keyBytes.size(), iv);

        // Encrypt the data
        std::string cipherText;
        StringSource ss(token, true,
            new StreamTransformationFilter(encryptor,
                new StringSink(cipherText),
                StreamTransformationFilter::PKCS_PADDING
            )
        );

        // Base64 encode the encrypted binary data
        StringSource ss2(cipherText, true, new Base64Encoder(new StringSink(encrypted), false)); // `false` avoids line breaks
    }
    catch (const Exception& e) {
        std::cerr << "Encryption error: " << e.what() << std::endl;
        return "";
    }

    return encrypted;
}

std::string decryptData(const std::string& encryptedToken, const std::string& key) {
    using namespace CryptoPP;
    std::string decrypted;

    // Set up key and IV (Initialization Vector)
    SecByteBlock keyBytes((const byte*)key.data(), std::min(key.size(), static_cast<size_t>(AES::DEFAULT_KEYLENGTH)));
    byte iv[AES::BLOCKSIZE] = { 0 }; // IV initialized to zero

    try {
        // Decode the Base64 encoded string
        std::string decoded;
        StringSource ss1(encryptedToken, true, new Base64Decoder(new StringSink(decoded)));

        // Decrypting the data using CBC mode and PKCS7 padding
        CBC_Mode<AES>::Decryption decryptor;
        decryptor.SetKeyWithIV(keyBytes, keyBytes.size(), iv);

        // Decrypt the data
        StringSource ss2(decoded, true,
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

std::unordered_map<std::string, std::string> loadEnv(const std::string& filename) {
    std::unordered_map<std::string, std::string> envVars;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open .env file");
    }

    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            envVars[key] = value;
        }
    }
    file.close();
    return envVars;
}

