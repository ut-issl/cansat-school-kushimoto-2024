#include "computer.hpp"

#include "logger.hpp"

namespace Utility
{

    bool Computer::init(const unsigned long& baud_rate, const unsigned long& timeout_ms)
    {
        Serial.begin(baud_rate);
        unsigned long start = millis();
        while (!Serial) {
            if (millis() - start > timeout_ms) {
                return false;
            }
        }
        delay(1000);
        Utility::logger.info(F("[Computer] Initialized"));
        return true;
    }

}  // namespace Utility
