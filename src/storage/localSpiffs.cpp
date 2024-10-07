#include "localSpiffs.h"

void localSpiffs::init () {

    if (!SPIFFS.begin ()) {
        BLEM_LOG_ALL ("Card Mount Failed");
        return;
    }

    get_BLEM_data ("/BLEM_data.json");

    // checkListDir (SPIFFS, "/outbox/LOW", 0);


    listDir (SPIFFS, "/", 0);
    // removeDir (SPIFFS, "/mydir");
    // createDir (SPIFFS, "/mydir");
    // deleteFile (SPIFFS, "/hello.txt");
    // writeFile (SPIFFS, "/hello.txt", "Hello ");
    // appendFile (SPIFFS, "/hello.txt", "World!\n");
    // listDir (SPIFFS, "/", 0);
    // readFile (SPIFFS, "/running_data.json");
}

void localSpiffs::get_BLEM_data (const char* path) {
    std::string data = readFileString (SPIFFS, path);
    BLEM_LOG_SERIAL ("%s", data.data ());
    auto _status = deviceStatus::GetInstance ();
    _status->set_json_ies_identity (data.data ());
    DynamicJsonDocument doc (1024);
    deserializeJson (doc, data);
    JsonObject obj     = doc.as<JsonObject> ();
    std::string esn    = obj[String ("esn")];
    std::string aes256 = obj[String ("aes256")];

    _status->set_key_AES256 (aes256.c_str ());
    _status->set_BLE_server (esn.c_str ());

    uint8_t retrieved_key[32];
    _status->get_key_AES256 (retrieved_key);

    printf ("Retrieved key: ");
    for (int i = 0; i < 32; i++) {
        printf ("%02X ", retrieved_key[i]);
    }

    BLEM_LOG_SERIAL ("set_BLE_server: %s", _status->get_BLE_server ().c_str ());
}