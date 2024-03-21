#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "../definitions.hpp"

struct repeat_attr {
    // thinking we can handle the none case when repeat days are 0
    unsigned short days;
};

struct time_block { 
    TimeAndDate start;
    TimeAndDate end;
    repeat_attr repeat_interval;
};

struct calendar { // contains busy times only! - happy to discuss data structure for this tho
    std::vector<time_block> busy_times;

    /* will probably contain functions for:
        - getting busy times for a certain day
        - getting busy times for a certain time range
        - adding busy times ensuring they're valid
        - removing busy times
        - etc.
    */
};

int main () {
    // testing - making simple block between 6 am - 8 am
    TimeAndDate start_time = TimeAndDate::build(360, 0, 2024); 
    TimeAndDate end_time = TimeAndDate::build(480, 0, 2024);
    time_block busy_time = {start_time, end_time, 0};
    calendar my_cal = {};   
    my_cal.busy_times.push_back(busy_time);
    return 0;
}