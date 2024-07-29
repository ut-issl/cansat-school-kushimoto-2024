#include "sd_card.hpp"

#include <SPI.h>
#include <Wire.h>

#include "logger.hpp"

namespace Utility
{

    bool SDCard::init(uint8_t txd_pin, uint8_t rxd_pin, const unsigned long& timeout_ms)
    {
        logger.info(F("[SDCard] Initializing..."));

        Serial.begin(9600, SERIAL_8N1, txd_pin, rxd_pin);
        unsigned long start = millis();
        while (!Serial) {
            if (millis() - start > timeout_ms) {
                return false;
            }
        }
        delay(1000);
        logger.info(F("[SDCard] Initialized"));
        return true;
    }

}  // namespace Utility