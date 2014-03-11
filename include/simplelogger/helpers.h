/*  This is a very simple logger for c++
    Copyright (C) 2013, 2014 Christian Rapp

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
}


#endif // HELPERS_H
