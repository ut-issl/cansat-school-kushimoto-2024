#pragma once

#include <Arduino.h>

namespace Utility
{

    class Printer
    {

      public:

        template <class... Args>
        static void print(Args... args);

      private:

        template <class Last>
        static void print_impl(Last last);
        template <class Head, class... Args>
        static void print_impl(Head head, Args... args);

        static void print_impl(float last);
        template <class... Args>
        static void print_impl(float head, Args... args);
    };

    template <class... Args>
    void Printer::print(Args... args)
    {
        print_impl(args...);
    }

    template <class Last>
    void Printer::print_impl(Last last)

    {
        Serial.println(last);
    }

    template <class Head, class... Args>
    void Printer::print_impl(Head head, Args... args)
    {
        Serial.print(head);
        Serial.print(F(" "));
        print_impl(args...);
    }

    template <class... Args>
    void print(Args... args)
    {
        Printer::print(args...);
    }

    void Printer::print_impl(float last)
    {
        Serial.println(last, 6);
    }

    template <class... Args>
    void Printer::print_impl(float head, Args... args)
    {
        Serial.print(head, 6);
        Serial.print(F(" "));
        print_impl(args...);
    }

}  // namespace Utility
