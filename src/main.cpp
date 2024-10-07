#include "handler/BLE_handler.h"
#include "netracubeBLE.h"
#include "storage/localSpiffs.h"
#include "tools/tools.h"
#include <Arduino.h>


BLE_handler thisstart;
netracubeBLE _at;
localSpiffs _storage;

void setup () {
    Serial.begin (115200);
    _storage.init ();
    thisstart.start ();
    _at.task_init ();
}

void loop () {
    delay (60000);
    BLEM_LOG_ALL ("heart beat: %d", get_time_BLEM ());
}
