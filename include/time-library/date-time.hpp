#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <string>
#include <chrono>
#include <ctime>

namespace util {
//namespace date_time {

    /**  predefined char count for ctime() */
    constexpr int ctime_char_count{ 26 };

    /**
    * System date& time: Tue Aug 17 15 : 48 : 18 2021
    * As C Time. Format is defined & limited
    */
    inline std::string CurrentDateTimeC() {
        std::time_t time{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
        std::string time_str(ctime_char_count, ' ');

        ctime_s(&time_str[0], sizeof(time_str[0]) * time_str.size(), &time);
        time_str.erase(ctime_char_count - 2, 2); // last two characters are: \n\0
        return time_str;
    }

    /** UTC Time */
    inline std::string CurrentDateTimeUTC() {
        std::time_t time{ std::time(nullptr) };
        std::tm time_tm{};
        std::string time_str_gm(ctime_char_count, ' ');

        gmtime_s(&time_tm, &time);
        // TODO: [C:/Development/Projects/My_Libraries/date-time.h:31] (style) Obsolete function 'asctime_s' called. It is recommended to use 'strftime' instead. [asctime_sCalled]
        asctime_s(&time_str_gm[0], sizeof(time_str_gm[0]) * time_str_gm.size(), &time_tm);
        time_str_gm.erase(ctime_char_count - 2, 2); // last two characters are: \n\0
        return time_str_gm;
    }

    /** Local Time */
    inline std::string CurrentDateTimeLocal() {
        std::time_t time{ std::time(nullptr) };
        std::tm time_tm{};
        std::string time_str_loc(ctime_char_count, ' ');

        localtime_s(&time_tm, &time);
        // TODO: [C:/Development/Projects/My_Libraries/date-time.h:43] (style) Obsolete function 'asctime_s' called. It is recommended to use 'strftime' instead. [asctime_sCalled]
        asctime_s(&time_str_loc[0], sizeof(time_str_loc[0]) * time_str_loc.size(), &time_tm);
        time_str_loc.erase(ctime_char_count - 2, 2); // last two characters are: \n\0
        return time_str_loc;
    }

    // Formatted
    //std::string CurrentDateTimeFormatted(char* const format) {
    //    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    //    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    //    std::tm now_tm = *std::localtime(&now_c);

    //    // And then use strftime
    //    std::locale::global(std::locale("ja_JP.utf8"));
    //    std::time_t t = std::time(nullptr);
    //    char mbstr[100];
    //    if (std::strftime(mbstr, sizeof(mbstr), "%A %c", std::localtime(&t))) {
    //        std::cout << mbstr << '\n';
    //    }
    //}

} // !namespace util

#endif // !DATETIME_HPP



//constexpr int ctime_char_count{ 26 };
//void GetStrCurrentDateTimeC(char* const time_str) {
//    std::time_t time{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
//    char time_str[ctime_char_count];
//    ctime_s(&time_str[0], sizeof(time_str), &time);
//}
//
//void GetStrCurrentDateTimeUTC() {
//    std::time_t time{ std::time(nullptr) };
//    std::tm time_tm{};
//    char time_str_gm[ctime_char_count]{};
//
//    gmtime_s(&time_tm, &time);
//    asctime_s(&time_str_gm[0], sizeof(time_str_gm), &time_tm);
//}
//
//void GetStrCurrentDateTimeLocal() {
//    std::time_t time{ std::time(nullptr) };
//    std::tm time_tm{};
//    char time_str_loc[ctime_char_count]{};
//
//    localtime_s(&time_tm, &time);
//    asctime_s(&time_str_loc[0], sizeof(time_str_loc), &time_tm);
//}
//
//void GetStrCurrentDateTimeFormatted(char* const format) {
//    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
//    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
//    std::tm now_tm = *std::localtime(&now_c);
//
//    // And then use strftime
//    std::locale::global(std::locale("ja_JP.utf8"));
//    std::time_t t = std::time(nullptr);
//    char mbstr[100];
//    if (std::strftime(mbstr, sizeof(mbstr), "%A %c", std::localtime(&t))) {
//        std::cout << mbstr << '\n';
//    }
//}
