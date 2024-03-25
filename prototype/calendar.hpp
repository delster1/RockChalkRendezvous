#ifndef RCR_CALENDAR_DEFINITIONS
#define RCR_CALENDAR_DEFINITIONS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "definitions.hpp"
#include "datetime_functions.hpp"


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



#endif