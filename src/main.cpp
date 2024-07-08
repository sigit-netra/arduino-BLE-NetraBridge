#include "Arduino.h"
#include "BLEDevice.h"
//#include "BLEScan.h"
#include "AES_256.h"
#include "global.hpp"
#include "netracubeBLE.h"
#include "status.hpp"

AES_256 aes256_test;

netracubeBLE NetraCubeBLE;
auto status = Status::GetInstance ();


int prevInterval    = 0;
int currentInterval = 0;

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

const long interval_BLE_connection          = 30000;
unsigned long previousMillis_BLE_connection = 0;

#define bleServerName "70002921" // hanyak contoh

// The remote service we wish to connect to.
static BLEUUID serviceUUID ("6ecb2400-dc4c-40cc-a6e0-81e0dbda54e5");
// The characteristic of the remote service we are interested in.
static BLEUUID UUID_CHAR_GF_MT_CHAR1 ("6ecb2401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MT_ACT ("6ecb3401-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_CHAR1 ("6ecb2481-dc4c-40cc-a6e0-81e0dbda54e5");
static BLEUUID UUID_CHAR_GF_MO_ACT ("6ecb3481-dc4c-40cc-a6e0-81e0dbda54e5");

static BLEAddress bleAddr ("df:19:c2:99:eb:76"); // hanyak contoh

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
        if (advertisedDevice.getName () == status->get_BLE_server () /*bleServerName*/) {
            BLEDevice::getScan ()->stop ();
            myDevice  = new BLEAdvertisedDevice (advertisedDevice);
            doConnect = true;
            doScan    = true;
            Serial.println ("Device found by bleServerName ");
        }

    } // onResult
};    // MyAdvertisedDeviceCallbacks

void ledTask(void *pvParameters) {
    for(;;) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500); // LED on for 500 ms
        digitalWrite(LED_BUILTIN, LOW);
        delay(500); // LED off for 500 ms
    }
}


void bleTask(void *pvParameters) {

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
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)ble_msg.msg_sos,
                                                               20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)ble_msg.msg_process,
                                                             1, true);
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
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)ble_msg.msg_cancel_sos,
                                                               20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)ble_msg.msg_process,
                                                             1, true);
                send_CANCEL_SOS = true;
            }
        }
        if (status->GetTamperStatus ()) {
            if (!send_tamper) {
                Serial.println ("TAMPER...");

                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)ble_msg.msg_tamper,
                                                               20, true);
                delay (100);
                // Set the characteristic's value to be the array of bytes that
                // is actually a string.
                pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)ble_msg.msg_process,
                                                             1, true);
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
        if (status->GetIntervalStatus () != prevInterval) {

            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)ble_msg.msg_int60m,
                                                           sizeof (ble_msg.msg_int60m), true);
            delay (100);
            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)ble_msg.msg_process,
                                                         1, true);

            status->SetIntervalStatus (0);
        }


    } else if (doScan) {
        // BLEDevice::getScan()->start(0);  // this is just example to start scan
        // after disconnect, most likely there is better way to do it in arduino
        BLEDevice::getScan ()->start (60, true); // this is just eample to start scan after disconnect, most
                                                 // likely there is better way to do it in arduino
    }

    if (!connected || status->GetBLEStatus () == 0) {
        status->SetBLEStatus (0);

        unsigned long currentMillis_BLE_connection = millis ();
        if (currentMillis_BLE_connection - previousMillis_BLE_connection >= interval_BLE_connection) {
            previousMillis_BLE_connection = currentMillis_BLE_connection;
            Serial.println ("own __ restart");
            delay (1000);
            ESP.restart ();
        }
        Serial.println (currentMillis_BLE_connection);
    }

    if (status->GetSOSStatus () > 1 && !return_GF_MO_CHAR1 && !return_GF_MO_ACT) {
        unsigned long currentMillis = millis ();

        if (currentMillis - previousMillis >= interval) {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            Serial.println ("RETRY SEND SOS...");

            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_CHAR1->writeValue ((uint8_t*)ble_msg.msg_sos,
                                                           20, true);
            delay (100);
            // Set the characteristic's value to be the array of bytes that
            // is actually a string.
            pRemoteCharacteristic_GF_MT_ACT->writeValue ((uint8_t*)ble_msg.msg_process,
                                                         1, true);
        }
    }
    delay (1000); // Delay a second between loops.
} // End of loop


void setup () {
    Serial.begin (115200);

    aes256_test.get_aes256 ();

    aes256_test.test_AES256 ();

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

    xTaskCreatePinnedToCore(
        bleTask,   /* Function to implement the task */
        "bleTask", /* Name of the task */
        10000,     /* Stack size in words */
        NULL,      /* Task input parameter */
        1,         /* Priority of the task */
        NULL,      /* Task handle. */
        0);        /* Core where the task should run */

    xTaskCreatePinnedToCore(
        ledTask,   /* Function to implement the task */
        "ledTask", /* Name of the task */
        10000,     /* Stack size in words */
        NULL,      /* Task input parameter */
        1,         /* Priority of the task */
        NULL,      /* Task handle. */
        1);        /* Core where the task should run */
} // End of setup.

// This is the Arduino main loop function.
void loop () {
    // delay (100000); // Delay a second between loops.
}