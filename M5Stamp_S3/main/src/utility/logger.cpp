#include "logger.hpp"

#include "computer.hpp"

namespace Utility
{

    void Logger::setDebug(const bool& is_debug)
    {
        is_debug_ = is_debug;
        this->info(F("[Logger] Debug mode is"), is_debug_ ? F("enabled.") : F("disabled."));
    }

    bool Logger::enableComputer(const unsigned long& baud_rate)
    {
        use_computer_ = true;
        if (!Computer::init(baud_rate)) {
            use_computer_ = false;
            return false;
        }
        this->info(F("[Logger] Start logging to computer"));
        return true;
    }

    bool Logger::enableSDCard(uint8_t txd_pin, uint8_t rxd_pin)
    {
        if (!SDCard::init(txd_pin, rxd_pin)) {
            this->warning(F("[Logger] Logging to SD card is disabled"));
            return false;
        }
        use_sd_card_ = true;
        SDCard::write(F("============================================================"));
        this->info(F("[Logger] Start logging to SD card"));
        return true;
    }

    String Logger::timestamp_() const
    {
        float sec = float(millis()) / 1000.0;
        char buffer[9];
        dtostrf(sec, 7, 2, buffer);
        return String(buffer);
    }

    Logger logger;

}  // namespace Utility

using Utility::logger;
