#pragma once

// #include "SPIFFS.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "tools/tools.h"
#include "mutex"
// #include "spiffs_config.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "SPIFFS.h"
#include "deviceStatus/deviceStatus.h"
#include "storageInterface.h"
#include "string"
#include <WiFi.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <time.h>


class localSpiffs : public storageInterface {
  private:
    // deviceStatus _status;

    long timezone    = 7;
    byte daysavetime = 1;

    void listDir (fs::FS& fs, const char* dirname, uint8_t levels) {
        Serial.printf ("Listing directory: %s\n", dirname);

        File root = fs.open (dirname);
        if (!root) {
            Serial.println ("Failed to open directory");
            return;
        }
        if (!root.isDirectory ()) {
            Serial.println ("Not a directory");
            return;
        }

        File file = root.openNextFile ();
        while (file) {
            if (file.isDirectory ()) {
                Serial.print ("  DIR : ");
                Serial.print (file.name ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
                if (levels) {
                    listDir (fs, file.path (), levels - 1);
                }
            } else {
                Serial.print ("  FILE: ");
                Serial.print (file.name ());
                Serial.print ("  SIZE: ");
                Serial.print (file.size ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
            }
            file = root.openNextFile ();
        }
    }

    void createDir (fs::FS& fs, const char* path) {
        Serial.printf ("Creating Dir: %s\n", path);
        if (fs.mkdir (path)) {
            Serial.println ("Dir created");
        } else {
            Serial.println ("mkdir failed");
        }
    }

    void removeDir (fs::FS& fs, const char* path) {
        Serial.printf ("Removing Dir: %s\n", path);
        if (fs.rmdir (path)) {
            Serial.println ("Dir removed");
        } else {
            Serial.println ("rmdir failed");
        }
    }

    void readFile (fs::FS& fs, const char* path) {
        Serial.printf ("Reading file: %s\n", path);

        File file = fs.open (path);
        if (!file) {
            Serial.println ("Failed to open file for reading");
            return;
        }

        Serial.print ("Read from file: ");
        while (file.available ()) {
            Serial.write (file.read ());
        }
        file.close ();
    }

    void writeFile (fs::FS& fs, const char* path, const char* message) {
        Serial.printf ("Writing file: %s\n", path);

        File file = fs.open (path, FILE_WRITE);
        if (!file) {
            Serial.println ("Failed to open file for writing");
            return;
        }
        if (file.print (message)) {
            Serial.println ("File written");
        } else {
            Serial.println ("Write failed");
        }
        file.close ();
    }

    void appendFile (fs::FS& fs, const char* path, const char* message) {
        Serial.printf ("Appending to file: %s\n", path);

        File file = fs.open (path, FILE_APPEND);
        if (!file) {
            Serial.println ("Failed to open file for appending");
            return;
        }
        if (file.print (message)) {
            Serial.println ("Message appended");
        } else {
            Serial.println ("Append failed");
        }
        file.close ();
    }

    void renameFile (fs::FS& fs, const char* path1, const char* path2) {
        Serial.printf ("Renaming file %s to %s\n", path1, path2);
        if (fs.rename (path1, path2)) {
            Serial.println ("File renamed");
        } else {
            Serial.println ("Rename failed");
        }
    }

    void deleteFile (fs::FS& fs, const char* path) {
        Serial.printf ("Deleting file: %s\n", path);
        if (fs.remove (path)) {
            Serial.println ("File deleted");
        } else {
            Serial.println ("Delete failed");
        }
    }


  public:
    void init ();
    void get_BLEM_data (const char* path);

    // bool remove_file (const char* path);

    std::string readFileString (fs::FS& fs, const char* path) {
        Serial.printf ("Reading file: %s\n", path);
        std::string debugLogData;
        if (SPIFFS.exists (path)) {
            File f = SPIFFS.open (path);
            if (f && f.size ()) {
                while (f.available ()) {
                    debugLogData += char (f.read ());
                }
                f.close ();
            } else {
                Serial.println ("File is empty");
            }
        }
        return debugLogData;
    }

    void writeFileString (fs::FS& fs, const char* path, const char* message) {
        Serial.printf ("Writing file: %s\n", path);

        File file = fs.open (path, FILE_WRITE);
        if (!file) {
            Serial.println ("Failed to open file for writing");
            return;
        }
        if (!file.print (message)) {
            Serial.println ("Write failed");
        }
        file.close ();
    }

    // See: https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.cpp#L60
    void writeFile2 (fs::FS& fs, const char* path, const char* message) {
        if (!fs.exists (path)) {
            if (strchr (path, '/')) {
                Serial.printf ("Create missing folders of: %s\r\n", path);
                char* pathStr = strdup (path);
                if (pathStr) {
                    char* ptr = strchr (pathStr, '/');
                    while (ptr) {
                        *ptr = 0;
                        fs.mkdir (pathStr);
                        *ptr = '/';
                        ptr  = strchr (ptr + 1, '/');
                    }
                }
                free (pathStr);
            }
        }

        Serial.printf ("Writing file to: %s\r\n", path);
        File file = fs.open (path, FILE_WRITE);
        if (!file) {
            Serial.println ("- failed to open file for writing");
            return;
        }
        if (file.print (message)) {
            Serial.println ("- file written");
        } else {
            Serial.println ("- write failed");
        }
        file.close ();
    }

    // See: https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.h#L149
    void deleteFile2 (fs::FS& fs, const char* path) {
        Serial.printf ("Deleting file and empty folders on path: %s\r\n", path);

        if (fs.remove (path)) {
            Serial.println ("- file deleted");
        } else {
            Serial.println ("- delete failed");
        }

        char* pathStr = strdup (path);
        if (pathStr) {
            char* ptr = strrchr (pathStr, '/');
            if (ptr) {
                Serial.printf ("Removing all empty folders on path: %s\r\n", path);
            }
            while (ptr) {
                *ptr = 0;
                fs.rmdir (pathStr);
                ptr = strrchr (pathStr, '/');
            }
            free (pathStr);
        }
    }

    void testFileIO (fs::FS& fs, const char* path) {
        Serial.printf ("Testing file I/O with %s\r\n", path);

        static uint8_t buf[512];
        size_t len = 0;
        File file  = fs.open (path, FILE_WRITE);
        if (!file) {
            Serial.println ("- failed to open file for writing");
            return;
        }

        size_t i;
        Serial.print ("- writing");
        uint32_t start = millis ();
        for (i = 0; i < 2048; i++) {
            if ((i & 0x001F) == 0x001F) {
                Serial.print (".");
            }
            file.write (buf, 512);
        }
        Serial.println ("");
        uint32_t end = millis () - start;
        Serial.printf (" - %u bytes written in %u ms\r\n", 2048 * 512, end);
        file.close ();

        file  = fs.open (path);
        start = millis ();
        end   = start;
        i     = 0;
        if (file && !file.isDirectory ()) {
            len         = file.size ();
            size_t flen = len;
            start       = millis ();
            Serial.print ("- reading");
            while (len) {
                size_t toRead = len;
                if (toRead > 512) {
                    toRead = 512;
                }
                file.read (buf, toRead);
                if ((i++ & 0x001F) == 0x001F) {
                    Serial.print (".");
                }
                len -= toRead;
            }
            Serial.println ("");
            end = millis () - start;
            Serial.printf ("- %u bytes read in %u ms\r\n", flen, end);
            file.close ();
        } else {
            Serial.println ("- failed to open file for reading");
        }
    }
    void checkListDir (fs::FS& fs, const char* dirname, uint8_t levels) {
        Serial.printf ("Listing directory: %s\n", dirname);

        File root = fs.open (dirname);
        if (!root) {
            Serial.println ("Failed to open directory");
            return;
        }
        if (!root.isDirectory ()) {
            Serial.println ("Not a directory");
            return;
        }

        File file = root.openNextFile ();
        if (file) {
            if (file.isDirectory ()) {
                Serial.print ("  DIR : ");
                Serial.print (file.name ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
                if (levels) {
                    listDir (fs, file.path (), levels - 1);
                }
            } else {
                Serial.print ("  FILE: ");
                Serial.print (file.name ());
                Serial.print ("  SIZE: ");
                Serial.print (file.size ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
                std::string filenameis = "/outbox/LOW/" + (std::string)file.name ();
                readFile (SPIFFS, filenameis.c_str ());
                // delay (3000);

                // deleteFile (SPIFFS, filenameis.c_str ());
            }
            file = root.openNextFile ();
        }
    }

    void msgAppendFile (const char* path, const char* message) {
        Serial.printf ("Appending to file: %s\n", path);

        File file = SPIFFS.open (path, FILE_APPEND);
        if (!file) {
            Serial.println ("Failed to open file for appending");
            return;
        }
        if (file.print (message)) {
            Serial.println ("Message appended");
        } else {
            Serial.println ("Append failed");
        }
        file.close ();
    }
    void getAllOutbox ( const char* dirname, uint8_t levels) {
        Serial.printf ("Listing directory: %s\n", dirname);

        File root = SPIFFS.open (dirname);
        if (!root) {
            Serial.println ("Failed to open directory");
            return;
        }
        if (!root.isDirectory ()) {
            Serial.println ("Not a directory");
            return;
        }

        File file = root.openNextFile ();
        while (file) {
            if (file.isDirectory ()) {
                Serial.print ("  DIR : ");
                Serial.print (file.name ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
                if (levels) {
                    listDir (SPIFFS, file.path (), levels - 1);
                }
            } else {
                Serial.print ("  FILE: ");
                Serial.print (file.name ());
                Serial.print ("  SIZE: ");
                Serial.print (file.size ());
                time_t t            = file.getLastWrite ();
                struct tm* tmstruct = localtime (&t);
                Serial.printf ("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",
                               (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1,
                               tmstruct->tm_mday, tmstruct->tm_hour,
                               tmstruct->tm_min, tmstruct->tm_sec);
                std::string filenameis = "/outbox/LOW/" + (std::string)file.name ();
                readFile (SPIFFS, filenameis.c_str ());
                // delay (3000);

                // deleteFile (SPIFFS, filenameis.c_str ());
            }
            file = root.openNextFile ();
        }
    }
};