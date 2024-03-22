#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>

#include "../definitions.hpp"
#include "../datetime_functions.hpp"

u32 days_difference( TimeAndDate& date1,  TimeAndDate& date2) {
    // Calculate the total number of days for each date from year 0
    u32 totalDays1 = 0;
    for (u32 year = 0; year < date1.get_year(); ++year) {
        totalDays1 += find_days_in_year(year);
    }
    totalDays1 += date1.get_day_of_year();

    u32 totalDays2 = 0;
    for (u32 year = 0; year < date2.get_year(); ++year) {
        totalDays2 += find_days_in_year(year);
    }
    totalDays2 += date2.get_day_of_year();

    // Return the absolute difference between the two totals
    return totalDays2 - totalDays1;
}

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
    u16 duration;
    // u16 interval_days; - For Daily repetition - later 

    // Private constructor to force using build functions for clarity
    private:
    repeat_attr(RepeatType repeatType, u16 duration)
        : type(repeatType),duration(duration) {}

    public:
    // Static build functions for different repeat types
    
    static repeat_attr build_repeat_attr(RepeatType repeatType,  u16 duration) {
        return {repeatType, duration};
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
        - removing busy times - do we really need this?
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
    time_block nonRepeatingBlock = {startTime, endTime, repeat_attr::build_repeat_attr(RepeatType::NoRepeat, 0)};
    my_cal.add_time(nonRepeatingBlock);
	printf("%d %d %d:%d -> %d %d %d:%d\n",  my_cal.busy_times[0].start.get_year(), my_cal.busy_times[0].start.get_day_of_year(), my_cal.busy_times[0].start.get_hour(), my_cal.busy_times[0].start.get_minute(),  my_cal.busy_times[0].end.get_year(), my_cal.busy_times[0].end.get_day_of_year(), my_cal.busy_times[0].end.get_hour(), my_cal.busy_times[0].end.get_minute());

    // Adding a daily repeating time block
    time_block dailyRepeatingBlock = {startTime, endTime, repeat_attr::build_repeat_attr(RepeatType::Daily, 100)};
    my_cal.add_time(dailyRepeatingBlock);

    // testing making an invalid time
    TimeAndDate start_time_test = TimeAndDate::build(580, 0 , 2024);
    TimeAndDate end_time_test = TimeAndDate::build(580, 0 , 2024);
    time_block busy_time_test = {start_time_test, end_time_test, repeat_attr::build_repeat_attr(RepeatType::NoRepeat, 0)};
    my_cal.add_time(busy_time_test);
    
    // testing adding an overlapped time 
    TimeAndDate start_time_test_2 = TimeAndDate::build(500, 1, 2024); 
    TimeAndDate end_time_test_2 = TimeAndDate::build(560, 1, 2024);
    time_block busy_time_test_2 = {start_time_test_2, end_time_test_2, repeat_attr::build_repeat_attr( RepeatType::NoRepeat,0)};

    my_cal.add_time(busy_time_test_2);
    // ignore this huge print stmts, just testing seeing that everything is in correctly
	printf("%d %d %d:%d -> %d %d %d:%d\n",  my_cal.busy_times[1].start.get_year(), my_cal.busy_times[1].start.get_day_of_year(), my_cal.busy_times[1].start.get_hour(), my_cal.busy_times[1].start.get_minute(),  my_cal.busy_times[1].end.get_year(), my_cal.busy_times[1].end.get_day_of_year(), my_cal.busy_times[1].end.get_hour(), my_cal.busy_times[1].end.get_minute());

    return 0;
}