#include "BLE_handler.h"
auto _status = deviceStatus::GetInstance ();

BLE_handler::BLE_handler () {}

void BLE_handler::start () {
    xTaskCreatePinnedToCore (HandlerWrapper, "BLE_handler", 25 * 1024, this, 24, NULL, 1);
    xTaskCreatePinnedToCore (HandlerWrapper_parser, "BLE_handler_parser",
                             10 * 1024, this, 23, NULL, 1);
}

bool BLE_handler::initialize () { return 1; }


bool send_SOS        = false;
bool send_CANCEL_SOS = false;
bool send_tamper     = false;

bool return_send_SOS        = false;
bool return_send_CANCEL_SOS = false;
bool return_send_tamper     = false;
bool return_GF_MO_CHAR1     = false;
bool return_GF_MO_CHAR2     = false;
bool return_GF_MO_ACT       = false;
bool get_config             = false;


unsigned long previousMillis = 0;
const long interval          = 600000;

const long interval_BLE_connection          = 120000;
unsigned long previousMillis_BLE_connection = 0;

// BLE Information
// The remote service we wish to connect to.
static BLEUUID serviceUUID ("6ecb2400-dc4c-40cc-a6e0-81e0dbda54e5");
// The characteristic of the remote service we are interested in.
static BLEUUID UUID_CHAR_GF_MT_CHAR1 ("6ecb2401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MT_CHAR2 ("6ecb2402-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MT_ACT ("6ecb3401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_CHAR1 ("6ecb2481-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_CHAR2 ("6ecb2482-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_ACT ("6ecb3481-dc4c-40cc-a6e0-81e0dbda54e5");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan    = false;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MT_CHAR1;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MT_CHAR2;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MT_ACT;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MO_CHAR1;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MO_CHAR2;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MO_ACT;
static BLEAdvertisedDevice* myDevice;
BLEScan* pBLEScan;

uint8_t ble_ack_data[100];
uint8_t ble_ack_data_cnt = 0;

String toHexString (uint8_t* data, size_t length) {
    String hexString = "";
    for (size_t i = 0; i < length; i++) {
        if (data[i] < 16) {
            hexString += "0";
        }
        hexString += String (data[i], HEX);
    }
    hexString.toUpperCase ();
    return hexString;
}
// Function to convert a hex character to an integer
uint8_t hexCharToInt (char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}
static void notifyCallbackGF_MO_CHAR1 (BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                       uint8_t* pData,
                                       size_t length,
                                       bool isNotify) {
    BLEM_LOG_ALL ("Notify callback for notifyCallbackGF_MO_CHAR1 : ");
    BLEM_LOG_ALL ("%s", pBLERemoteCharacteristic->getUUID ().toString ().c_str ());
    BLEM_LOG_ALL (" of data length ");
    BLEM_LOG_ALL ("%d", length);

    String hexString = toHexString (pData, length);
    BLEM_LOG_ALL ("data: ");
    BLEM_LOG_ALL ("%s", hexString.c_str ());

    return_GF_MO_CHAR1 = true;
    auto _status       = deviceStatus::GetInstance ();
    _status->set_ble_ack_data (ble_ack_data, 20);
    memcpy (ble_ack_data, pData, 20);

    // _status->set_sos_status (0);
    if (send_CANCEL_SOS && send_SOS) {
        send_CANCEL_SOS = false;
        send_SOS        = false;
    }
    // _status->set_ble_ack_status (1);
    // LL_esp32.blink_led (500, O_LED_STATUS);
}

static void notifyCallbackGF_MO_CHAR2 (BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                       uint8_t* pData,
                                       size_t length,
                                       bool isNotify) {
    BLEM_LOG_ALL ("Notify callback for notifyCallbackGF_MO_CHAR2 : ");
    BLEM_LOG_ALL ("%s", pBLERemoteCharacteristic->getUUID ().toString ().c_str ());
    BLEM_LOG_ALL (" of data length ");
    BLEM_LOG_ALL ("%d", length);

    String hexString = toHexString (pData, length);
    BLEM_LOG_ALL ("data: ");
    BLEM_LOG_ALL ("%s", hexString.c_str ());

    return_GF_MO_CHAR2 = true;
    auto _status       = deviceStatus::GetInstance ();
    memcpy (ble_ack_data + 20, pData, 20);


    // _status->set_ble_ack_status (1);
}

static void notifyCallbackGF_MO_ACT (BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                     uint8_t* pData,
                                     size_t length,
                                     bool isNotify) {
    BLEM_LOG_ALL ("Notify callback for characteristic ");
    BLEM_LOG_ALL ("%s", pBLERemoteCharacteristic->getUUID ().toString ().c_str ());
    BLEM_LOG_ALL (" of data length ");
    BLEM_LOG_ALL ("%d", pData[0]);
    ble_ack_data_cnt = pData[0];
    String hexString = toHexString (pData, length);
    BLEM_LOG_ALL ("data: ");
    BLEM_LOG_ALL ("%s", hexString.c_str ());
    // for (int i = 0; i < ble_ack_data_cnt; ++i) {
    //     Serial.println (ble_ack_data[i], HEX);
    // }

    return_GF_MO_ACT = true;
    auto _status     = deviceStatus::GetInstance ();

    _status->set_ble_ack_data (ble_ack_data, ble_ack_data_cnt);

    if (_status->get_sos_status () == 100) {
        _status->set_sos_status (0);
    }
    if (send_CANCEL_SOS && send_SOS) {
        send_CANCEL_SOS = false;
        send_SOS        = false;
    }
    _status->set_ble_ack_status (1);
    // LL_esp32.blink_led (500, O_LED_STATUS);


    if (_status->get_ble_ack_status () == 1 || return_GF_MO_ACT) {
        int startIndex = 1;                    // Byte ke-1 (0-indexed)
        int endIndex   = ble_ack_data_cnt - 3; // 3 byte sebelum terakhir

        int length = endIndex - startIndex;
        uint8_t result[length];

        for (int i = 0; i < length; i++) {
            result[i] = ble_ack_data[startIndex + i];
        }
        // Print the result
        Serial.println ("Extracted data:");
        for (int i = 0; i < length; i++) {
            Serial.printf ("%02X", result[i]);
        }
        Serial.println ("");
        uint8_t decryptedData[34]; // Buffer untuk data terdekripsi
        uint8_t retrieved_key[32];
        _status->get_key_AES256 (retrieved_key);
        AES_256 _aes256;

        _aes256.decrypt_aes256 (retrieved_key, result, decryptedData);

        if (decryptedData[0] == 0x48) {
            // Tampilkan hasil dekripsi
            Serial.println ("Decrypted data:");
            for (int i = 0; i < sizeof (decryptedData); i++) {
                Serial.printf ("%02X", decryptedData[i]);
            }
            Serial.println ("");
            // Extract fields according to the format
            uint8_t type          = decryptedData[0];
            uint8_t messageLength = decryptedData[1];
            uint32_t timestamp = (decryptedData[2] << 24) | (decryptedData[3] << 16) |
                                 (decryptedData[4] << 8) | decryptedData[5];
            uint16_t primaryInterval1 = (decryptedData[6] << 8) | decryptedData[7];
            uint16_t primaryInterval2 = (decryptedData[8] << 8) | decryptedData[9];
            uint16_t primaryInterval3 = (decryptedData[10] << 8) | decryptedData[11];
            uint16_t primaryInterval4 = (decryptedData[12] << 8) | decryptedData[13];
            uint16_t primaryInterval5 = (decryptedData[14] << 8) | decryptedData[15];
            uint16_t primaryInterval6 = (decryptedData[16] << 8) | decryptedData[17];
            uint16_t primaryInterval7 = (decryptedData[18] << 8) | decryptedData[19];
            uint16_t primaryInterval8 = (decryptedData[20] << 8) | decryptedData[21];

            uint8_t engineeringMessageInterval = decryptedData[22];
            uint16_t intervalInHours           = engineeringMessageInterval * 6;

            uint8_t vbmrMotionConfig        = decryptedData[23];
            uint8_t vbmrMotionMode          = (vbmrMotionConfig >> 6) & 0x03;
            uint8_t inMotionIntervalMinutes = vbmrMotionConfig & 0x3F;

            uint8_t mailboxCheckInterval = decryptedData[24];
            bool onlyCheckOnTX           = mailboxCheckInterval & 0x80;
            uint8_t mailboxPollInterval  = mailboxCheckInterval & 0x7F;

            uint8_t countMsgsAfterDepletion     = decryptedData[26];
            uint8_t hoursBetweenPositionReports = decryptedData[27];

            uint8_t gpsConfigByte29  = decryptedData[28];
            bool gbmrEnabled         = gpsConfigByte29 & 0x80;
            uint8_t gpsCheckInterval = gpsConfigByte29 & 0x7F;
            uint8_t homeRatio        = decryptedData[29];
            uint8_t awayRatio        = decryptedData[30];

            uint8_t advanced          = decryptedData[31];
            uint8_t startStopConfig   = (advanced >> 5) & 0x07;
            uint8_t iridiumRetryCount = (advanced >> 2) & 0x03;
            bool enableVBMRDuringPowerSave = advanced & 0x01; // Pembuatan JSON Object
            // StaticJsonDocument<512> doc; // Gunakan DynamicJsonDocument dengan ukuran yang cukup besar
            DynamicJsonDocument doc (512); // Tentukan ukuran yang sesuai dengan kebutuhan Anda

            // Menambahkan data ke JSON
            doc["type"]          = type;
            doc["messageLength"] = messageLength;
            doc["timestamp"]     = timestamp;

            // Menambahkan primary intervals sebagai array
            JsonArray primaryIntervals =
            doc.createNestedArray ("primaryIntervals");
            primaryIntervals.add (primaryInterval1);
            primaryIntervals.add (primaryInterval2);
            primaryIntervals.add (primaryInterval3);
            primaryIntervals.add (primaryInterval4);
            primaryIntervals.add (primaryInterval5);
            primaryIntervals.add (primaryInterval6);
            primaryIntervals.add (primaryInterval7);
            primaryIntervals.add (primaryInterval8);

            // Menambahkan engineering message interval
            JsonObject engineeringMessageIntervalJson =
            doc.createNestedObject ("engineeringMessageInterval");
            engineeringMessageIntervalJson["intervalInHours"] = intervalInHours;

            // Menambahkan vbmrMotionConfig
            JsonObject vbmrMotionConfigJson =
            doc.createNestedObject ("vbmrMotionConfig");
            vbmrMotionConfigJson["mode"]                    = vbmrMotionMode;
            vbmrMotionConfigJson["inMotionIntervalMinutes"] = inMotionIntervalMinutes;

            // Menambahkan mailboxCheckInterval
            JsonObject mailboxCheckIntervalJson =
            doc.createNestedObject ("mailboxCheckInterval");
            mailboxCheckIntervalJson["onlyCheckOnTX"]       = onlyCheckOnTX;
            mailboxCheckIntervalJson["pollIntervalInHours"] = mailboxPollInterval;

            // Menambahkan gpsBasedMotionReportingConfig
            JsonObject gpsBasedMotionReportingConfigJson =
            doc.createNestedObject ("gpsBasedMotionReportingConfig");
            gpsBasedMotionReportingConfigJson["enabled"] = gbmrEnabled;
            gpsBasedMotionReportingConfigJson["checkIntervalInMinutes"] =
            gpsCheckInterval * 5;
            gpsBasedMotionReportingConfigJson["homeRatio"] = homeRatio;
            gpsBasedMotionReportingConfigJson["awayRatio"] = awayRatio;

            // Menambahkan advanced
            JsonObject advancedJson = doc.createNestedObject ("advanced");
            advancedJson["startStopConfig"]           = startStopConfig;
            advancedJson["iridiumRetryCount"]         = iridiumRetryCount;
            advancedJson["enableVBMRDuringPowerSave"] = enableVBMRDuringPowerSave;

            // Serialisasi JSON ke string (untuk dikirim atau disimpan)
            String output;
            serializeJson (doc, output);

            // Menampilkan hasil JSON
            Serial.println (output);

            _status->set_json_ies_cfg (output.c_str ());

            // BLEM_LOG_ALL ("_msg: %s", output.c_str ());
        } else if (decryptedData[0] == 0x23) {
            Serial.println ("Decrypted data:");
            for (int i = 0; i < sizeof (decryptedData); i++) {
                Serial.printf ("%02X", decryptedData[i]);
            }
            Serial.println ("");
            // Extract fields according to the format
            uint8_t type          = decryptedData[0];
            uint8_t messageLength = decryptedData[1];
            uint8_t value         = decryptedData[2];
            uint8_t result        = decryptedData[3];
            if (value == 0x2a && result == 0x00) {
                _status->set_act_gen_config (100);
            } else {
                _status->set_act_gen_config (99);
            }

            if (value == 0x10 && result == 0x00) {
                _status->set_act_user_msg (100);
            }

            if (value == 0x24 && result == 0x00) {
                _status->set_act_user_location_msg (100);
            }
        }
        // _status->set_ble_ack_status (0);
        // return_GF_MO_ACT = false;
    }
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect (BLEClient* pclient) {
        // Serial.println ("on Connected");
        // _status->set_ble_status (1);
    }

    void onDisconnect (BLEClient* pclient) {
        connected = false;
        _status->set_ble_status (0);
        BLEM_LOG_ALL ("onDisconnect");
        delay (2000);
        doScan = true;
    }
};

bool connectToServer () {
    BLEM_LOG_ALL ("Forming a connection to ");
    BLEM_LOG_ALL ("%s", myDevice->getAddress ().toString ().c_str ());


    BLEClient* pClient = BLEDevice::createClient ();
    BLEM_LOG_ALL (" - Created client");


    pClient->setClientCallbacks (new MyClientCallback ());


    // Connect to the remove BLE Server.
    pClient->connect (myDevice); // if you pass BLEAdvertisedDevice instead of
                                 // address, it will be recognized type of peer
                                 // device address (public or private)
    BLEM_LOG_ALL (" - Connected to server");
    pClient->setMTU (517); // set client to request maximum MTU from server
                           // (default is 23 otherwise)


    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService (serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.print ("Failed to find our service UUID: ");
        Serial.println (serviceUUID.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }
    BLEM_LOG_ALL (" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE
    // server.
    pRemoteCharacteristic_GF_MT_CHAR1 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MT_CHAR1);
    pRemoteCharacteristic_GF_MT_CHAR2 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MT_CHAR2);
    pRemoteCharacteristic_GF_MT_ACT =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MT_ACT);
    pRemoteCharacteristic_GF_MO_CHAR1 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MO_CHAR1);
    pRemoteCharacteristic_GF_MO_CHAR2 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MO_CHAR2);
    pRemoteCharacteristic_GF_MO_ACT =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MO_ACT);

    if (pRemoteCharacteristic_GF_MT_CHAR1 == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MT_CHAR1 UUID: ");
        Serial.println (UUID_CHAR_GF_MT_CHAR1.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MT_CHAR2 == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MT_CHAR2 UUID: ");
        Serial.println (UUID_CHAR_GF_MT_CHAR2.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MT_ACT == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MT_ACT UUID: ");
        Serial.println (UUID_CHAR_GF_MT_ACT.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MO_CHAR1 == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MO_CHAR1 UUID: ");
        Serial.println (UUID_CHAR_GF_MO_CHAR1.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MO_CHAR2 == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MO_CHAR2 UUID: ");
        Serial.println (UUID_CHAR_GF_MO_CHAR2.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MO_ACT == nullptr) {
        BLEM_LOG_ALL (
        "Failed to find our pRemoteCharacteristic_GF_MO_ACT UUID: ");
        Serial.println (UUID_CHAR_GF_MO_ACT.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }
    BLEM_LOG_ALL (" - Found our characteristic");


    // Read the value of the characteristic.
    if (pRemoteCharacteristic_GF_MT_CHAR1->canRead ()) {
        std::string value = pRemoteCharacteristic_GF_MT_CHAR1->readValue ();
        Serial.print ("The pRemoteCharacteristic_GF_MT_CHAR1 value was: ");
        Serial.println (value.c_str ());
    }
    // Read the value of the characteristic.
    if (pRemoteCharacteristic_GF_MT_CHAR2->canRead ()) {
        std::string value = pRemoteCharacteristic_GF_MT_CHAR2->readValue ();
        Serial.print ("The pRemoteCharacteristic_GF_MT_CHAR2 value was: ");
        Serial.println (value.c_str ());
    }


    if (pRemoteCharacteristic_GF_MO_CHAR1->canNotify ())
        pRemoteCharacteristic_GF_MO_CHAR1->registerForNotify (notifyCallbackGF_MO_CHAR1);

    if (pRemoteCharacteristic_GF_MO_CHAR2->canNotify ())
        pRemoteCharacteristic_GF_MO_CHAR2->registerForNotify (notifyCallbackGF_MO_CHAR2);

    if (pRemoteCharacteristic_GF_MO_ACT->canNotify ())
        pRemoteCharacteristic_GF_MO_ACT->registerForNotify (notifyCallbackGF_MO_ACT);

    connected = true;
    // _status->set_ble_status (1);

    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we
 * are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    /**
     * Called for each advertising BLE server.
     */
    void onResult (BLEAdvertisedDevice advertisedDevice) {
        // Serial.print ("BLE Advertised Device found: ");
        // Serial.println (advertisedDevice.toString ().c_str ());

        auto _status = deviceStatus::GetInstance ();

        if (advertisedDevice.getName () == _status->get_BLE_server () /*bleServerName*/) {
            BLEDevice::getScan ()->stop ();
            myDevice  = new BLEAdvertisedDevice (advertisedDevice);
            doConnect = true;
            doScan    = false;

            BLEM_LOG_ALL ("Device found by bleServerName ");
            uint8_t* data = advertisedDevice.getPayload ();
            int len       = advertisedDevice.getPayloadLength ();

            uint8_t flag_size     = data[0];
            uint8_t flag_type     = data[1];
            uint8_t flags         = data[2];
            uint8_t manuData_size = data[3];
            uint8_t manuData_type = data[4];

            // BLEM_LOG_ALL("Flag Size: %u\n", flag_size);
            // BLEM_LOG_ALL("Flag Type: 0x%02X\n", flag_type);
            // BLEM_LOG_ALL("Flags Set: 0x%02X\n", flags);
            // BLEM_LOG_ALL("Manufacturer Data Size: %u\n", manuData_size);
            // BLEM_LOG_ALL("Manufacturer Data Type: 0x%02X\n", manuData_type);

            if (manuData_type == 0xFF && data[5] == 0x9D && data[6] == 0x00) {
                uint32_t esn =
                (data[7] << 24) | (data[8] << 16) | (data[9] << 8) | data[10];
                // BLEM_LOG_ALL("ESN: %u\n", esn);
                // if (esn == 70076190) {

                uint8_t device_flags = data[11];
                BLEM_LOG_ALL ("Device Flags: 0x%02X\n", device_flags);
                BLEM_LOG_ALL ("  Iridium Modem Present: %s\n",
                              (device_flags & 0x02) ? "Yes" : "No");
                BLEM_LOG_ALL ("  Cell Modem Present: %s\n",
                              (device_flags & 0x04) ? "Yes" : "No");
                BLEM_LOG_ALL ("  Encryption Enabled: %s\n",
                              (device_flags & 0x08) ? "Yes" : "No");
                BLEM_LOG_ALL ("  Primary Cells in Use: %s\n",
                              (device_flags & 0x10) ? "Yes" : "No");
                BLEM_LOG_ALL ("  Solar Present: %s\n", (device_flags & 0x20) ? "Yes" : "No");
                BLEM_LOG_ALL ("  In Motion: %s\n",
                              (device_flags & 0x80) ? "Yes" : "No");

                uint8_t app_version_major = (data[12] >> 4) & 0x0F;
                uint8_t app_version_minor = data[12] & 0x0F;
                BLEM_LOG_ALL ("App SW Version: %u.%u\n", app_version_major, app_version_minor);

                uint8_t app_sw_build = data[13];
                BLEM_LOG_ALL ("App SW Build: %u\n", app_sw_build);

                uint8_t cell_sw_version = data[14];
                BLEM_LOG_ALL ("Cell SW Version: %u\n", cell_sw_version);

                uint8_t sd_sw_version = data[15];
                BLEM_LOG_ALL ("SD SW Version: %u\n", sd_sw_version);

                uint8_t primary_batt = data[16];
                BLEM_LOG_ALL ("Primary Battery: %u%%\n", primary_batt);

                uint8_t secondary_batt = data[17];
                BLEM_LOG_ALL ("Secondary Battery: %u%%\n", secondary_batt);

                int8_t secondary_surplus_deficit = data[18];
                BLEM_LOG_ALL ("Secondary Surplus/Deficit: %d%%\n", secondary_surplus_deficit);

                uint8_t reserved = data[19];
                BLEM_LOG_ALL ("Reserved: 0x%02X\n", reserved);

                uint8_t health_status = data[20];
                BLEM_LOG_ALL ("Health Status: 0x%02X\n", health_status);
                BLEM_LOG_ALL ("  Iridium communication success: %s\n",
                              (health_status & 0x01) ? "Yes" : "No");
                BLEM_LOG_ALL ("  GPS success: %s\n", (health_status & 0x02) ? "Yes" : "No");
                BLEM_LOG_ALL ("  User Scratchpad Data: %s\n",
                              (health_status & 0x04) ? "Pending" : "No");
                BLEM_LOG_ALL ("  Powersave mode status: %s\n",
                              (health_status & 0x08) ? "In Powersave" : "Not In Powersave");

                uint8_t gatt_name_length = data[21];
                BLEM_LOG_ALL ("GATT Name Length: %u\n", gatt_name_length);

                uint8_t gatt_name_type = data[22];
                BLEM_LOG_ALL ("GATT Name Type: 0x%02X\n", gatt_name_type);

                char esn_ascii[8 + 1];
                memcpy (esn_ascii, &data[23], 8);
                esn_ascii[8] = '\0';
                BLEM_LOG_ALL ("ESN ASCII Representation: %s\n", esn_ascii);

                if (esn_ascii == _status->get_BLE_server ()) {
                    // _status->set_ble_status (1);
                }

                // StaticJsonDocument<512> doc;

                // // Device Flags
                // doc["Device Flags"]["Iridium Modem Present"] =
                // (device_flags & 0x02) ? "Yes" : "No";
                // doc["Device Flags"]["Cell Modem Present"] =
                // (device_flags & 0x04) ? "Yes" : "No";
                // doc["Device Flags"]["Encryption Enabled"] =
                // (device_flags & 0x08) ? "Yes" : "No";
                // doc["Device Flags"]["Primary Cells in Use"] =
                // (device_flags & 0x10) ? "Yes" : "No";
                // doc["Device Flags"]["Solar Present"] = (device_flags & 0x20) ? "Yes" : "No";
                // doc["Device Flags"]["In Motion"] =
                // (device_flags & 0x80) ? "Yes" : "No";

                // // App SW Version
                // doc["App SW Version"]["Major"] = app_version_major;
                // doc["App SW Version"]["Minor"] = app_version_minor;

                // // App SW Build
                // doc["App SW Build"] = data[13];

                // // Cell SW Version
                // doc["Cell SW Version"] = data[14];

                // // SD SW Version
                // doc["SD SW Version"] = data[15];

                // // Primary Battery
                // doc["Primary Battery"] = data[16];

                // // Secondary Battery
                // doc["Secondary Battery"] = data[17];

                // // Secondary Surplus/Deficit
                // doc["Secondary Surplus/Deficit"] = (int8_t)data[18];

                // // Reserved
                // doc["Reserved"] = data[19];

                // // Health Status
                // doc["Health Status"]["Iridium communication success"] =
                // (health_status & 0x01) ? "Yes" : "No";
                // doc["Health Status"]["GPS success"] = (health_status & 0x02) ? "Yes" : "No";
                // doc["Health Status"]["User Scratchpad Data"] =
                // (health_status & 0x04) ? "Pending" : "No";
                // doc["Health Status"]["Powersave mode status"] =
                // (health_status & 0x08) ? "In Powersave" : "Not In Powersave";

                // // GATT Name Length
                // doc["GATT Name Length"] = data[21];

                // // GATT Name Type
                // doc["GATT Name Type"] = data[22];

                // // ESN ASCII Representation
                // doc["ESN ASCII Representation"] = esn_ascii;

                // // Output the JSON
                // serializeJson (doc, Serial);
            }
            pBLEScan->clearResults ();
        }

    } // onResult
};    // MyAdvertisedDeviceCallbacks

void scanCompleteCallback (BLEScanResults scanResults) {
    // Serial.println ("Scan complete!");
    // int count = scanResults.getCount ();
    // BLEM_LOG_ALL ("Found %d devices\n", count);

    // for (int i = 0; i < count; i++) {
    //     BLEAdvertisedDevice device = scanResults.getDevice (i);
    //     BLEM_LOG_ALL ("Device %d: %s, RSSI: %d\n", i+1, device.toString ().c_str (), device.getRSSI ());
    // }
}

void BLE_handler::entrypoint () {
    aes256.checkAndCreateEncryptionKeysFile ();
    aes256.get_encryption_payload ();

    BLEM_LOG_ALL ("Starting Arduino BLE Client application...");
    BLEDevice::init ("");

    // Retrieve a Scanner and set the callback we want to use to be informed
    // when we have detected a new device.  Specify that we want active scanning
    // and start the scan to run for 5 seconds.
    pBLEScan = BLEDevice::getScan ();
    pBLEScan->setAdvertisedDeviceCallbacks (new MyAdvertisedDeviceCallbacks ());
    pBLEScan->setInterval (2130); // Set interval to 2.13 seconds
    pBLEScan->setWindow (2130); // Set window to 2.13 seconds for continuous scanning
    pBLEScan->setActiveScan (true);
    pBLEScan->start (10, &scanCompleteCallback, true); // Start scan indefinitely

    bool set_60min = false;
    while (1) {
        // If the flag "doConnect" is true then we have scanned for and found
        // the desired BLE Server with which we wish to connect.  Now we connect
        // to it. Once we are connected we set the connected flag to be true.
        if (doConnect == true) {
            if (connectToServer ()) {
                BLEM_LOG_ALL ("We are now connected to the BLE Server.");
                _status->set_ble_status (1);
            } else {
                BLEM_LOG_ALL (
                "We have failed to connect to the server; there is nothin "
                "more we will do.");
                _status->set_ble_status (0);
            }
            doConnect = false;
        }


        // If we are connected to a peer BLE Server, update the characteristic
        // each time we are reached with the current time since boot.
        if (connected) {


            if (_status->get_ble_status () == 1) {
                if (_status->get_sos_status () == 99) {
                    if (!send_SOS) {
                        BLEM_LOG_ALL ("SOS...");
                        set_interval_30m ();
                        set_interval_60m ();
                        pRemoteCharacteristic_GF_MT_CHAR1->writeValue (
                        (uint8_t*)_status->get_e_msg_sos (), 20, true);
                        delay (100);
                        pRemoteCharacteristic_GF_MT_ACT->writeValue (
                        (uint8_t*)_status->get_process (), 1, true);
                        //-------------------------------------------------
                        delay (5000);
                        send_eng_msg ();
                        delay (5000);
                        send_ping_location ();
                        //-------------------------------------------------
                        send_SOS = true;
                    }
                }

                if (_status->get_sos_status () == 100) {
                    if (!send_CANCEL_SOS && send_SOS) {
                        BLEM_LOG_ALL ("CANCEL SOS...");
                        // set_interval_30m ();
                        // set_interval_60m ();
                        pRemoteCharacteristic_GF_MT_CHAR1->writeValue (
                        (uint8_t*)_status->get_e_msg_cancel_sos (), 20, true);
                        delay (100);
                        pRemoteCharacteristic_GF_MT_ACT->writeValue (
                        (uint8_t*)_status->get_process (), 1, true);
                        //-------------------------------------------------
                        delay (10000);
                        send_eng_msg ();
                        delay (10000);
                        send_ping_location ();
                        //-------------------------------------------------
                        send_CANCEL_SOS = true;
                    }
                }

                if (!get_config) {
                    delay (10000);
                    BLEM_LOG_ALL ("Get config");
                    pRemoteCharacteristic_GF_MT_CHAR1->writeValue (
                    (uint8_t*)_status->get_e_ies_cfg (), 20, true);
                    delay (100);
                    pRemoteCharacteristic_GF_MT_ACT->writeValue (
                    (uint8_t*)_status->get_process (), 1, true);
                    // delay (60000);

                    get_config = !get_config;
                }
                // delay (10000);
                // disini
            }


        } else if (doScan) {
            // BLEDevice::getScan()->start(0);  // this is just example to start scan
            // after disconnect, most likely there is better way to do it in arduino
            // BLEDevice::getScan ()->start (60, true); // this is just eample to start scan after disconnect, most
            //                                          // likely there is better way to do it in arduino

            BLEScanResults foundDevices = pBLEScan->start (5, false);

            BLEM_LOG_ALL ("Devices found: %d | Scan done!", foundDevices.getCount ());
            pBLEScan->clearResults (); // delete results fromBLEScan buffer to release memory
        }
        // BLEM_LOG_ALL ("connection status = %d || %d", connected,
        //               _status->get_ble_status ());

        if (!connected || _status->get_ble_status () == 0) {
            unsigned long currentMillis_BLE_connection = millis ();
            if (currentMillis_BLE_connection - previousMillis_BLE_connection >=
                interval_BLE_connection) {
                previousMillis_BLE_connection = currentMillis_BLE_connection;
                Serial.println ("do scan");
                // delay (1000);
                // ESP.restart ();
                doScan = true;
            }
            // Serial.println (currentMillis_BLE_connection);
        }


        delay (100); // Delay a second between loops.
    }                // End of loop
}


void BLE_handler::entrypoint_parser () {
    auto _status = deviceStatus::GetInstance ();

    long previousMillis = 0; // will store last time LED was updated

    // the follow variables is a long because the time, measured in miliseconds,
    // will quickly become a bigger number than can be stored in an int.
    long interval = 600000; // interval at which to blink (milliseconds)

    while (1) {
        unsigned long currentMillis = millis ();
        if (!connected || _status->get_ble_status () == 0) {
            if (currentMillis - previousMillis > interval) {
                // save the last time you blinked the LED
                previousMillis = currentMillis;
                Serial.println ("own __ restart");
                delay (3000);
                // ESP.restart ();
                // BLEM_LOG_ALL ("_msg.get_msg_json_tracker : %s",
                //                  _msg.get_msg_json_tracker ().c_str ());
            }
        }


        if (connected) {
            if (_status->get_act_gen_config () == 1) {
                BLEM_LOG_ALL ("Set config");

                // Variables to store the split parts
                uint8_t encryptedMessage[40];
                uint8_t part1[20];
                uint8_t part2[20];

                memcpy (encryptedMessage, _status->get_ble_general_config (), 40);

                // Split the data into part1 and part2
                memcpy (part1, encryptedMessage, 20);
                memcpy (part2, encryptedMessage + 20, 20);

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (part1, 20, true);
                delay (100);

                // Write the next 20 bytes to the second characteristic
                pRemoteCharacteristic_GF_MT_CHAR2->writeValue (part2, 16, true);
                delay (100);

                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);
                delay (2000);

                get_config = !get_config;
            } else if (_status->get_act_user_msg () == 20) {

                BLEM_LOG_ALL ("send user msg");

                // Variables to store the split parts
                uint8_t encryptedMessage[40];
                uint8_t part1[20];
                uint8_t part2[20];

                memcpy (encryptedMessage, _status->get_ble_user_msg (), 20);

                // Split the data into part1 and part2
                memcpy (part1, encryptedMessage, 20);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < 20; i++) {
                    Serial.printf ("%02X", encryptedMessage[i]);
                }
                Serial.println ();

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (part1, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);
                delay (3000);
            } else if (_status->get_act_user_location_msg () == 20) {

                BLEM_LOG_ALL ("send user location msg");

                // Variables to store the split parts
                uint8_t encryptedMessage[40];
                uint8_t part1[20];
                uint8_t part2[20];

                memcpy (encryptedMessage, _status->get_ble_user_location_msg (), 20);

                // Split the data into part1 and part2
                memcpy (part1, encryptedMessage, 20);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < 20; i++) {
                    Serial.printf ("%02X", encryptedMessage[i]);
                }
                Serial.println ();

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (part1, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);
                delay (3000);
            } else if (_status->get_act_location_only () == 1) {

                BLEM_LOG_ALL ("send location only");
                uint8_t msg_location_only[20] = { 0x23, 0x00, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff };

                // Encrypt the byte array
                AES_256 aes256;
                uint8_t buffer[20];

                uint8_t retrieved_key[32];
                auto _status = deviceStatus::GetInstance ();
                _status->get_key_AES256 (retrieved_key);
                aes256.encrypt_aes256 (retrieved_key, msg_location_only, 16, buffer);
                delay (100);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < (20); i++) {
                    Serial.printf ("%02X", buffer[i]);
                }
                Serial.println ();
                _status->set_act_location_only (0);

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);
                delay (3000);
            } else if (_status->get_act_eng_msg () == 1) {

                BLEM_LOG_ALL ("send eng msg");
                uint8_t msg_send_eng_msg[20] = { 0x22, 0x00, 0xff, 0xff,
                                                 0xff, 0xff, 0xff, 0xff,
                                                 0xff, 0xff, 0xff, 0xff,
                                                 0xff, 0xff, 0xff, 0xff };

                // Encrypt the byte array
                AES_256 aes256;
                uint8_t buffer[20];

                uint8_t retrieved_key[32];
                auto _status = deviceStatus::GetInstance ();
                _status->get_key_AES256 (retrieved_key);
                aes256.encrypt_aes256 (retrieved_key, msg_send_eng_msg, 16, buffer);
                delay (100);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < (20); i++) {
                    Serial.printf ("%02X", buffer[i]);
                }
                Serial.println ();
                _status->set_act_eng_msg (0);

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);
                delay (3000);
            } else if (_status->get_tamper_BLEM () == 99) {

                BLEM_LOG_ALL ("send Tamper BLEM");
                uint8_t msg_tamper_BLEM[20] = { 0x24, 0x07, 0x09, 0xbb,
                                                0xbb, 0xbb, 0xbb, 0xbb,
                                                0xbb, 0xff, 0xff, 0xff,
                                                0xff, 0xff, 0xff, 0xff };

                // Encrypt the byte array
                AES_256 aes256;
                uint8_t buffer[20];

                uint8_t retrieved_key[32];
                auto _status = deviceStatus::GetInstance ();
                _status->get_key_AES256 (retrieved_key);
                aes256.encrypt_aes256 (retrieved_key, msg_tamper_BLEM, 16, buffer);
                delay (100);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < (20); i++) {
                    Serial.printf ("%02X", buffer[i]);
                }
                Serial.println ();
                _status->set_act_location_only (0);

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);

                _status->set_tamper_BLEM (0);
            } else if (_status->get_tamper_NBLITE () == 99) {

                BLEM_LOG_ALL ("send Tamper NBLite");
                uint8_t msg_tamper_NBLITE[20] = { 0x24, 0x07, 0x09, 0xaa,
                                                  0xaa, 0xaa, 0xaa, 0xaa,
                                                  0xaa, 0xff, 0xff, 0xff,
                                                  0xff, 0xff, 0xff, 0xff };

                // Encrypt the byte array
                AES_256 aes256;
                uint8_t buffer[20];

                uint8_t retrieved_key[32];
                auto _status = deviceStatus::GetInstance ();
                _status->get_key_AES256 (retrieved_key);
                aes256.encrypt_aes256 (retrieved_key, msg_tamper_NBLITE, 16, buffer);
                delay (100);

                // Output the byte array for verification
                Serial.printf ("buffer Array: ");
                for (size_t i = 0; i < (20); i++) {
                    Serial.printf ("%02X", buffer[i]);
                }
                Serial.println ();
                _status->set_act_location_only (0);

                // Write the first 20 bytes to the first characteristic
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
                delay (100);
                // Write the process status
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (),
                                                             1, true);

                _status->set_tamper_NBLITE (0);
            }
        }

        delay (100);
    }
}

