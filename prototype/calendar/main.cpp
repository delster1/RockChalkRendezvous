#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

#include "../definitions.hpp"
#include "../datetime_functions.hpp"

enum class RepeatType {
    NoRepeat,
    Daily,
    Weekly,
    Monthly,
    Yearly,
    RepeatBlock
};

struct repeat_attr {
    RepeatType type;
    TimeAndDate start;
    TimeAndDate end;
    TimeAndDate repeat_config; // Specific configuration depending on repeat type
    u16 interval_days; // For Daily repetition

    // Private constructor to force using build functions for clarity
    private:
    repeat_attr(RepeatType repeatType, const TimeAndDate& start, const TimeAndDate& end, const TimeAndDate& config, unsigned short interval)
        : type(repeatType), start(start), end(end), repeat_config(config), interval_days(interval) {}

    static repeat_attr buildRepeatedAttr(const TimeAndDate repeatDate) {
        return {RepeatType::RepeatBlock, repeatDate, repeatDate, repeatDate, 0}; // ik this code is cursed 
    }
    public:
    // Static build functions for different repeat types
    static repeat_attr buildNone(const TimeAndDate& start, const TimeAndDate& end) {
        return {RepeatType::NoRepeat, start, end, TimeAndDate(), 0};
    }

    static repeat_attr buildDaily(const TimeAndDate& start, const TimeAndDate& end, unsigned short interval_days) {
        return {RepeatType::Daily, start, end, TimeAndDate(), interval_days};
    }

    static repeat_attr buildWeekly(const TimeAndDate& start, const TimeAndDate& end, const TimeAndDate& weekDay) {
        return {RepeatType::Weekly, start, end, weekDay, 0};
    }

    static repeat_attr buildMonthly(const TimeAndDate& start, const TimeAndDate& end, const TimeAndDate& monthDay) {
        return {RepeatType::Monthly, start, end, monthDay, 0};
    }

    static repeat_attr buildYearly(const TimeAndDate& start, const TimeAndDate& end, const TimeAndDate& yearDay) {
        return {RepeatType::Yearly, start, end, yearDay, 0};
    }
};

struct time_block { 
    TimeAndDate start;
    TimeAndDate end;
    std::vector<RepeatType> repeat_intervals; // list of repeat attributes - determines multiple repeats - e.g: monthly and daily
};

struct calendar { // contains busy times only! - happy to discuss data structure for this tho
    std::vector<time_block> busy_times;
    /* will probably contain functions for:
        - getting busy times for a certain day
        - getting busy times for a certain time range
        - adding busy times ensuring they're valid -
        - removing busy times
        - comparison function for two times to sort
        - etc.
    */
    public:
    bool is_valid_time(time_block time) {
        if (time.start >= time.end ) {
            return false;
        }
        for ( auto& busy_time : busy_times) {
            // Check for any kind of overlap between the new time block and existing busy times
            if (overlaps(time.start, time.end, busy_time.start, busy_time.end)) {
                return false; // Found an overlap, time is not valid
            }
        }
        return true;
    }

    bool overlaps( TimeAndDate& start1,  TimeAndDate& end1,  TimeAndDate& start2,  TimeAndDate& end2) {
        // Check if one time block starts before the other ends and ends after the other starts (inverse of having no overlap)
        return start1 <= end2 && start2 <= end1;
    }

    void update_repeats(time_block time) {
        for (auto& repeat : time.repeat_intervals) {
            switch (repeat) {
                case RepeatType::NoRepeat:
                    break;
                case RepeatType::Daily:
                    // Add repeated time blocks for each day 
                    
                    break;
                case RepeatType::Weekly:
                    // Add repeated time blocks for each week
                    
                    break;
                case RepeatType::Monthly:
                    // Add repeated time blocks for each month
                   
                    break;
                case RepeatType::Yearly:
                    // Add repeated time blocks for each year
                    
                    break;
                case RepeatType::RepeatBlock:
                    // Add repeated time block with the same start and end time
                    
                    break;
            }
        }
    }

    void add_time(time_block to_add) {
        if (is_valid_time(to_add)) {

            busy_times.push_back(to_add);
            // UPDATE REPEATS NOT IMPLEMENTED YET
            update_repeats(to_add); // updates calendar with repeated time blocks depending on attributes in to_add obj
        }
        else {
            std::cout << "Invalid time!\n";
        }
    }

};

int main () {
    calendar my_cal = {};   
    // testing - making simple block between 6 am - 8 am
    TimeAndDate start_time = TimeAndDate::build(360, 0, 2024); 
    TimeAndDate end_time = TimeAndDate::build(480, 0, 2024);
    time_block busy_time = {start_time, end_time, {RepeatType::NoRepeat}};
    my_cal.add_time(busy_time);

    // testing making an invalid time
    TimeAndDate start_time_test = TimeAndDate::build(580, 0 , 2024);
    TimeAndDate end_time_test = TimeAndDate::build(580, 0 , 2024);
    time_block busy_time_test = {start_time_test, end_time_test, {RepeatType::NoRepeat}};
    my_cal.add_time(busy_time_test);
    
    // testing adding an overlapped time 
    TimeAndDate start_time_test_2 = TimeAndDate::build(380, 0, 2024); 
    TimeAndDate end_time_test_2 = TimeAndDate::build(470, 0, 2024);
    time_block busy_time_test_2 = {start_time_test_2, end_time_test_2, {RepeatType::NoRepeat}};
    my_cal.add_time(busy_time_test_2);
    return 0;
}