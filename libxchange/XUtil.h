#pragma once
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Crypt32.lib")

class XUtil
{
public:
    static std::string FormatTimestamp(long long timestamp);
    static std::string FormatTimestampWithDate(long long timestamp);
    static std::string FormatTimestamp(long long timestamp, const std::string& format);

    static std::string FormatDuration(long long seconds);

    static std::string FloatToString(float value, int precision);

    static long long Now();
    static long long NowMs();
    static long long MinuteInS(int minutes);
    static long long HourInS(int seconds);
    static long long DaysInS(int days);

    static double RoundToDecimalPlaces(double value, int decimalPlaces);

    static constexpr float DegreesToRadians(float degrees);
};

