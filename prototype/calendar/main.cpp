#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

struct repeat_attr {
    // thinking we can handle the none case when repeat days are 0
    unsigned short days;
};
// bens time and date - changed to minutes b/c it might be simpler - happy to discuss though
struct time_and_date {
    unsigned int minutes;
    unsigned short days;
    unsigned short year;
};

struct time_block { 
    time_and_date start;
    time_and_date end;
    repeat_attr repeat_interval;
};

struct calendar { // contains busy times only! - happy to change what data structure we use
    std::vector<time_block> busy_times;
};

int main () {
    // testing - making simple block between 6 am - 8 am
    time_and_date start_time = {360, 0, 0}; 
    time_and_date end_time = {480, 0, 0};
    time_block busy_time = {start_time, end_time, 0};
    calendar my_cal = {};
    my_cal.busy_times.push_back(busy_time);
    return 0;
}