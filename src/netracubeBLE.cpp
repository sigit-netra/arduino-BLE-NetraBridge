#include "netracubeBLE.h"

void netracubeBLE::task_init () {
    xTaskCreatePinnedToCore (this->task_at_commands_wrapper, "task_at_commands", 4 * 1024,
                             this, 4, NULL, 0);
}


void netracubeBLE::task_at_commands () { 

    // plus baudrate and pin assigment
    Serial2.begin (9600);
    // Serial2.begin (9600, SERIAL_8N1, 16, 17);

    while (true) {
        if (Serial2.available () > 0) {
            std::string input = Serial2.readStringUntil ('\n').c_str ();

            // Trim any '\r' and '\n' character from the input
            input.erase (std::remove (input.begin (), input.end (), '\r'), input.end ());
            input.erase (std::remove (input.begin (), input.end (), '\n'), input.end ());

            std::string output = _at.processCommand (input);
            Serial2.println (output.c_str ());
        }
        delay(10);
    }
 }