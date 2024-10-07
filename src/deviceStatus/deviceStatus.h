/**
 * @file deviceStatus.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-12-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include "Arduino.h"
#include "mutex"
#include "stdint.h"
#include "string"
#include <stdio.h>
#include <string.h>


class deviceStatus {
  public:
    // deviceStatus (/* args */);
    deviceStatus ();

    static deviceStatus* GetInstance ();

    uint8_t get_init_wifi ();
    void set_init_wifi (uint8_t u8);
    uint8_t get_p_wifi ();
    void set_p_wifi (uint8_t p_wifi);
    uint8_t get_p_gsm ();
    void set_p_gsm (uint8_t p_gsm);
    uint8_t get_p_gps ();
    void set_p_gps (uint8_t p_gps);
    uint8_t get_p_s_env ();
    void set_p_s_env (uint8_t p_s_env);
    uint8_t get_p_s_gyro ();
    void set_p_s_gyro (uint8_t p_s_gyro);
    uint8_t get_p_s_vibra ();
    void set_p_s_vibra (uint8_t p_s_vibra);
    uint8_t get_p_s_ldr ();
    void set_p_s_ldr (uint8_t p_s_ldr);
    uint8_t get_p_s_batt_devider ();
    void set_p_s_batt_devider (uint8_t p_s_batt_devider);
    uint8_t get_p_o_buzzer ();
    void set_p_o_buzzer (uint8_t p_o_buzzer);

    std::string get_serial_number ();
    void set_serial_number (std::string serial_number);
    std::string get_asset_type ();
    void set_asset_type (std::string asset_type);
    uint32_t get_device_type ();
    void set_device_type (uint32_t device_type);
    uint32_t get_production_date ();
    void set_production_date (uint32_t production_date);
    uint32_t get_runTime ();
    void set_runTime (uint32_t runTime);
    uint32_t get_last_usage ();
    void set_last_usage (uint32_t last_usage);
    // esp data
    std::string get_imei ();
    void set_imei (std::string imei);

    std::string get_wifi_ap_ssid ();
    void set_wifi_ap_ssid (std::string wifi_AP_ssid);

    std::string get_wifi_ap_pswd ();
    void set_wifi_ap_pswd (std::string wifi_AP_pswd);

    std::string get_wifi_sta_ssid ();
    void set_wifi_sta_ssid (std::string wifi_STA_ssid);

    std::string get_wifi_sta_pswd ();
    void set_wifi_sta_pswd (std::string wifi_STA_pswd);

    uint8_t get_percent_battery ();
    void set_percent_battery (uint8_t percent_battery);

    uint16_t get_val_battery ();
    void set_val_battery (uint16_t val_battery);

    uint16_t get_val_tamper ();
    void set_val_tamper (uint16_t val_tamper);

    bool get_status_tamper ();
    void set_status_tamper (bool status_tamper);

    // power data
    bool get_pwr_standby ();
    void set_pwr_standby (bool pwr_standby);

    bool get_pwr_acc ();
    void set_pwr_acc (bool pwr_acc);

    bool get_pwr_engine ();
    void set_pwr_engine (bool pwr_engine);

    // uint32_t get_time_now ();
    // void set_time_now (uint32_t time_now);

    uint32_t get_running_time ();
    void set_running_time (uint32_t running_time);

    uint32_t get_interval_sending ();
    void set_interval_sending (uint32_t interval_sending);

    // gps data
    double get_latitude ();
    void set_latitude (double latitude);

    double get_longitude ();
    void set_longitude (double longitude);

    double get_point_latitude ();
    void set_point_latitude (double point_latitude);

    double get_point_longitude ();
    void set_point_longitude (double point_longitude);

    double get_speed ();
    void set_speed (double speed);

    double get_course ();
    void set_course (double course);

    uint32_t get_radius ();
    void set_radius (uint32_t radius);

    uint32_t get_altitude ();
    void set_altitude (uint32_t altitude);

    uint32_t get_sat_value ();
    void set_sat_value (uint32_t sat_value);


    // env data
    float get_temperature ();
    void set_temperature (float temperature);

    float get_humidity ();
    void set_humidity (float humidity);

    // interval

    uint32_t get_i_msg_tracker ();
    void set_i_msg_tracker (uint32_t i_msg_tracker);
    uint32_t get_i_msg_alert ();
    void set_i_msg_alert (uint32_t i_msg_tracker);
    uint32_t get_i_msg_sos ();
    void set_i_msg_sos (uint32_t i_msg_sos);

    // Generate encrypted msg

    void get_key_AES256 (uint8_t* key);
    void set_key_AES256 (const String& keyString);

    // BLE env
    std::string get_BLE_server ();
    void set_BLE_server (std::string s);

    uint8_t get_ble_status ();
    void set_ble_status (uint8_t u8);

    uint8_t get_sos_status ();
    void set_sos_status (uint8_t u8);

    uint8_t get_tamper_BLEM ();
    void set_tamper_BLEM (uint8_t u8);

    uint8_t get_tamper_NBLITE ();
    void set_tamper_NBLITE (uint8_t u8);

    uint8_t get_button_sos_status ();
    void set_button_sos_status (uint8_t u8);

    std::string get_json_ies_cfg ();
    void set_json_ies_cfg (std::string s);

    std::string get_json_ies_identity ();
    void set_json_ies_identity (std::string s);

    // Encryption data

    uint8_t* get_r_msg_sos ();
    uint8_t* get_r_msg_cancel_sos ();
    uint8_t* get_r_msg_tamper ();
    uint8_t* get_r_get_cfg ();

    uint8_t* get_r_set_30m ();
    uint8_t* get_r_set_60m ();


    uint8_t* get_process ();


    const uint8_t* get_e_msg_sos ();
    void set_e_msg_sos (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_e_msg_cancel_sos ();
    void set_e_msg_cancel_sos (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_e_msg_tamper ();
    void set_e_msg_tamper (const uint8_t* u8, size_t payload_size);


    const uint8_t* get_e_ies_cfg ();
    void set_e_ies_cfg (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_ble_ack_data ();
    void set_ble_ack_data (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_ble_general_config ();
    void set_ble_general_config (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_ble_user_msg ();
    void set_ble_user_msg (const uint8_t* u8, size_t payload_size);

    const uint8_t* get_ble_user_location_msg ();
    void set_ble_user_location_msg (const uint8_t* u8, size_t payload_size);

    uint8_t get_ble_ack_status ();
    void set_ble_ack_status (uint8_t u8);

    uint8_t get_ack_gen_config ();
    void set_ack_gen_config (uint8_t u8);

    uint8_t get_act_gen_config ();
    void set_act_gen_config (uint8_t u8);

    uint8_t get_act_user_msg ();
    void set_act_user_msg (uint8_t u8);

    uint8_t get_act_user_location_msg ();
    void set_act_user_location_msg (uint8_t u8);

    uint8_t get_act_location_only ();
    void set_act_location_only (uint8_t u8);

    uint8_t get_act_eng_msg ();
    void set_act_eng_msg (uint8_t u8);

    uint8_t* get_e_set_30m ();
    void set_e_set_30m (const uint8_t* u8, size_t payload_size);

    uint8_t* get_e_set_60m ();
    void set_e_set_60m (const uint8_t* u8, size_t payload_size);

    uint8_t hexCharToByte (char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    }


  private:
    static deviceStatus* instance;

    // device_data
    uint8_t _init_wifi        = 0;
    uint8_t _p_wifi           = 0;
    uint8_t _p_gsm            = 0;
    uint8_t _p_gps            = 0;
    uint8_t _p_s_env          = 0;
    uint8_t _p_s_gyro         = 0;
    uint8_t _p_s_vibra        = 0;
    uint8_t _p_s_ldr          = 0;
    uint8_t _p_s_batt_devider = 0;
    uint8_t _p_o_buzzer       = 0;


    // interval

    uint32_t _i_msg_tracker = 0;
    uint32_t _i_msg_alert   = 0;
    uint32_t _i_msg_sos     = 0;
    // asset_data
    std::string _serial_number;
    std::string _asset_type;
    uint32_t _device_type     = 0;
    uint32_t _production_date = 0;
    uint32_t _runTime         = 0;
    uint32_t _last_usage      = 0;
    //------------------------------

    std::string _imei          = "";
    std::string _wifi_AP_ssid  = "Netra - BLEM";
    std::string _wifi_AP_pswd  = "tothemoon";
    std::string _wifi_STA_ssid = "nissin-wafers";
    std::string _wifi_STA_pswd = "nissiN!90";

    // esp data
    uint16_t _val_battery      = 0;
    uint8_t _percent_battery   = 0;
    uint16_t _val_tamper       = 0;
    bool _status_tamper        = false;
    bool _pwr_standby          = false;
    bool _pwr_acc              = false;
    bool _pwr_engine           = false;
    uint32_t _time_now         = 0;
    uint32_t _running_time     = 0;
    uint32_t _interval_sending = 0;


    // gps data
    double _latitude        = 0.0; //-6.2788284;//0.0;
    double _longitude       = 0.0; // 106.816666;//0.0;
    double _point_latitude  = 0.0; //-6.2788284;//0.0;
    double _point_longitude = 0.0; // 106.816666;//0.0;
    double _speed           = 0.0;
    double _course          = 0.0;
    uint32_t _radius        = 0;
    uint32_t _altitude      = 0;
    uint32_t _sat_value     = 0;

    // env data
    float _temperature = 0.0;
    float _humidity    = 0.0;

    // BLE env
    std::string _BLE_server = "70076190";
    std::string _json_get_cfg =
    "{\"type\":72,\"messageLength\":30,\"timestamp\":4294967295,"
    "\"primaryIntervals\":[14400,0,0,0,0,0,0,0],\"engineeringMessageInterval\":"
    "{\"intervalInHours\":336},\"vbmrMotionConfig\":{\"mode\":0,"
    "\"inMotionIntervalMinutes\":0},\"mailboxCheckInterval\":{"
    "\"onlyCheckOnTX\":false,\"pollIntervalInHours\":3},"
    "\"powerSaveConfiguration\":{\"countMsgsAfterDepletion\":4,"
    "\"hoursBetweenPositionReports\":12},\"gpsBasedMotionReportingConfig\":{"
    "\"enabled\":false,\"checkIntervalInMinutes\":0,\"homeRatio\":0,"
    "\"awayRatio\":0},\"advanced\":{\"startStopConfig\":0,"
    "\"iridiumRetryCount\":3,\"enableVBMRDuringPowerSave\":false}}";

    std::string _json_ies_identity =
    "{\"esn\":\"70076190\",\"aes256\":"
    "\"39F4553CE5FA10DF0318E28A232AC6BCA32D27FF33E55F38EFEB1ADB0909E88E\"}";

    uint8_t _tamper_BLEM       = 0;
    uint8_t _tamper_NBLITE     = 0;
    uint8_t _sos_status        = 0;
    uint8_t _ble_status        = 0;
    uint8_t _button_sos_status = 0;
    uint8_t _ble_ack_status    = 0;

    uint8_t _ack_gen_config            = 0;
    uint8_t _act_gen_config            = 0;
    uint8_t _act_ble_user_msg          = 0;
    uint8_t _act_ble_user_location_msg = 0;
    uint8_t _act_ble_location_only     = 0;
    uint8_t _act_ble_eng_msg           = 0;

    // Generate encrypted msg
    uint8_t _key_AES256[32];
    uint8_t _encrypted_msg_sos[20];
    uint8_t _encrypted_msg_cancel_sos[20];
    uint8_t _encrypted_msg_tamper[20];
    uint8_t _encrypted_msg_get_cfg[40];
    uint8_t _encrypted_msg_set_30m[40];
    uint8_t _encrypted_msg_set_60m[40];
    uint8_t _ble_ack_data[100];
    uint8_t _ble_general_config[50];
    uint8_t _ble_user_msg[124];
    uint8_t _ble_user_location_msg[124];

    uint8_t _msg_process[1] = { 0x01 };

    uint8_t msg_sos[20] = { 0x24, 0x07, 0x09, 0x48, 0xe4, 0x92, 0x39, 0x24,
                            0x8e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t msg_cancel_sos[20] = { 0x24, 0x07, 0x09, 0x08, 0x03, 0x42,
                                   0x10, 0xb4, 0x8e, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff };
    uint8_t msg_tamper[20] = { 0x24, 0x07, 0x09, 0x4c, 0x03, 0x0f, 0x11, 0x13,
                               0x8d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t msg_get_config[20]        = { 0x19, 0x01, 0x48, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff };
    uint8_t msg_send_eng_msg[20]      = { 0x22, 0x00, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                     0xff, 0xff, 0xff, 0xff };
    uint8_t msg_send_location_msg[20] = { 0x23, 0x00, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff };

    uint8_t msg_set_60m[40] = { 0x2A, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x3C,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00,
                                0x80, 0xFF, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x0D };

    uint8_t msg_set_30m[40] = { 0x2A, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1E,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00,
                                0x80, 0xFF, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x0D };

    uint8_t msg_int60m[35]  = { 0x2a, 0x1e, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00,
                               0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07 };


    std::mutex _config_vals_mtx;
};
