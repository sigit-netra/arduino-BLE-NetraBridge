#include "netracubeBLE.h"

void netracubeBLE::task_init () {
    // xTaskCreatePinnedToCore (this->task_at_commands_wrapper, "task_at_commands",
    //                          4 * 1024, this, 4, NULL, 0);

    xTaskCreatePinnedToCore (this->task_button_wrapper, "task_button_wrapper",
                             4 * 1024, this, 4, NULL, 0);
}

void netracubeBLE::task_at_commands () {
    Serial2.begin (9600); // Inisialisasi Serial2

    while (true) {
        // Jika ada data yang tersedia di Serial2
        if (Serial2.available () > 0) {
            std::string input = Serial2.readStringUntil ('\n').c_str ();

            // Hapus karakter '\r' dan '\n' dari input
            input.erase (std::remove (input.begin (), input.end (), '\r'), input.end ());
            input.erase (std::remove (input.begin (), input.end (), '\n'), input.end ());

            std::string output = _at.processCommand (input);
            Serial2.println (output.c_str ()); // Kirim hasil ke Serial2

            // Tambahkan delay kecil untuk mencegah blocking task terlalu lama
            vTaskDelay (10 / portTICK_PERIOD_MS); // Delay 10ms
        }

        // Reset watchdog timer
        // esp_task_wdt_reset();

        // Tambahkan delay untuk memberi waktu pada sistem
        vTaskDelay (10 / portTICK_PERIOD_MS); // Delay 10ms agar task lain punya waktu
        //  yield();  // Memaksa scheduler untuk menjalankan task lain
    }
}


void netracubeBLE::task_button () {
    pinMode (I_BUTTON_SOS, INPUT_PULLUP);

    gpio_pad_pullup (I_BUTTON_SOS);
    gpio_set_direction (I_BUTTON_SOS, GPIO_MODE_INPUT);
    auto _status   = deviceStatus::GetInstance ();
    int sos_status = 0;
    while (1) {
        // Serial.println (gpio_get_level (I_BUTTON_SOS));
        if (!gpio_get_level (I_BUTTON_SOS) == 1) {
            vTaskDelay (100 / portTICK_PERIOD_MS);
            if (!gpio_get_level (I_BUTTON_SOS) == 1) {
                vTaskDelay (3000 / portTICK_PERIOD_MS);
                if (!gpio_get_level (I_BUTTON_SOS) == 1) {
                    if (!_status->get_button_sos_status ()) {
                        printf ("SOS...\n");
                        _status->set_button_sos_status (1);
                        _status->set_sos_status (99);
                        _status->set_ble_ack_status (0);
                        vTaskDelay (60000 / portTICK_PERIOD_MS);
                        continue;
                    }
                } else if (!gpio_get_level (I_BUTTON_SOS) == 0) {
                    if (_status->get_button_sos_status ()) {
                        printf ("CANCEL SOS...\n");
                        _status->set_button_sos_status (0);
                        _status->set_sos_status (100);
                        _status->set_ble_ack_status (0);
                        vTaskDelay (60000 / portTICK_PERIOD_MS);
                        continue;
                    }
                }
            }
        }
        vTaskDelay (50 / portTICK_PERIOD_MS);
    }
}
