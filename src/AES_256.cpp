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
    // Serial.println (aes256);
    ble_server.BLE_server = esn.c_str ();

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
        ble_server.key_AES256[i] = myuint[i];
    }

    for (uint8_t i = 0; i < 32; i++) {
        Serial.print (ble_server.key_AES256[i], HEX);
        // server.AES256[i] = myuint[i];
    }
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

void AES_256::test_AES256 () {
    Serial.println ("*********************");
    Serial.println ("");
    gen_aes256 (ble_server.key_AES256, ble_raw_msg.msg_cancel_sos, ble_msg.msg_cancel_sos);
    gen_aes256 (ble_server.key_AES256, ble_raw_msg.msg_sos, ble_msg.msg_sos);
    gen_aes256 (ble_server.key_AES256, ble_raw_msg.msg_tamper, ble_msg.msg_tamper);

    // delay (100);
    // for (int i = 0; i < 20; i++) {
    //     Serial.printf ("%02X", ble_msg.msg_cancel_sos[i], HEX);
    // }
    Serial.println ("");

    Serial.println ("*********************");
}

void AES_256::restart_cmd () {
    delay (3000);
    ESP.restart ();
}

void AES_256::gen_aes256 (uint8_t* key, uint8_t* message, uint8_t* buffNewData) {
    uint8_t buffer[1024];
    uint8_t blockBuffer[cipher->blockSize ()];
    uint8_t messageBuffer[cipher->blockSize ()];

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
        cipher->setKey (ble_server.key_AES256, cipher->keySize ());
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
    // for (int i = 0; i < blocks * (int)cipher->blockSize (); i++) {
    //     Serial.printf ("%02X", buffer[i], HEX);
    // }
    // Serial.println ();
    int lenlen = blocks * (int)cipher->blockSize ();
    // uint8_t buffNewData[10];
    // Serial.println (lenlen);
    // buffNewData    = (uint8_t*)malloc (lenlen + 1);
    buffNewData[0] = 0x01;
    memmove (buffNewData + 1, buffer, lenlen);
    uint16_t crcss          = GF_Common_Crc16 (buffNewData, lenlen + 1);
    buffNewData[lenlen + 1] = 0x1e;


    buffNewData[lenlen + 2] = (uint8_t) (crcss >> 8) & 0xff;
    buffNewData[lenlen + 3] = crcss;


    // Serial.println (crcss, HEX);


    // std::string str2 = "a";
    // /* Copies contents of str2 to str1 */
    // str2.insert(1, buffer);
    delay (100);
    for (int i = 0; i < lenlen + 4; i++) {
        Serial.printf ("%02X", buffNewData[i], HEX);
    }
    Serial.println ();
}