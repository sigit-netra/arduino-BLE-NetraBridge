#include "AT.h"
#include "ctre.hpp"

// clang-format off
// TODO: Decide if we want to replace regex ([0,1]{1}) with \d+
// constexpr is C++11 feature
static constexpr inline ctll::fixed_string AT_ONLY                 = "^AT$";
static constexpr inline ctll::fixed_string AT_SEND                 = "^AT\\+\\+SEND=([0,1]{1}):(.+)$";
static constexpr inline ctll::fixed_string AT_SETBLE               = "^AT\\+\\+SETBLE=(\\w+):(\\w+)$";//^AT\+\+SETBLE=[0-9]+:[A-Za-z0-9]+$
static constexpr inline ctll::fixed_string AT_STATUS               = "^AT\\+\\+STATUS=\\?$";
static constexpr inline ctll::fixed_string AT_SOS                  = "^AT\\+\\+SOS=(\\d+)";
static constexpr inline ctll::fixed_string AT_SETIESINTERVAL      = "^AT\\+\\+SETIESINTERVAL=([0,1]{1}),(\\d+)$";

// clang-format on

/**
 * @brief Construct a new AT::AT object
 *
 * @param msgbox
 * @param settings
 * @param status
 */
AT::AT () { this->status = Status::GetInstance (); }

/**
 * @brief AT command string processor. Will call respective functions
 *
 * @param input Raw AT command string
 * @return std::string Raw AT response from the input
 */
std::string AT::processCommand (std::string input) {

    if (auto [whole] = ctre::match<AT_ONLY> (input); whole) {
        printf ("AT_ONLY\r\n");
        return "\r\nOK\r\n";
    }

    if (auto [whole] = ctre::match<AT_STATUS> (input); whole) {
        printf ("AT_STATUS\r\n");
        char tmp[50] = { 0 };
        auto _Status = deviceStatus::GetInstance ();

        sprintf (tmp, "%d,%d,%d,%.6f,%.6f,%d,%d", _Status->get_ble_status (),
                 _Status->get_ble_status (), _Status->get_ble_status (), 1.000,
                 1.000, 0, 0);

        std::string result;
        result += "++STATUS:";
        result += tmp;
        result += "\r\n\r\nOK\r\n";

        printf (result.c_str ());


        return result;
    }
    if (auto [whole, datatype, payload] = ctre::match<AT_SEND> (input); whole) {
        printf ("AT_SEND\r\n");
        status->SetTamperStatus (1);
        // auto result = _msgbox.AT_outboxSendMessage (payload.str (), datatype.str ());
        std::string resp;
        resp += "++SEND:";
        resp += std::to_string (1);
        resp += ',';
        resp += std::to_string (datatype.str () == "0" ? payload.str ().length () / 2 :
                                                         payload.str ().length ());
        resp += "\r\n\r\n";
        resp += 1 == true ? "OK" : "AT_ERROR";
        resp += "\r\n";

        return resp;
    }
    if (auto [whole, value] = ctre::match<AT_SOS> (input); whole) {
        printf ("AT_SOS\r\n");

        int valueInt = std::stoi (value.str ());
        auto _Status = deviceStatus::GetInstance ();

        if (valueInt == 0) {
            // _msgbox.outboxCancelSOS ();
            _Status->set_sos_status (100);
            printf ("SetSOSStatus CANCEL SOS...\r\n");


        } else if (valueInt == 1) {
            // _msgbox.outboxSendSOS ();
            _Status->set_sos_status (99);
            printf ("SetSOSStatus SOS...\r\n");
        }

        std::string result;

        if (valueInt == 0 || valueInt == 1) {
            result += "++SOS:0";
            result += "\r\n\r\nOK\r\n";
        } else {
            result += "++SOS:1";
            result += "\r\n\r\nAT_ERROR\r\n";
        }

        return result;
    }

    if (auto [whole, esn, aes256] = ctre::match<AT_SETBLE> (input); whole) {
        printf ("SETBLE\r\n");

        std::string split_esn;
        split_esn += (esn.str ());
        char* spt_esn = strtok ((char*)split_esn.c_str (), ":");

        StaticJsonDocument<200> doc;
        doc[String ("esn")]    = spt_esn;
        doc[String ("aes256")] = aes256.str ();

        std::string output;
        serializeJson (doc, output);

        aes.writeFileString (SPIFFS, "/BLEM_data.json", output.data ());

        xTaskCreatePinnedToCore (aes.restart_cmd_wrapper, "restart", 1 * 1024,
                                 this, 4, NULL, 0);

        std::string resp;
        resp += "++SETBLE:";
        resp += std::to_string (esn);
        resp += ',';
        resp += std::to_string (aes256.str ().length () / 2);
        resp += "\r\n\r\n";
        resp += 1 == true ? "OK" : "AT_ERROR";
        resp += "\r\n";
        return resp;
    }
    if (auto [whole, on, interval] = ctre::match<AT_SETIESINTERVAL> (input); whole) {
        printf ("AT_SETIESINTERVAL_set\r\n");

        bool onInt      = std::stoi (on.str ());
        int intervalInt = std::stoi (interval.str ());

        bool valid = (intervalInt >= 30 && interval <= 1440);
        status->SetIntervalStatus (1);

        if (!valid) {
            return "AT_ERROR";
        }

        std::string result;
        result += "++SETIESINTERVAL:";
        result += std::to_string (valid);
        result += "\r\n\r\n";
        result += valid ? "OK" : "AT_ERROR";
        result += "\r\n";

        return result;
    }

    // printf ("--%s--\r\n", input.c_str ());
    // printf ("AT_ERROR\r\n");
    return "AT_ERROR\r\n";
}