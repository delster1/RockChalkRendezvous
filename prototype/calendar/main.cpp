#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "../definitions.hpp"
#include "../datetime_functions.hpp"


enum class RepeatType {
    NoRepeat,
    Daily,
    Weekly,
    Monthly,
    Yearly
};

struct repeat_attr {
    RepeatType type;
    u16 duration_days;
    // u16 interval_days; - For Daily repetition - later 

    // Private constructor to force using build functions for clarity
    private:
    repeat_attr(RepeatType repeatType, u16 duration_days)
        : type(repeatType),duration_days(duration_days) {}

    public:
    // Static build functions for different repeat types
    
    static repeat_attr build_repeat_attr(RepeatType repeatType,  u16 duration_days) {
        return {repeatType, duration_days};
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
        - getting busy times for a certain day - done
        - getting busy times for a certain time range - done
        - removing busy times - do we really need this - we can just find in the list and remove at index?
        - comparison function for two times to sort - done
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

    bool within( TimeAndDate& start1,  TimeAndDate& end1,  TimeAndDate& start2,  TimeAndDate& end2) {
        return start1 <= start2 && end2 <= end1;
    }

    void add_time(time_block to_add) {
        if (is_valid_time(to_add)) {
            busy_times.push_back(to_add);
        }
        else {
            std::cout << "Invalid time!\n";
        }
    }

    std::vector<time_block> get_busy_times_in_day(TimeAndDate& day) {
        // this function compares each obj in the list of busy times to this TimeAndDate object day. It checks both start and end to return times in a specific day only
        std::vector<time_block> out;
        for (auto& busy_time : busy_times) {
            if (busy_time.start.get_day_of_year() == day.get_day_of_year() && busy_time.start.get_year() == day.get_year() && busy_time.end.get_day_of_year() == day.get_day_of_year()){
	            // printf("%d %d %d:%d -> %d %d %d:%d\n",  busy_time.start.get_year(), busy_time.start.get_day_of_year(), busy_time.start.get_hour(), busy_time.start.get_minute(),  busy_time.end.get_year(), busy_time.end.get_day_of_year(), busy_time.end.get_hour(), busy_time.end.get_minute());
                
                out.push_back(busy_time);
            } 
        }
        return out;
    }

    std::vector<time_block> get_busy_times_in_range(TimeAndDate& start, TimeAndDate& end) {
        // this function compares each obj in the list of busy times to this TimeAndDate object day. It checks both start and end to return times in a specific day only
        std::vector<time_block> out;
        for (auto& busy_time : busy_times) {
            if (within(start, end, busy_time.start, busy_time.end)) {
	            printf("%d %d %d:%d -> %d %d %d:%d\n",  busy_time.start.get_year(), busy_time.start.get_day_of_year(), busy_time.start.get_hour(), busy_time.start.get_minute(),  busy_time.end.get_year(), busy_time.end.get_day_of_year(), busy_time.end.get_hour(), busy_time.end.get_minute());

                out.push_back(busy_time);
            }
        }
        return out;
    }

    void sort_busy_times() {
        std::sort(busy_times.begin(), busy_times.end(), []( time_block& a,  time_block& b) {
            if (a.start != b.start) {
                return a.start < b.start;
            } else {
                return a.end < b.end; // If starts are equal, compare ends
            }
        });
    }
};

int main () {
    // this main function is horribly messy - it just tests all of the functions above 
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
    // ignore the huge print stmts, just testing seeing that everything is in correctly
	// printf("%d %d %d:%d -> %d %d %d:%d\n",  my_cal.busy_times[1].start.get_year(), my_cal.busy_times[1].start.get_day_of_year(), my_cal.busy_times[1].start.get_hour(), my_cal.busy_times[1].start.get_minute(),  my_cal.busy_times[1].end.get_year(), my_cal.busy_times[1].end.get_day_of_year(), my_cal.busy_times[1].end.get_hour(), my_cal.busy_times[1].end.get_minute());

    TimeAndDate day_to_test = TimeAndDate::build(0,1,2024);
    std::vector<time_block> busy_times_on_day = my_cal.get_busy_times_in_day(day_to_test);

    TimeAndDate start_to_test = TimeAndDate::build(480,1,2024);
    TimeAndDate end_to_test = TimeAndDate::build(0,2,2024);

    std::vector<time_block> busy_times_in_range = my_cal.get_busy_times_in_range(start_to_test,end_to_test );

    my_cal.sort_busy_times();


    return 0;
}