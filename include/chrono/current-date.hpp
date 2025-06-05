#ifndef DATETIME_HPP_
#define DATETIME_HPP_

#include <string>
#include <chrono>
#include <ctime>

namespace cpp {
    //namespace date_time {

    std::string CurrentDate() {
        const std::chrono::time_point now{ std::chrono::system_clock::now() };
        const std::chrono::year_month_day current_date{ std::chrono::floor<std::chrono::days>(now) };
        const int year{ static_cast<int>(current_date.year()) };
        const unsigned month{ static_cast<unsigned>(current_date.month()) };
        const unsigned day{ static_cast<unsigned>(current_date.day()) };
        const std::string text{ std::to_string(year) + '.' + std::to_string(month) + '.' + std::to_string(day) };
        return text;
    }

} // !namespace cpp

#endif // !DATETIME_HPP_