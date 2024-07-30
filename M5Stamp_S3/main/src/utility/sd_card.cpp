#include "sd_card.hpp"

#include <SPI.h>
#include <Wire.h>

#include "logger.hpp"

namespace Utility
{

    bool SDCard::init(uint8_t txd_pin, uint8_t rxd_pin, const unsigned long& timeout_ms)
    {
        logger.info(F("[SDCard] Initializing..."));

        Serial2.begin(9600, SERIAL_8N1, txd_pin, rxd_pin);
        unsigned long start = millis();
        while (!Serial2) {
            if (millis() - start > timeout_ms) {
                return false;
            }
        }
        delay(1000);
        logger.info(F("[SDCard] Initialized"));
        return true;
    }

    void SDCard::write_impl(float last)
    {
        Serial2.println(last, 6);
    }

}  // namespace Utility