void BLE_handler::set_interval_30m () {
    BLEM_LOG_ALL ("Set config 30m");

    // Variables to store the split parts
    uint8_t encryptedMessage[40];
    uint8_t part1[20];
    uint8_t part2[20];

    memcpy (encryptedMessage, _status->get_e_set_30m (), 40);

    // Split the data into part1 and part2
    memcpy (part1, encryptedMessage, 20);
    memcpy (part2, encryptedMessage + 20, 20);

    // Write the first 20 bytes to the first characteristic
    pRemoteCharacteristic_GF_MT_CHAR1->writeValue (part1, 20, true);
    delay (100);

    // Write the next 20 bytes to the second characteristic
    pRemoteCharacteristic_GF_MT_CHAR2->writeValue (part2, 16, true);
    delay (100);

    // Write the process status
    pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (), 1, true);
    delay (5000);
}

void BLE_handler::set_interval_60m () {
    BLEM_LOG_ALL ("Set config 60m");

    // Variables to store the split parts
    uint8_t encryptedMessage[40];
    uint8_t part1[20];
    uint8_t part2[20];

    memcpy (encryptedMessage, _status->get_e_set_60m (), 40);

    // Split the data into part1 and part2
    memcpy (part1, encryptedMessage, 20);
    memcpy (part2, encryptedMessage + 20, 20);

    // Write the first 20 bytes to the first characteristic
    pRemoteCharacteristic_GF_MT_CHAR1->writeValue (part1, 20, true);
    delay (100);

    // Write the next 20 bytes to the second characteristic
    pRemoteCharacteristic_GF_MT_CHAR2->writeValue (part2, 16, true);
    delay (100);

    // Write the process status
    pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (), 1, true);
    delay (5000);
}


