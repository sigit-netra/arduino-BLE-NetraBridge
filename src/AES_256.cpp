#include "AES_256.h"
void AES_256::checkAndCreateEncryptionKeysFile () {
    if (!SPIFFS.begin (true)) {
        Serial.println ("An Error has occurred while mounting SPIFFS");
        return;
    }

    if (!SPIFFS.exists ("/BLEM_data.json")) {
        // File does not exist, create and write default content
        Serial.println ("/BLEM_data.json does not exist. Creating...");

        StaticJsonDocument<1024> doc;
        doc["esn"] = "70076190";
        doc["aes256"] =
        "2C5F27D94AF5572CB3C135A9EE39A5E5B3A6B329264E2DDD40E45D28B9D03FAD";

        File file = SPIFFS.open ("/BLEM_data.json", FILE_WRITE);
        if (file) {
            serializeJson (doc, file);
            file.close ();
            Serial.println ("File created successfully.");
        } else {
            Serial.println ("Error creating file.");
        }
    } else {
        Serial.println ("/BLEM_data.json exists.");
    }
}

void AES_256::get_aes256 () {
    // Check and create /BLEM_data.json if it doesn't exist
    checkAndCreateEncryptionKeysFile ();
    Serial.println ("");
}

uint16_t AES_256::GF_Common_Crc16 (uint8_t* data, uint16_t length) {
    uint16_t crc     = 0xFFFF;
    uint16_t poly    = 0x8408;
    uint8_t cur_byte = 0x00;
    for (uint16_t i = 0; i < length; i++) {
        cur_byte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & 0x0001) ^ (cur_byte & 0x0001)) {
                crc = (crc >> 1) ^ poly;
            } else {
                crc >>= 1;
            }
            cur_byte >>= 1;
        }
    }
    crc = ~crc;
    crc = (crc << 8) | ((crc >> 8) & 0xff);
    return crc;
}

void AES_256::get_encryption_payload () {
    Serial.println ("*********************");
    auto _status = deviceStatus::GetInstance ();
    uint8_t buffer[100];

    uint8_t retrieved_key[100];
    _status->get_key_AES256 (retrieved_key);

    gen_aes256 (retrieved_key, _status->get_r_msg_sos (), buffer);
    _status->set_e_msg_sos (buffer, 20);
    memset (buffer, 0x00, sizeof (buffer));

    gen_aes256 (retrieved_key, _status->get_r_msg_cancel_sos (), buffer);
    _status->set_e_msg_cancel_sos (buffer, 20);
    memset (buffer, 0x00, sizeof (buffer));

    gen_aes256 (retrieved_key, _status->get_r_msg_tamper (), buffer);
    _status->set_e_msg_tamper (buffer, 20);
    memset (buffer, 0x00, sizeof (buffer));

    Serial.println ("*****Get Config*****");

    gen_aes256 (retrieved_key, _status->get_r_get_cfg (), buffer);
    _status->set_e_ies_cfg (buffer, 20);
    memset (buffer, 0x00, sizeof (buffer));

    Serial.println ("*****set Config*****");

    encrypt_aes256 (retrieved_key, _status->get_r_set_30m (), 32, buffer);
    _status->set_e_set_30m (buffer, 36);
    memset (buffer, 0x00, sizeof (buffer));

    encrypt_aes256 (retrieved_key, _status->get_r_set_60m (), 32, buffer);
    _status->set_e_set_60m (buffer, 36);
    memset (buffer, 0x00, sizeof (buffer));

    // Serial.println ("*********************");
    // decrypt_aes256 (retrieved_key, _status->get_e_set_30m (), buffer);
    // memset (buffer, 0x00, sizeof (buffer));

    // decrypt_aes256 (retrieved_key, _status->get_e_set_60m (), buffer);
    // memset (buffer, 0x00, sizeof (buffer));
}

void AES_256::restart_cmd () {
    delay (3000);
    ESP.restart ();
}

