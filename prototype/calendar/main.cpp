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
    // u16 interval_days; - For Daily repetition - later 

    // Private constructor to force using build functions for clarity
    private:
    repeat_attr(RepeatType repeatType, const TimeAndDate& start, const TimeAndDate& end)
        : type(repeatType), start(start), end(end) {}

    public:
    // Static build functions for different repeat types
    static repeat_attr build_repeat_attr(RepeatType repeatType, const TimeAndDate& start, const TimeAndDate& end) {
        switch (repeatType) {
            case RepeatType::NoRepeat:
                return {RepeatType::NoRepeat, start, end};
            case RepeatType::Daily:
                return {RepeatType::Daily, start, end};
            case RepeatType::Weekly:
                return {RepeatType::Weekly, start, end};
            case RepeatType::Monthly:
                return {RepeatType::Monthly, start, end};
            case RepeatType::Yearly:
                return {RepeatType::Yearly, start, end};
            case RepeatType::RepeatBlock:
                return {RepeatType::RepeatBlock, start, end};

        }
    }
};

struct time_block { 
    TimeAndDate start;
    TimeAndDate end;
    repeat_attr repeat_interval; // list of repeat attributes - determines multiple repeats - e.g: monthly and daily

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


    void add_time(time_block to_add) {
        if (is_valid_time(to_add)) {
            busy_times.push_back(to_add);
        }
        else {
            std::cout << "Invalid time!\n";
        }
    }

};

int main () {
    calendar my_cal = {};   
    // testing - making simple block between 6 am - 8 am
    TimeAndDate startTime = TimeAndDate::build(360, 1, 2024); // Assuming 6:00 AM on the first day of the year 2024
    TimeAndDate endTime = TimeAndDate::build(480, 1, 2024); // Assuming 8:00 AM on the same day

    // Adding a non-repeating time block
    time_block nonRepeatingBlock = {startTime, endTime, repeat_attr::build_repeat_attr(RepeatType::NoRepeat, startTime, endTime)};
    my_cal.add_time(nonRepeatingBlock);

    // Adding a daily repeating time block
    time_block dailyRepeatingBlock = {startTime, endTime, repeat_attr::build_repeat_attr(RepeatType::Daily, startTime, endTime)};
    my_cal.add_time(dailyRepeatingBlock);

    // testing making an invalid time
    TimeAndDate start_time_test = TimeAndDate::build(580, 0 , 2024);
    TimeAndDate end_time_test = TimeAndDate::build(580, 0 , 2024);
    time_block busy_time_test = {start_time_test, end_time_test, repeat_attr::build_repeat_attr(RepeatType::NoRepeat, start_time_test, end_time_test)};
    my_cal.add_time(busy_time_test);
    
    // testing adding an overlapped time 
    TimeAndDate start_time_test_2 = TimeAndDate::build(380, 1, 2024); 
    TimeAndDate end_time_test_2 = TimeAndDate::build(470, 1, 2024);
    time_block busy_time_test_2 = {start_time_test_2, end_time_test_2, repeat_attr::build_repeat_attr( RepeatType::NoRepeat,start_time_test_2, end_time_test_2)};

    my_cal.add_time(busy_time_test_2);
    return 0;
}