void BLE_handler::send_eng_msg () {


    BLEM_LOG_ALL ("send eng msg");
    uint8_t msg_send_eng_msg[20] = { 0x22, 0x00, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0xff, 0xff };

    // Encrypt the byte array
    AES_256 aes256;
    uint8_t buffer[20];

    uint8_t retrieved_key[32];
    auto _status = deviceStatus::GetInstance ();
    _status->get_key_AES256 (retrieved_key);
    aes256.encrypt_aes256 (retrieved_key, msg_send_eng_msg, 16, buffer);
    delay (100);

    // Output the byte array for verification
    Serial.printf ("buffer Array: ");
    for (size_t i = 0; i < (20); i++) {
        Serial.printf ("%02X", buffer[i]);
    }
    Serial.println ();
    _status->set_act_eng_msg (0);

    // Write the first 20 bytes to the first characteristic
    pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
    delay (100);
    // Write the process status
    pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (), 1, true);
}
void BLE_handler::send_ping_location () {

    BLEM_LOG_ALL ("send location only");
    uint8_t msg_location_only[20] = { 0x23, 0x00, 0xff, 0xff, 0xff, 0xff,
                                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                      0xff, 0xff, 0xff, 0xff };

    // Encrypt the byte array
    AES_256 aes256;
    uint8_t buffer[20];

    uint8_t retrieved_key[32];
    auto _status = deviceStatus::GetInstance ();
    _status->get_key_AES256 (retrieved_key);
    aes256.encrypt_aes256 (retrieved_key, msg_location_only, 16, buffer);
    delay (100);

    // Output the byte array for verification
    Serial.printf ("buffer Array: ");
    for (size_t i = 0; i < (20); i++) {
        Serial.printf ("%02X", buffer[i]);
    }
    Serial.println ();
    _status->set_act_location_only (0);

    // Write the first 20 bytes to the first characteristic
    pRemoteCharacteristic_GF_MT_CHAR1->writeValue (buffer, 20, true);
    delay (100);
    // Write the process status
    pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)_status->get_process (), 1, true);
    delay (3000);
}