void AES_256::gen_aes256 (uint8_t* key, uint8_t* message, uint8_t* buffNewData) {
    uint8_t buffer[1024]; // Penampung untuk data hasil enkripsi
    uint8_t blockBuffer[cipher->blockSize ()]; // Penampung untuk blok terenkripsi sementara
    uint8_t messageBuffer[cipher->blockSize ()]; // Penampung untuk blok pesan

    // Hitung panjang pesan, jumlah blok penuh, dan sisa karakter < ukuran blok
    int msgLen   = strlen ((char*)message);
    int blocks   = msgLen / cipher->blockSize ();
    int leftOver = msgLen % cipher->blockSize ();

    // Proses setiap blok
    for (int i = 0; i <= blocks; i++) {
        int startingPosition = i * cipher->blockSize ();
        int endingPosition   = startingPosition + cipher->blockSize ();

        // Salin data blok ke messageBuffer, tambahkan padding jika perlu
        for (int k = startingPosition; k < endingPosition; k++) {
            if (i == blocks && leftOver != 0 && k >= startingPosition + leftOver) {
                messageBuffer[k - startingPosition] = 0x00; // Padding
            } else {
                messageBuffer[k - startingPosition] = message[k];
            }
        }

        // Atur kunci dan enkripsi blok
        cipher->setKey (key, cipher->keySize ());
        cipher->encryptBlock (blockBuffer, messageBuffer);

        // Salin blok terenkripsi ke buffer akhir
        for (int m = startingPosition; m < endingPosition; m++) {
            buffer[m] = blockBuffer[m - startingPosition];
        }
    }

    // Hitung panjang total data terenkripsi
    if (leftOver != 0) {
        blocks = blocks + 1;
    }
    int lenlen     = blocks * (int)cipher->blockSize ();
    buffNewData[0] = 0x01; // Tambahkan header atau indikator
    memmove (buffNewData + 1, buffer, lenlen); // Salin data terenkripsi ke buffNewData

    // Hitung CRC dan tambahkan ke buffNewData
    uint16_t crcss          = GF_Common_Crc16 (buffNewData, lenlen + 1);
    buffNewData[lenlen + 1] = 0x1e; // Tambahkan indikator akhir
    buffNewData[lenlen + 2] = (uint8_t) (crcss >> 8) & 0xff; // Tambahkan CRC
    buffNewData[lenlen + 3] = crcss & 0xff;

    // Cetak data terenkripsi untuk debugging
    delay (100);
    for (int i = 0; i < lenlen + 4; i++) {
        Serial.printf ("%02X", buffNewData[i]);
    }
    Serial.println ("");
}

void AES_256::decrypt_aes256 (uint8_t* key, uint8_t* encryptedMessage, uint8_t* decryptedMessage) {
    uint8_t blockBuffer[cipher->blockSize ()];
    uint8_t encryptedBuffer[cipher->blockSize ()];

    // Buffer untuk menyimpan semua blok dalam satu line
    String encryptedLine = "";

    // Hitung panjang pesan terenkripsi
    int encryptedLen = strlen ((char*)encryptedMessage);
    int blocks       = encryptedLen / cipher->blockSize ();

    // Loop over the blocks
    for (int i = 0; i < blocks; i++) {
        int startingPosition = i * cipher->blockSize ();
        int endingPosition   = startingPosition + cipher->blockSize ();

        // Copy encrypted data into temporary buffer
        for (int k = startingPosition; k < endingPosition; k++) {
            encryptedBuffer[k - startingPosition] = encryptedMessage[k];
        }

        // Gabungkan semua blok menjadi satu line heksadesimal
        for (int j = 0; j < cipher->blockSize (); j++) {
            encryptedLine += String (encryptedBuffer[j], HEX); // Tambahkan byte ke string
            if (encryptedBuffer[j] < 16) {
                encryptedLine =
                encryptedLine.substring (0, encryptedLine.length () - 1) +
                "0" + encryptedLine.substring (encryptedLine.length () - 1); // Pastikan ada leading zero
            }
        }

        // Perform decryption on the temp buffer
        crypto_feed_watchdog ();
        cipher->setKey (key, cipher->keySize ());
        cipher->decryptBlock (blockBuffer, encryptedBuffer);

        // Copy the decrypted data into the final buffer
        for (int m = startingPosition; m < endingPosition; m++) {
            decryptedMessage[m] = blockBuffer[m - startingPosition];
        }
    }

    // Tampilkan pesan terenkripsi gabungan dalam satu line
    // Serial.print ("Encrypted Message Line: ");
    // Serial.println (encryptedLine);

    // Null-terminate the decrypted message if it's a string
    decryptedMessage[encryptedLen] = '\0';

    // Tampilkan pesan yang sudah didekripsi
    // Serial.print ("Decrypted Message: ");
    // Serial.println ((char*)decryptedMessage);
}

