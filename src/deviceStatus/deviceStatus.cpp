/**
 * @file deviceStatus.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-12-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "deviceStatus.h"
deviceStatus* deviceStatus::instance = NULL;
deviceStatus* deviceStatus::GetInstance () {
    if (!instance) {
        instance = new deviceStatus ();
        return instance;
    } else {
        return instance;
    }
}
deviceStatus::deviceStatus (/* args */) {}

void deviceStatus::get_key_AES256 (uint8_t* key) {
    memcpy (key, _key_AES256, 32);
}
void deviceStatus::set_key_AES256 (const String& keyString) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    for (int i = 0; i < 32; i++) {
        _key_AES256[i] = (hexCharToByte (keyString[2 * i]) << 4) |
                         hexCharToByte (keyString[2 * i + 1]);
    }
    // memcpy(key_AES256, key, 32);
}

std::string deviceStatus::get_BLE_server () { return _BLE_server; }
void deviceStatus::set_BLE_server (std::string s) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _BLE_server = s;
}

uint8_t deviceStatus::get_ble_status () { return _ble_status; }
void deviceStatus::set_ble_status (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _ble_status = u8;
}

uint8_t deviceStatus::get_sos_status () { return _sos_status; }
void deviceStatus::set_sos_status (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _sos_status = u8;
}

uint8_t deviceStatus::get_tamper_BLEM () { return _tamper_BLEM; }
void deviceStatus::set_tamper_BLEM (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _tamper_BLEM = u8;
}

uint8_t deviceStatus::get_tamper_NBLITE () { return _tamper_NBLITE; }
void deviceStatus::set_tamper_NBLITE (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _tamper_NBLITE = u8;
}

uint8_t deviceStatus::get_button_sos_status () { return _button_sos_status; }
void deviceStatus::set_button_sos_status (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _button_sos_status = u8;
}

//---------------------------------------------

uint8_t* deviceStatus::get_r_msg_sos () { return msg_sos; }
uint8_t* deviceStatus::get_r_msg_cancel_sos () { return msg_cancel_sos; }
uint8_t* deviceStatus::get_r_msg_tamper () { return msg_tamper; }
uint8_t* deviceStatus::get_r_get_cfg () { return msg_get_config; }
uint8_t* deviceStatus::get_r_set_30m () { return msg_set_30m; }
uint8_t* deviceStatus::get_r_set_60m () { return msg_set_60m; }

uint8_t* deviceStatus::get_process () { return _msg_process; }


uint8_t* deviceStatus::get_e_set_30m () { return _encrypted_msg_set_30m; }
void deviceStatus::set_e_set_30m (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 40) {
        memcpy (_encrypted_msg_set_30m, u8, payload_size);
    }
}

 uint8_t* deviceStatus::get_e_set_60m () { return _encrypted_msg_set_60m; }
void deviceStatus::set_e_set_60m (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 40) {
        memcpy (_encrypted_msg_set_60m, u8, payload_size);
    }
}

const uint8_t* deviceStatus::get_e_msg_sos () { return _encrypted_msg_sos; }
void deviceStatus::set_e_msg_sos (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 20) {
        memcpy (_encrypted_msg_sos, u8, payload_size);
    }
}

const uint8_t* deviceStatus::get_e_msg_cancel_sos () {
    return _encrypted_msg_cancel_sos;
}
void deviceStatus::set_e_msg_cancel_sos (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 20) {
        memcpy (_encrypted_msg_cancel_sos, u8, payload_size);
    }
}

const uint8_t* deviceStatus::get_e_msg_tamper () {
    return _encrypted_msg_tamper;
}
void deviceStatus::set_e_msg_tamper (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 20) {
        memcpy (_encrypted_msg_tamper, u8, payload_size);
    }
}


uint8_t deviceStatus::get_ble_ack_status () { return _ble_ack_status; }
void deviceStatus::set_ble_ack_status (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _ble_ack_status = u8;
}

// get config
const uint8_t* deviceStatus::get_e_ies_cfg () { return _encrypted_msg_get_cfg; }
void deviceStatus::set_e_ies_cfg (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 20) {
        memcpy (_encrypted_msg_get_cfg, u8, payload_size);
    }
}

// commands
const uint8_t* deviceStatus::get_ble_ack_data () { return _ble_ack_data; }
void deviceStatus::set_ble_ack_data (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 100) {
        memcpy (_ble_ack_data, u8, payload_size);
    }
}

const uint8_t* deviceStatus::get_ble_general_config () {
    return _ble_general_config;
}
void deviceStatus::set_ble_general_config (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 50) {
        memcpy (_ble_general_config, u8, payload_size);
    }
}

const uint8_t* deviceStatus::get_ble_user_msg () { return _ble_user_msg; }
void deviceStatus::set_ble_user_msg (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 124) {
        memcpy (_ble_user_msg, u8, payload_size);
    }
}
const uint8_t* deviceStatus::get_ble_user_location_msg () {
    return _ble_user_location_msg;
}
void deviceStatus::set_ble_user_location_msg (const uint8_t* u8, size_t payload_size) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    if (payload_size <= 124) {
        memcpy (_ble_user_location_msg, u8, payload_size);
    }
}

//

std::string deviceStatus::get_json_ies_cfg () { return _json_get_cfg; }
void deviceStatus::set_json_ies_cfg (std::string s) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _json_get_cfg = s;
}

std::string deviceStatus::get_json_ies_identity () {
    return _json_ies_identity;
}
void deviceStatus::set_json_ies_identity (std::string s) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _json_ies_identity = s;
}

uint8_t deviceStatus::get_ack_gen_config () { return _ack_gen_config; }
void deviceStatus::set_ack_gen_config (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _ack_gen_config = u8;
}

uint8_t deviceStatus::get_act_gen_config () { return _act_gen_config; }
void deviceStatus::set_act_gen_config (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _act_gen_config = u8;
}

uint8_t deviceStatus::get_act_user_msg () { return _act_ble_user_msg; }
void deviceStatus::set_act_user_msg (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _act_ble_user_msg = u8;
}
uint8_t deviceStatus::get_act_user_location_msg () {
    return _act_ble_user_location_msg;
}
void deviceStatus::set_act_user_location_msg (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _act_ble_user_location_msg = u8;
}

uint8_t deviceStatus::get_act_location_only () {
    return _act_ble_location_only;
}
void deviceStatus::set_act_location_only (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _act_ble_location_only = u8;
}

uint8_t deviceStatus::get_act_eng_msg () { return _act_ble_eng_msg; }
void deviceStatus::set_act_eng_msg (uint8_t u8) {
    std::lock_guard<std::mutex> locker (_config_vals_mtx);
    _act_ble_eng_msg = u8;
}