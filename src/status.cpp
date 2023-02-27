#include "status.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

Status::Status () { this->lock = xSemaphoreCreateMutex (); }
Status* Status::instance = NULL;
Status* Status::GetInstance () {
    if (!instance) {
        instance = new Status ();
        return instance;
    } else {
        return instance;
    }
}

uint8_t Status::GetTamperStatus () { return this->_tamper_status; }
void Status::SetTamperStatus (uint8_t tamper_status) {
    xSemaphoreTake (this->lock, portMAX_DELAY);
    this->_tamper_status = tamper_status;
    xSemaphoreGive (this->lock);
}

uint8_t Status::GetSOSStatus () { return this->_sos_status; }
void Status::SetSOSStatus (uint8_t sos_status) {
    xSemaphoreTake (this->lock, portMAX_DELAY);
    this->_sos_status = sos_status;
    xSemaphoreGive (this->lock);
}

uint8_t Status::GetBLEStatus () { return this->_BLE_status; }
void Status::SetBLEStatus (uint8_t BLE_status) {
    xSemaphoreTake (this->lock, portMAX_DELAY);
    this->_BLE_status = BLE_status;
    xSemaphoreGive (this->lock);
}


std::string Status::get_BLE_server () { return this->_BLE_server; }
void Status::set_BLE_server (std::string BLE_server) {
    xSemaphoreTake (this->lock, portMAX_DELAY);
    this->_BLE_server = BLE_server;
    xSemaphoreGive (this->lock);
}