void AES_256::decrypt_aes256_new(uint8_t* key, uint8_t* encryptedMessage, int messageLength, uint8_t* decryptedMessage) {
    int blockSize = cipher->blockSize();  // Ambil ukuran blok
    int blocks = messageLength / blockSize;  // Hitung jumlah blok
    uint8_t blockBuffer[blockSize];  // Buffer sementara untuk blok didekripsi
    uint8_t encryptedBuffer[blockSize];  // Buffer sementara untuk blok terenkripsi

    // Set key untuk dekripsi
    cipher->setKey(key, cipher->keySize());

    // Proses setiap blok
    for (int i = 0; i < blocks; i++) {
        // Salin blok terenkripsi ke buffer
        memcpy(encryptedBuffer, encryptedMessage + (i * blockSize), blockSize);

        // Dekripsi blok
        cipher->decryptBlock(blockBuffer, encryptedBuffer);

        // Salin blok yang didekripsi ke buffer pesan akhir
        memcpy(decryptedMessage + (i * blockSize), blockBuffer, blockSize);
    }

    // Null-terminate the decrypted message if it's a string (hanya jika pesan berupa string)
    decryptedMessage[messageLength] = '\0';

    // Tampilkan hasil dekripsi
    Serial.print("Decrypted Message: ");
    for (int i = 0; i < messageLength; i++) {
        Serial.printf("%02X", decryptedMessage[i]);
    }
    Serial.println();
}

void AES_256::encrypt_aes256 (uint8_t* key,
                              uint8_t* decryptedMessage,
                              int messageLen,
                              uint8_t* bufferEncryptedMessage) {
    uint8_t blockBuffer[cipher->blockSize ()];   // Buffer for encrypted blocks
    uint8_t messageBuffer[cipher->blockSize ()]; // Buffer for message blocks
    uint8_t encryptedMessage[32] = { 0 }; // Adjusted size based on the actual data length

    int blockSize = cipher->blockSize ();
    int blocks = (messageLen + blockSize - 1) / blockSize; // Calculate the number of blocks
    int leftOver = messageLen % blockSize;

    BLEM_LOG_ALL ("Message length: %d, Blocks: %d, LeftOver: %d\n", messageLen,
                  blocks, leftOver);

    // Process each block
    for (int i = 0; i < blocks; i++) {
        // Clear buffers to ensure no leftover data
        memset (messageBuffer, 0, blockSize);
        memset (blockBuffer, 0, blockSize);

        int startingPosition = i * blockSize;
        int endingPosition   = startingPosition + blockSize;

        // Copy data to messageBuffer and add padding if necessary
        for (int k = 0; k < blockSize; k++) {
            if (startingPosition + k < messageLen) {
                messageBuffer[k] = decryptedMessage[startingPosition + k];
            } else {
                messageBuffer[k] = 0xff; // Padding
            }
        }

        // Set the key and encrypt the block
        cipher->setKey (key, cipher->keySize ());
        cipher->encryptBlock (blockBuffer, messageBuffer);

        // Print blockBuffer for debugging
        Serial.printf ("Block %d Encrypted: ", i);
        for (int j = 0; j < blockSize; j++) {
            Serial.printf ("%02X", blockBuffer[j]);
        }
        Serial.println ("");

        // Copy the encrypted block to the final buffer
        memcpy (&encryptedMessage[startingPosition], blockBuffer, blockSize);
    }

    int totalEncryptedSize = blocks * blockSize;

    // Ensure bufferEncryptedMessage is large enough
    if (bufferEncryptedMessage != nullptr) {
        bufferEncryptedMessage[0] = 0x01; // Add header or indicator
        memmove (bufferEncryptedMessage + 1, encryptedMessage, totalEncryptedSize); // Copy encrypted data

        // Calculate CRC and add to the buffer
        uint16_t crcss = GF_Common_Crc16 (bufferEncryptedMessage, totalEncryptedSize + 1);
        bufferEncryptedMessage[totalEncryptedSize + 1] = 0x1e; // Add end indicator
        bufferEncryptedMessage[totalEncryptedSize + 2] = (uint8_t) (crcss >> 8) & 0xff; // Add CRC
        bufferEncryptedMessage[totalEncryptedSize + 3] = crcss & 0xff;

        // Print encrypted data for debugging
        delay (100);
        for (int i = 0; i < totalEncryptedSize + 4; i++) {
            Serial.printf ("%02X", bufferEncryptedMessage[i]);
        }
        Serial.println ("");
    } else {
        Serial.println ("Error: bufferEncryptedMessage is null.");
    }
}
