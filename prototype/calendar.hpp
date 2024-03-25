<<<<<<< HEAD
// TODO: 
// - refactor decode - move to timeblock and refactor calendar decode as folloiws
// - 
=======
#ifndef RCR_CALENDAR_DEFINITIONS
#define RCR_CALENDAR_DEFINITIONS
>>>>>>> c5ad4af6608e658e08b4951e3300a2e7075886e1

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "definitions.hpp"
#include "datetime_functions.hpp"
#include "common_types.hpp"

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
    inline repeat_attr(RepeatType type, u16 duration_days) : type(type), duration_days(duration_days) {}

    // u16 interval_days; - For Daily repetition - later 

    // Private constructor to force using build functions for clarity
    private:

    public:
    inline repeat_attr() : type(), duration_days() {}

    // Static build functions for different repeat types
    std::string repeaAttrToString(RepeatType type) {
        switch (type) {
            case RepeatType::NoRepeat: return "NoRepeat";
            case RepeatType::Daily: return "Daily";
            case RepeatType::Weekly: return "Weekly";
            case RepeatType::Monthly: return "Monthly";
            case RepeatType::Yearly: return "Yearly";
            default: return "Unknown";
        }
    }
    
    static Option<RepeatType> stringToRepeatType(const std::string& str) {
        if (str == "NoRepeat") return Option<RepeatType>::some(RepeatType::NoRepeat);
        if (str == "Daily") return Option<RepeatType>::some(RepeatType::Daily);
        if (str == "Weekly") return Option<RepeatType>::some(RepeatType::Weekly);
        if (str == "Monthly") return Option<RepeatType>::some(RepeatType::Monthly);
        if (str == "Yearly") return Option<RepeatType>::some(RepeatType::Yearly);

        return Option<RepeatType>::none(); // Default case
    }

    std::string encode() {
        std::ostringstream oss;
        oss << repeaAttrToString(type) << " " << duration_days;
        return oss.str();
    }

    static Option<repeat_attr> decode(std::istringstream& s) {
        
        std::string part;
        std::getline(s, part, ' '); // Get RepeatType part
        Option<RepeatType> type = stringToRepeatType(part);
        std::getline(s, part, ' '); // Get duration_days part
        u16 duration_days;
        if (s >> duration_days && type.is_some()) {
            return Option<repeat_attr>::some(repeat_attr::build_repeat_attr(type.unwrap(), duration_days));
        }
        else {
            return Option<repeat_attr>::none();
        }
    }

    static repeat_attr build_repeat_attr(RepeatType repeatType,  u16 duration_days) {
        return {repeatType, duration_days};
    }
};

struct time_block { 
    TimeAndDate start;
    TimeAndDate end;
    repeat_attr repeat_interval; // list of repeat attributes - determines multiple repeats - e.g: monthly and daily
    
    inline time_block(TimeAndDate start, TimeAndDate end, repeat_attr repeat_interval) : start(start), end(end), repeat_interval(repeat_interval) {}
    public:
    inline time_block() : start(), end(), repeat_interval() {}
    std::string encode() {
        std::ostringstream oss;
        oss << repeat_interval.encode() << " "<< start.encode() << " " << end.encode();
        return oss.str();
    }

    static Option<time_block> decode(std::istringstream& s) {
        std::string repeatIntervalString;
        std::string timeBlockStartString;
        std::string timeBlockEndString;

        std::getline(s, repeatIntervalString, ' ');
        std::getline(s, timeBlockStartString, ' ');
        std::getline(s, timeBlockEndString, ' ');

        std::istringstream repeatIntervalStream(repeatIntervalString);
        std::istringstream timeBlockStartStream(timeBlockStartString);
        std::istringstream timeBlockEndStream(timeBlockEndString);

        Option<repeat_attr> repeat = repeat_attr::decode(repeatIntervalStream);
        Option<TimeAndDate> start = TimeAndDate::decode(timeBlockStartStream);
        Option<TimeAndDate> end = TimeAndDate::decode(timeBlockEndStream);

        if (repeat.is_some() && start.is_some() && end.is_some()) {
            return Option<time_block>::some({start.unwrap(), end.unwrap(), repeat.unwrap()});
        }
        else {
            return Option<time_block>::none();
        }          
    }



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
	            // printf("%d %d %d:%d -> %d %d %d:%d\n",  busy_time.start.get_year(), busy_time.start.get_day_of_year(), busy_time.start.get_hour(), busy_time.start.get_minute(),  busy_time.end.get_year(), busy_time.end.get_day_of_year(), busy_time.end.get_hour(), busy_time.end.get_minute());

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
<<<<<<< HEAD

    calendar decode(std::istringstream& s) {
        calendar decodedCal;
        

        return decodedCal;
    }

    
    std::string encodeCalendar() {
        std::ostringstream oss;
        oss << busy_times.size() << "\n";
        for(auto& busy_time : busy_times) {

            oss << busy_time.encode() << "\n";
        } 
        return oss.str();
    }
};

=======
};



#endif
>>>>>>> c5ad4af6608e658e08b4951e3300a2e7075886e1
