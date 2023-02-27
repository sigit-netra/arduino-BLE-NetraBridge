#include "Arduino.h"
#include "BLEDevice.h"
//#include "BLEScan.h"
#include "AES_256.h"
#include "netracubeBLE.h"
#include "status.hpp"


AES_256 aes256_test;

netracubeBLE NetraCubeBLE;
auto status = Status::GetInstance ();

bool send_SOS        = false;
bool send_CANCEL_SOS = false;
bool send_tamper     = false;

bool return_send_SOS        = false;
bool return_send_CANCEL_SOS = false;
bool return_send_tamper     = false;
bool return_GF_MO_CHAR1     = false;
bool return_GF_MO_ACT       = false;

unsigned long previousMillis = 0;
const long interval          = 600000;

const long interval_BLE_connection          = 300000;
unsigned long previousMillis_BLE_connection = 0;

// #define bleServerName "70005216"
// #define bleServerName "70002557"
#define bleServerName "70002921"


// The remote service we wish to connect to.
static BLEUUID serviceUUID ("6ecb2400-dc4c-40cc-a6e0-81e0dbda54e5");
// The characteristic of the remote service we are interested in.
static BLEUUID UUID_CHAR_GF_MT_CHAR1 ("6ecb2401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MT_ACT ("6ecb3401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_CHAR1 ("6ecb2481-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_ACT ("6ecb3481-dc4c-40cc-a6e0-81e0dbda54e5");

// static BLEAddress bleAddr ("f6:6a:e2:9c:a4:95");
static BLEAddress bleAddr ("df:19:c2:99:eb:76");

const uint8_t msg_sos[20]        = { 0x01, 0xD3, 0xAE, 0x4A, 0x17, 0xC1, 0xFA,
                              0x05, 0xEC, 0xD7, 0x11, 0x1E, 0x06, 0x71,
                              0xE8, 0x7E, 0x3E, 0x1E, 0x27, 0x4F };
const uint8_t msg_cancel_sos[20] = { 0x01, 0xAE, 0x8D, 0x93, 0xDD, 0x1B, 0x61,
                                     0x1D, 0x1C, 0xC5, 0x4D, 0xDF, 0xDF, 0xAA,
                                     0xF5, 0x0D, 0x26, 0x1E, 0x32, 0xBF };
const uint8_t msg_tamper[20]     = { 0x01, 0x36, 0x48, 0x27, 0xA7, 0x18, 0x0B,
                                 0x6D, 0x21, 0x05, 0xCB, 0xFA, 0xD3, 0x6F,
                                 0xC6, 0x27, 0xD6, 0x1E, 0xB5, 0x14 };

const uint8_t msg_process[1] = { 0x01 };

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan    = false;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MT_CHAR1;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MT_ACT;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MO_CHAR1;
static BLERemoteCharacteristic* pRemoteCharacteristic_GF_MO_ACT;
static BLEAdvertisedDevice* myDevice;

static void notifyCallbackGF_MO_CHAR1 (BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                       uint8_t* pData,
                                       size_t length,
                                       bool isNotify) {
    Serial.print ("Notify callback for characteristic ");
    Serial.print (pBLERemoteCharacteristic->getUUID ().toString ().c_str ());
    Serial.print (" of data length ");
    Serial.println (length);
    Serial.print ("data: ");
    Serial.println ((char*)pData);
    return_GF_MO_CHAR1 = true;
    status->SetSOSStatus (0);
}

static void notifyCallbackGF_MO_ACT (BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                     uint8_t* pData,
                                     size_t length,
                                     bool isNotify) {
    Serial.print ("Notify callback for characteristic ");
    Serial.print (pBLERemoteCharacteristic->getUUID ().toString ().c_str ());
    Serial.print (" of data length ");
    Serial.println (length);
    Serial.print ("data: ");
    Serial.println ((char*)pData);
    return_GF_MO_ACT = true;
    status->SetSOSStatus (0);
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect (BLEClient* pclient) {}


    void onDisconnect (BLEClient* pclient) {
        connected = false;
        Serial.println ("onDisconnect");
    }
};

bool connectToServer () {
    Serial.print ("Forming a connection to ");
    Serial.println (myDevice->getAddress ().toString ().c_str ());


    BLEClient* pClient = BLEDevice::createClient ();
    Serial.println (" - Created client");


    pClient->setClientCallbacks (new MyClientCallback ());


    // Connect to the remove BLE Server.
    pClient->connect (myDevice); // if you pass BLEAdvertisedDevice instead of
                                 // address, it will be recognized type of peer
                                 // device address (public or private)
    Serial.println (" - Connected to server");
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
    Serial.println (" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE
    // server.
    pRemoteCharacteristic_GF_MT_CHAR1 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MT_CHAR1);
    pRemoteCharacteristic_GF_MT_ACT =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MT_ACT);
    pRemoteCharacteristic_GF_MO_CHAR1 =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MO_CHAR1);
    pRemoteCharacteristic_GF_MO_ACT =
    pRemoteService->getCharacteristic (UUID_CHAR_GF_MO_ACT);

    if (pRemoteCharacteristic_GF_MT_CHAR1 == nullptr) {
        Serial.print ("Failed to find our characteristic UUID: ");
        Serial.println (UUID_CHAR_GF_MT_CHAR1.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MT_ACT == nullptr) {
        Serial.print ("Failed to find our characteristic UUID: ");
        Serial.println (UUID_CHAR_GF_MT_ACT.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MO_CHAR1 == nullptr) {
        Serial.print ("Failed to find our characteristic UUID: ");
        Serial.println (UUID_CHAR_GF_MO_CHAR1.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }

    if (pRemoteCharacteristic_GF_MO_ACT == nullptr) {
        Serial.print ("Failed to find our characteristic UUID: ");
        Serial.println (UUID_CHAR_GF_MO_ACT.toString ().c_str ());
        pClient->disconnect ();
        return false;
    }
    Serial.println (" - Found our characteristic");


    // Read the value of the characteristic.
    if (pRemoteCharacteristic_GF_MT_CHAR1->canRead ()) {
        std::string value = pRemoteCharacteristic_GF_MT_CHAR1->readValue ();
        Serial.print ("The characteristic value was: ");
        Serial.println (value.c_str ());
    }


    if (pRemoteCharacteristic_GF_MO_CHAR1->canNotify ())
        pRemoteCharacteristic_GF_MO_CHAR1->registerForNotify (notifyCallbackGF_MO_CHAR1);

    if (pRemoteCharacteristic_GF_MO_ACT->canNotify ())
        pRemoteCharacteristic_GF_MO_ACT->registerForNotify (notifyCallbackGF_MO_ACT);

    connected = true;
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
        Serial.print ("BLE Advertised Device found: ");
        Serial.println (advertisedDevice.toString ().c_str ());

        auto status = Status::GetInstance ();
        // status->set_BLE_server (esn.c_str ());


        // We have found a device, let us now see if it contains the service we
        // are looking for.
        if (advertisedDevice.haveServiceUUID () &&
            advertisedDevice.isAdvertisingService (serviceUUID)) {

            BLEDevice::getScan ()->stop ();
            myDevice  = new BLEAdvertisedDevice (advertisedDevice);
            doConnect = true;
            doScan    = true;

        } // Found our server
        if (advertisedDevice.getAddress ().equals (bleAddr)) {
            BLEDevice::getScan ()->stop ();
            myDevice  = new BLEAdvertisedDevice (advertisedDevice);
            doConnect = true;
            doScan    = true;
            Serial.println ("Device found by address ");
        }

        if (advertisedDevice.getName () == status->get_BLE_server () /*bleServerName*/) {
            BLEDevice::getScan ()->stop ();
            myDevice  = new BLEAdvertisedDevice (advertisedDevice);
            doConnect = true;
            doScan    = true;
            Serial.println ("Device found by bleServerName ");
        }

    } // onResult
};    // MyAdvertisedDeviceCallbacks


void setup () {
    Serial.begin (115200);

    aes256_test.get_aes256 ();

    aes256_test.test_AES256 ();

    // Serial.println ("-----------");
    // for (uint8_t i = 0; i < 32; i++) {
    //     Serial.print (BLE_components::key_AES256[i], HEX);
    //     // server.AES256[i] = myuint[i];
    // }
    // Serial.println ("-----------");

    NetraCubeBLE.task_init ();

    Serial.println ("Starting Arduino BLE Client application...");
    BLEDevice::init ("");

    // Retrieve a Scanner and set the callback we want to use to be informed
    // when we have detected a new device.  Specify that we want active scanning
    // and start the scan to run for 5 seconds.
    BLEScan* pBLEScan = BLEDevice::getScan ();
    pBLEScan->setAdvertisedDeviceCallbacks (new MyAdvertisedDeviceCallbacks ());
    pBLEScan->setInterval (1349);
    pBLEScan->setWindow (449);
    pBLEScan->setActiveScan (true);
    pBLEScan->start (60, true);
} // End of setup.


// This is the Arduino main loop function.
void loop () {

    // If the flag "doConnect" is true then we have scanned for and found the
    // desired BLE Server with which we wish to connect.  Now we connect to it.
    // Once we are connected we set the connected flag to be true.
    if (doConnect == true) {
        if (connectToServer ()) {
            Serial.println ("We are now connected to the BLE Server.");
        } else {
            Serial.println (
            "We have failed to connect to the server; there is nothin "
            "more we will do.");
        }
        doConnect = false;
    }


    // If we are connected to a peer BLE Server, update the characteristic each
    // time we are reached with the current time since boot.
    if (connected) {
        status->SetBLEStatus (1);
        // String newValue = "Time since boot: " + String (millis () / 1000);
        // Serial.println ("Setting new characteristic value to \"" + newValue + "\"");
        if (status->GetSOSStatus () == 5) {
            if (!send_SOS) {
                Serial.println ("SOS...");

                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)msg_sos, 20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)msg_process, 1, true);
                return_GF_MO_CHAR1 = false;
                return_GF_MO_ACT   = false;

                send_SOS = true;
            }
        }

        if (status->GetSOSStatus () == 10) {
            if (!send_CANCEL_SOS && send_SOS) {
                Serial.println ("CANCEL SOS...");

                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)msg_cancel_sos,
                                                               20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)msg_process, 1, true);
                send_CANCEL_SOS = true;
            }
        }
        if (status->GetTamperStatus ()) {
            if (!send_tamper) {
                Serial.println ("TAMPER...");

                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)msg_tamper,
                                                               20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)msg_process, 1, true);
                send_tamper = true;
            }
        }

        if ((status->GetSOSStatus ()) == 10) {
            send_SOS        = false;
            send_CANCEL_SOS = false;
        }
        if (!(status->GetTamperStatus ())) {
            send_tamper = false;
        }
    } else if (doScan) {
        // BLEDevice::getScan()->start(0);  // this is just example to start scan
        // after disconnect, most likely there is better way to do it in arduino
        BLEDevice::getScan ()->start (60, true); // this is just eample to start scan after disconnect, most
                                                 // likely there is better way to do it in arduino
    }

    if (!connected) {
        status->SetBLEStatus (0);

        unsigned long currentMillis_BLE_connection = millis ();
        if (currentMillis_BLE_connection - previousMillis_BLE_connection >= interval_BLE_connection) {
            previousMillis_BLE_connection = currentMillis_BLE_connection;
            ESP.restart ();
        }
    }

    if (status->GetSOSStatus () > 1 && !return_GF_MO_CHAR1 && !return_GF_MO_ACT) {
        unsigned long currentMillis = millis ();

        if (currentMillis - previousMillis >= interval) {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            Serial.println ("RETRY SEND SOS...");

            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)msg_sos, 20, true);
            delay (100);
            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)msg_process, 1, true);
        }
    }


    delay (1000); // Delay a second between loops.
} // End of loop
