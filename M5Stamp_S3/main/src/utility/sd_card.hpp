#pragma once

#include <Arduino.h>

#include "printer.hpp"

namespace Utility
{

    class SDCard
    {
      public:

        static bool init(uint8_t txd_pin, uint8_t rxd_pin, const unsigned long& timeout_ms = 10000);

        template <class... Args>
        static void write(Args... args);

      private:

        template <class Last>
        static void write_impl(Last last);
        template <class Head, class... Args>
        static void write_impl(Head head, Args... args);

        static void write_impl(float last);
        template <class... Args>
        static void write_impl(float head, Args... args);
    };

    template <class... Args>
    void SDCard::write(Args... args)
    {
        write_impl(args...);
    }

    template <class Last>
    void SDCard::write_impl(Last last)
    {
        Serial2.println(last);
    }

    template <class Head, class... Args>
    void SDCard::write_impl(Head head, Args... args)
    {
        Serial2.print(head);
        Serial2.print(F(" "));
        write_impl(args...);
    }

    void SDCard::write_impl(float last)
    {
        Serial2.println(last, 6);
    }

    template <class... Args>
    void SDCard::write_impl(float head, Args... args)
    {
        Serial2.print(head, 6);
        Serial2.print(F(" "));
        write_impl(args...);
    }

}  // namespace Utility
