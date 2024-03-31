#ifndef RCR_CALENDAR_DEFINITIONS
#define RCR_CALENDAR_DEFINITIONS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "timeanddate.hpp"

enum class RepeatType {
    Invalid,
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
    inline repeat_attr() : type(), duration_days() {} // none constructor

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
    
    static RepeatType stringToRepeatType(const std::string& str) {
        if (str == "NoRepeat") return (RepeatType::NoRepeat);
        if (str == "Daily") return (RepeatType::Daily);
        if (str == "Weekly") return (RepeatType::Weekly);
        if (str == "Monthly") return (RepeatType::Monthly);
        if (str == "Yearly") return (RepeatType::Yearly);

        return RepeatType::Invalid; // Default case
    }

    std::string encode() {
        std::ostringstream oss;
        oss << repeaAttrToString(type) << " " << duration_days;
        return oss.str();
    }

    static Status decode(std::istringstream& s, repeat_attr repeat) {
        
        std::string part;
        std::getline(s, part, ' '); // Get RepeatType part
        RepeatType type = stringToRepeatType(part);
        std::getline(s, part, ' '); // Get duration_days part
        u16 duration_days;
        try {
            duration_days = static_cast<uint16_t>(std::stoi(part));
        } catch (const std::invalid_argument& e) {
    // Handle the case where the conversion cannot be performed
    // because the input is not a valid number
        } catch (const std::out_of_range& e) {
    // Handle the case where the number is out of the int range
        }
        if (duration_days >= 0 && type != RepeatType::Invalid) {
            repeat_attr repeat = repeat_attr::build_repeat_attr(type, duration_days);
            return Success;
        }
        else {

            return Failure;
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
        oss << repeat_interval.encode() << " "<< start.encode() << ":" << end.encode();
        return oss.str();
    }

    static Status decode(std::istringstream& s, time_block block) {
        std::string repeatTypeString;
        std::string repeatIntervalString;
        std::string timeBlockStartString;
        std::string timeBlockEndString;

        std::getline(s, repeatTypeString, ' ');
        std::getline(s, repeatIntervalString, ' ');
        std::getline(s, timeBlockStartString, ':');

        std::getline(s, timeBlockEndString, '\n');
        std::istringstream repeatStream(repeatTypeString + ' '+ repeatIntervalString); // ik this code is cursed - j threw the delim in lol
        std::istringstream timeBlockStartStream(timeBlockStartString);
        std::istringstream timeBlockEndStream(timeBlockEndString);
        repeat_attr repeat;
        Status repeat_status = repeat_attr::decode(repeatStream, repeat);
        TimeAndDate start;
        Status start_status = TimeAndDate::decode(timeBlockStartStream, start);
        TimeAndDate end;
        Status end_status = TimeAndDate::decode(timeBlockEndStream, end);

        u16 duration_days;
        try {
            duration_days = static_cast<uint16_t>(std::stoi(repeatIntervalString));
        } catch (const std::invalid_argument& e) {
    // Handle the case where the conversion cannot be performed
    // because the input is not a valid number
        } catch (const std::out_of_range& e) {
    // Handle the case where the number is out of the int range
        }
        if (repeat_status != Failure && start_status != Failure && end_status != Failure) {
            block = time_block(start, end, repeat);
            return Success;
        }
        else {
            return Failure;
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
   inline calendar(std::vector<time_block>busy_times) : busy_times(busy_times) {} // default constructor
   inline calendar() : busy_times() {} // none concstructor
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

    Status  decode(std::istringstream& s,calendar decoded_cal) {
        

        // read first line of string stream - number to loop
        std::string string_repetitions;
        int num_reps;
        std::getline(s, string_repetitions, '\n');
        num_reps = stoi(string_repetitions);
        for (int i = 0; i < num_reps; i++ ){
            std::string line;
            
            std::getline(s, line, '\n');

            std::istringstream time_block_istream(line);
            std::ostringstream oss;
            time_block block;
            Status block_decode_status = time_block::decode(time_block_istream, block);
            oss << time_block_istream.rdbuf();
            std::string time_block_string = oss.str();
            
            if(block_decode_status == Success){ 

                decoded_cal.add_time(block);
            }
            
            // How do we want to handle invalid time blocks here?
            
        }

        return Failure;
    }
    
    
    std::string encode() {
        std::ostringstream oss;
        oss << busy_times.size() << "\n";
        for(auto& busy_time : busy_times) {
            
            oss << busy_time.encode() << "\n";
        }
        return oss.str();
    }
};



#endif