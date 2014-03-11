#ifndef HELPERS_H
#define HELPERS_H

#include <ctime>
#include <cstdio>
#include <cstdint> /* defines special types like uint8_t */
#include <string>

namespace helpers
{
    /**
     * @brief getFormattedTimeString Return a formatted time string
     * @param timeFormat
     * @return
     */
    inline std::string getFormattedTimeString(const std::string &timeFormat)
    {
        //get raw time
        time_t rawtime;
        //time struct
        struct tm *timeinfo;
        //buffer where we store the formatted time string
        char buffer[80];

        std::time (&rawtime);
        timeinfo = std::localtime(&rawtime);

        std::strftime(buffer, 80, timeFormat.c_str(), timeinfo);
        return (std::string(buffer));
    }


    inline std::string getFormattedTimeString(std::time_t t, const std::string &timeFormat)
    {
        //time struct
        struct tm *timeinfo;
        //buffer where we store the formatted time string
        char buffer[80];

        timeinfo = std::localtime(&t);

        std::strftime(buffer, 80, timeFormat.c_str(), timeinfo);
        return (std::string(buffer));
    }



//    inline uint8_t flipBits( uint8_t n )
//    {
////        const uint8_t LOOKUPTABLE_UINT8[16] = {
////           0x0, 0x8, 0x4, 0xC,
////           0x2, 0xA, 0x6, 0xE,
////           0x1, 0x9, 0x5, 0xD,
////           0x3, 0xB, 0x7, 0xF };
//       //http://stackoverflow.com/a/2603254
//       //This should be just as fast and it is easier to understand.
////       return (helpers::global_constants::LOOKUPTABLE_UINT8.at(n&0x0F) << 4) |
////               helpers::global_constants::LOOKUPTABLE_UINT8.at(n>>4);
//    }
}

namespace global_constants
{
    /*
     * Global identifier and constants that are used application wide
     */
    const static int SENSOR_GROVEDUST = 1;
    const static int SENSOR_SHARPDUST = 2;
    const static int SENSOR_DSMDUST = 3;
    const static int SENSOR_MICSNO = 4;
    const static int SENSOR_DSBTEMP = 5;

    const static int VENTILATOR = 100;
}

#endif // HELPERS_H
