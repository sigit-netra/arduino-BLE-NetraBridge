#include "AT.h"
#include "ctre.hpp"

// clang-format off
// TODO: Decide if we want to replace regex ([0,1]{1}) with \d+
// constexpr is C++11 feature
static constexpr inline ctll::fixed_string AT_ONLY                 = "^AT$";
static constexpr inline ctll::fixed_string AT_SEND                 = "^AT\\+\\+SEND=([0,1]{1}):(.+)$";
static constexpr inline ctll::fixed_string AT_OUTBOX               = "^AT\\+\\+OUTBOX=\\?$";
static constexpr inline ctll::fixed_string AT_DELOUTBOX            = "^AT\\+\\+DELOUTBOX=(-?\\d+)$";
static constexpr inline ctll::fixed_string AT_REFRESH              = "^AT\\+\\+REFRESH$";
static constexpr inline ctll::fixed_string AT_INBOX                = "^AT\\+\\+INBOX=\\?$";
static constexpr inline ctll::fixed_string AT_DELINBOX             = "^AT\\+\\+DELINBOX=(-?\\d+)$";
static constexpr inline ctll::fixed_string AT_STATUS               = "^AT\\+\\+STATUS=\\?$";
static constexpr inline ctll::fixed_string AT_TRACKER              = "^AT\\+\\+TRACKER=\\?$";
static constexpr inline ctll::fixed_string AT_ABOUT                = "^AT\\+\\+ABOUT=\\?$";
static constexpr inline ctll::fixed_string AT_SETSTATUSUPDATER_get = "^AT\\+\\+SETSTATUSUPDATER=\\?$";
static constexpr inline ctll::fixed_string AT_SETSTATUSUPDATER_set = "^AT\\+\\+SETSTATUSUPDATER=([0,1]{1}),(\\d+)$";
static constexpr inline ctll::fixed_string AT_SETTRACKER_get       = "^AT\\+\\+SETTRACKER=\\?$";
static constexpr inline ctll::fixed_string AT_SETTRACKER_set       = "^AT\\+\\+SETTRACKER=([0,1]{1}),(\\d+)$";
static constexpr inline ctll::fixed_string AT_WIFI                 = "^AT\\+\\+WIFI=([0,1]{1})$";
static constexpr inline ctll::fixed_string AT_SETALERT_get         = "^AT\\+\\+SETALERT=\\?$";
static constexpr inline ctll::fixed_string AT_SETALERT_set         = "^AT\\+\\+SETALERT=([0,1]{1}),([0,1]{1}),([0,1]{1}),(\\d+),(\\d+)$";
static constexpr inline ctll::fixed_string AT_SETWIFI_get          = "^AT\\+\\+SETWIFI=\\?$";
static constexpr inline ctll::fixed_string AT_SETWIFI_set          = "^AT\\+\\+SETWIFI=(\\S+),(.+)$";
static constexpr inline ctll::fixed_string AT_SOS                  = "^AT\\+\\+SOS=(\\d+)";
static constexpr inline ctll::fixed_string AT_POSTACTIVATION       = "^AT\\+\\+POSTACTIVATION=(\\d+):(.+)$";
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
        sprintf (tmp, "%d,%d,%d,%.6f,%.6f,%d,%d", 1, 1, status->GetBLEStatus (),
                 1.000, 1.000, 0, 0);
        //  status->GetBLEStatus (), 0.000, 0.000, 0, 0);

        std::string result;
        result += "++STATUS:";
        result += tmp;
        result += "\r\n\r\nOK\r\n";

        // printf (result.c_str());


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

        if (valueInt == 0) {
            // _msgbox.outboxCancelSOS ();
            status->SetSOSStatus (10);
            printf ("SetSOSStatus CANCEL SOS...\r\n");


        } else if (valueInt == 1) {
            // _msgbox.outboxSendSOS ();
            status->SetSOSStatus (5);
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

    // printf ("--%s--\r\n", input.c_str ());
    // printf ("AT_ERROR\r\n");
    return "AT_ERROR\r\n";
}