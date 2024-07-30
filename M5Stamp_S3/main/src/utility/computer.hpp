#pragma once

#include <Arduino.h>

namespace Utility
{

    namespace Computer
    {

        bool init(const unsigned long& baud_rate = 9600, const unsigned long& timeout_ms = 10000);

    }  // namespace Computer

}  // namespace Utility
