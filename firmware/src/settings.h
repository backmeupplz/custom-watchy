#pragma once

// Weather location (Vancouver, BC)
#define LAT "49.2827"
#define LON "-123.1207"
#define WEATHER_INTERVAL_MIN 30 // also re-syncs NTP time

watchySettings settings{
    .lat = LAT,
    .lon = LON,
    .ntpServer = "pool.ntp.org",
    .gmtOffset = -8 * 3600, // until the first weather fetch supplies the real offset (incl. DST)
    .vibrateOClock = false,
};
