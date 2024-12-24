#include "XUtil.h"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <iostream>
#include <chrono>


std::string XUtil::FormatTimestamp(long long timestamp) 
{
    std::time_t time = static_cast<std::time_t>(timestamp /*/ 1000*/); // Umrechnung von Millisekunden in Sekunden
    std::tm tm;
    if (localtime_s(&tm, &time) != 0) {
        std::cerr << "Fehler beim Umwandeln des Zeitstempels in Zeitstruktur." << std::endl;
        return "";
    }

    std::stringstream ss;
    //ss << std::put_time(&tm, "%d.%m %H:%M:%S"); // Formatieren als YYYY-MM-DD
    ss << std::put_time(&tm, "%H:%M"); // Formatieren als YYYY-MM-DD
    return ss.str();
}

std::string XUtil::FormatTimestampWithDate(long long timestamp)
{
    std::time_t time = static_cast<std::time_t>(timestamp /*/ 1000*/); // Umrechnung von Millisekunden in Sekunden
    std::tm tm;
    if (localtime_s(&tm, &time) != 0) {
        std::cerr << "Fehler beim Umwandeln des Zeitstempels in Zeitstruktur." << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << std::put_time(&tm, "%a %d.%m.%y %H:%M:%S"); // Formatieren als YYYY-MM-DD
    //ss << std::put_time(&tm, "%H:%M"); // Formatieren als YYYY-MM-DD
    return ss.str();
}

std::string XUtil::FormatTimestamp(long long timestamp, const std::string& format)
{
    std::time_t time = static_cast<std::time_t>(timestamp /*/ 1000*/); // Umrechnung von Millisekunden in Sekunden
    std::tm tm;
    if (localtime_s(&tm, &time) != 0) {
        std::cerr << "Fehler beim Umwandeln des Zeitstempels in Zeitstruktur." << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str()); // Formatieren als YYYY-MM-DD
    //ss << std::put_time(&tm, "%H:%M"); // Formatieren als YYYY-MM-DD
    return ss.str();
}

long long XUtil::MinuteInS(int minutes)
{
    long long milli = 60;
    return minutes * milli;
}

long long XUtil::HourInS(int hours)
{
    long long milli = 60 * 60;
    return hours * milli;
}

long long XUtil::DaysInS(int days)
{
    long long millisecondsInADay = 24LL * 60 * 60;
    return days * millisecondsInADay;
}

long long XUtil::Now()
{
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    long long unixTimestamp = static_cast<long long>(t);
    return unixTimestamp;
}

long long XUtil::NowMs()
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return now_ms.count();
}

std::string XUtil::FormatDuration(long long seconds) {
    std::stringstream ss;

    // Calculate weeks, days, hours, and minutes
    int weeks = seconds / (7 * 24 * 60 * 60);
    seconds %= 7 * 24 * 60 * 60;
    int days = seconds / (24 * 60 * 60);
    seconds %= 24 * 60 * 60;
    int hours = seconds / (60 * 60);
    seconds %= 60 * 60;
    int minutes = seconds / 60;
    seconds %= 60;

    // Format the output based on the non-zero values
    if (weeks > 0) {
        ss << weeks << "w ";
    }
    if (days > 0) {
        ss << days << "d ";
    }
    if (hours > 0) {
        ss << hours << "h ";
    }
    if (minutes > 0) {
        ss << minutes << "m ";
    }

    // Remove the trailing space if necessary
    if (!ss.str().empty()) {
        ss.seekp(-1, std::ios_base::end);
        ss.put('\0');
    }

    return ss.str();
}

std::string XUtil::FloatToString(float value, int precision) 
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

double XUtil::RoundToDecimalPlaces(double value, int decimalPlaces)
{
    double factor = std::pow(10.0, decimalPlaces);
    return std::round(value * factor) / factor;
}


constexpr float XUtil::DegreesToRadians(float degrees) {
    return degrees * (3.14159265358979323846f / 180.0f);
}
