#include "AES_256.h"

void AES_256::get_aes256 () {
    // Initialize SPIFFS
    if (!SPIFFS.begin (true)) {
        Serial.println ("An Error has occurred while mounting SPIFFS");
        return;
    }

    listDir (SPIFFS, "/", 0);
    // writeFile (SPIFFS, "/hello.txt", "Hello ");
    // appendFile (SPIFFS, "/hello.txt", "World!\r\n");
    // readFile (SPIFFS, "/hello.txt");
    // renameFile (SPIFFS, "/hello.txt", "/foo.txt");
    // readFile (SPIFFS, "/foo.txt");
    // deleteFile (SPIFFS, "/foo.txt");
    // testFileIO (SPIFFS, "/test.txt");
    // deleteFile (SPIFFS, "/test.txt");
    // Serial.println ("Test complete");

    std::string data = readFileString (SPIFFS, "/encryption_keys.json");
    // CUBE_LOG_SERIAL ("%s", data.data ());
    DynamicJsonDocument doc (1024);
    deserializeJson (doc, data);
    JsonObject obj = doc.as<JsonObject> ();
    String esn     = obj[String ("esn")];
    String aes256  = obj[String ("aes256")];

    Serial.println (esn);
    Serial.println (aes256);
    server.BLE_server = esn.c_str ();

    auto status = Status::GetInstance ();
    status->set_BLE_server (esn.c_str ());

    char mystring[aes256.length () + 2];
    aes256.toCharArray (mystring, aes256.length () + 2);
    uint8_t myuint[100];
    for (uint8_t i = 0; i < 32; i++) {
        myuint[i] = 0;
        for (uint8_t j = 0; j < 2; j++) {
            char firstchar = mystring[(i * 2) + j];
            // printf("myuint[%d] = %3d mystring[%d+%d] = %c ", i, myuint[i], i, j, mystring[(i*2)+j]);
            if (firstchar >= '0' && firstchar <= '9') {
                // Serial.print("Number");
                myuint[i] = myuint[i] * 16 + firstchar - '0';
            } else if (firstchar >= 'A' && firstchar <= 'F') {
                // Serial.print("LETTER");
                myuint[i] = myuint[i] * 16 + firstchar - 'A' + 10;
            } else if (firstchar >= 'a' && firstchar <= 'f') {
                // Serial.print("letter");
                myuint[i] = myuint[i] * 16 + firstchar - 'a' + 10;
            } else {
                // error
                Serial.println ("NOOOO");
            }
            // printf(" myuint[%d] = %3d\n", i, myuint[i]);
        }
    }

    for (uint8_t i = 0; i < 32; i++) {
        // Serial.println (myuint[i], HEX);
        server.key_AES256[i] = myuint[i];
    }

    for (uint8_t i = 0; i < 32; i++) {
        Serial.print (server.key_AES256[i], HEX);
        // server.AES256[i] = myuint[i];
    }
    Serial.println ("");

    Serial.println ("Ending");
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

void AES_256::test_AES256 () {

    Serial.println ("testtstst");

    uint8_t buffer[1024];
    uint8_t blockBuffer[cipher->blockSize ()];
    uint8_t messageBuffer[cipher->blockSize ()];
    uint8_t key[33] = { 0x66, 0x0e, 0xab, 0x1c, 0x68, 0xe0, 0x9a, 0xb5,
                        0xe6, 0x84, 0x6d, 0x72, 0x59, 0x8f, 0x18, 0x41,
                        0xe5, 0x60, 0x1f, 0xaa, 0x6c, 0x1f, 0xa1, 0x18,
                        0x62, 0xf1, 0x90, 0xb1, 0xec, 0x95, 0x67, 0x76 }; // 32 Byte key
    // char message[1024] = {0x3b, 0x0e, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14}; // message of up to 1024 in length
    char message[1024] = { 0x24, 0x07, 0x09, 0x35, 0xeb, 0x6b, 0x68, 0x2b,
                           0x9b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // message of up to 1024 in length


    // Calculate the total length, number of full blocks, and leftover characters < the size of a block.
    int msgLen   = strlen ((char*)message);
    int blocks   = msgLen / 16;
    int leftOver = msgLen % 16;


    // Loop over the blocks
    for (int i = 0; i <= blocks; i++) {
        int startingPosition = i * cipher->blockSize ();
        int endingPosition   = startingPosition + cipher->blockSize ();


        // Loop over the data in i block and push it into a temp buffer
        for (int k = startingPosition; k < endingPosition; k++) {


            // if we have leftover data, we need to control it by padding.
            if (i == blocks && leftOver != 0 && k >= startingPosition + leftOver) {
                messageBuffer[k - startingPosition] = 0x00;
            } else {
                messageBuffer[k - startingPosition] = message[k];
            }
        }


        // Perform crypto on the temp buffer and push it to another temp buffer
        crypto_feed_watchdog ();
        cipher->setKey (server.key_AES256, cipher->keySize ());
        cipher->encryptBlock (blockBuffer, messageBuffer);


        // push the temp buffer to the final buffer
        for (int m = startingPosition; m < endingPosition; m++) {
            buffer[m] = blockBuffer[m - startingPosition];
        }
    }


    // If we have dangling data increment blocks
    if (leftOver != 0) {
        blocks = blocks + 1;
    }
    // char csrc[100] = {0x01};
    // memcpy(buffer + 1, buffer, sizeof(buffer));
    // buffer[0] = 0x01;
    // Print the encrypted hex
    for (int i = 0; i < blocks * (int)cipher->blockSize (); i++) {
        Serial.printf ("%02X", buffer[i], HEX);
    }
    Serial.println ();
    int lenlen = blocks * (int)cipher->blockSize ();
    Serial.println (lenlen);
    uint8_t* buffNewData = (uint8_t*)malloc (lenlen + 1);
    buffNewData[0]       = 0x01;
    memmove (buffNewData + 1, buffer, lenlen);
    uint16_t crcss          = GF_Common_Crc16 (buffNewData, lenlen + 1);
    buffNewData[lenlen + 1] = 0x1e;


    buffNewData[lenlen + 2] = (uint8_t) (crcss >> 8) & 0xff;
    buffNewData[lenlen + 3] = crcss;


    Serial.println (crcss, HEX);


    // std::string str2 = "a";
    // /* Copies contents of str2 to str1 */
    // str2.insert(1, buffer);
    delay (100);
    for (int i = 0; i < lenlen + 4; i++) {
        Serial.printf ("%02X", buffNewData[i], HEX);
    }
    Serial.println ();
}