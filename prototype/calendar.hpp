#ifndef RCR_CALENDAR_DEFINITIONS
#define RCR_CALENDAR_DEFINITIONS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream> // Necessary for string streams. <sstream> is not part of <string>
#include <bitset>
#include <algorithm>
#include "timeanddate.hpp" // Assuming this is a custom header for handling time and date

enum class RepeatType : char { // Assigning underlying type as char for ease of encoding/decoding
    NoRepeat = 0,
    Daily = 1, 
    Weekly = 2,
    Monthly = 3,
    Yearly = 4
};

struct RepeatAttribute { // Struct names should be CamelCase
    RepeatType type;
    uint16_t duration_days; // Use standard uint16_t instead of "u16" assuming it was a typedef
    
    // Constructor with default values
    RepeatAttribute(RepeatType type = RepeatType::NoRepeat, uint16_t duration_days = 0) : type(type), duration_days(duration_days) {}

    static std::string to_string(RepeatType type) { // Renamed method
        switch (type) {
            case RepeatType::NoRepeat: return "NoRepeat";
            case RepeatType::Daily: return "Daily";
            case RepeatType::Weekly: return "Weekly";
            case RepeatType::Monthly: return "Monthly";
            case RepeatType::Yearly: return "Yearly";
            default: return "Unknown";
        }
    }
    
    // Encoding method simplified to use type's numerical value
    std::string encode() const {
        std::ostringstream oss;
        oss << static_cast<int>(type) << " " << duration_days;
        return oss.str();
    }

    static RepeatAttribute decode(std::istream& s) {
        RepeatType type;
        uint16_t duration_days;
        int typeInt;
        s >> typeInt >> duration_days; // Direct parsing without getline
        if (!s.fail() && typeInt >= static_cast<int>(RepeatType::NoRepeat) && typeInt <= static_cast<int>(RepeatType::Yearly)) {
            type = static_cast<RepeatType>(typeInt);
            return RepeatAttribute(type, duration_days);
        } else {
            // Handle decoding failure, perhaps throw an exception or return a default value
            return RepeatAttribute();
        }
    }
};

struct TimeBlock { // Renamed to CamelCase
    TimeAndDate start;
    TimeAndDate end;
    RepeatAttribute repeat_interval;
    
    TimeBlock(TimeAndDate start, TimeAndDate end, RepeatAttribute repeat_interval) : start(start), end(end), repeat_interval(repeat_interval) {}
    
    TimeBlock() : start(), end(), repeat_interval() {} // Default constructor
    
    std::string encode() const {
        std::ostringstream oss;
        oss << repeat_interval.encode() << " " << start.encode() << ":" << end.encode();
        return oss.str();
    }

    static TimeBlock decode(std::istream& s) {
        RepeatAttribute repeat = RepeatAttribute::decode(s);
        TimeAndDate start; 
        TimeAndDate::decode(s, start); // Assuming TimeAndDate has a similar decode method
        TimeAndDate end; 
         TimeAndDate::decode(s, end);
        return TimeBlock(start, end, repeat);
    }
};

struct Calendar { // Renamed to CamelCase
    std::vector<TimeBlock> busy_times;

    Calendar(std::vector<TimeBlock> busy_times = {}) : busy_times(busy_times) {} // Constructor with default argument

    bool is_valid_time(const TimeBlock& time) const {
        if (time.start >= time.end) {
            return false;
        }
        for (const auto& busy_time : busy_times) {
            if (overlaps(time.start, time.end, busy_time.start, busy_time.end)) {
                return false;
            }
        }
        return true;
    }
    
    static bool overlaps(const TimeAndDate& start1, const TimeAndDate& end1, const TimeAndDate& start2, const TimeAndDate& end2) {
        return start1 <= end2 && start2 <= end1;
    }

    void add_time(const TimeBlock& to_add) {
        if (is_valid_time(to_add)) {
            busy_times.push_back(to_add);
        } else {
            std::cout << "Invalid time!\n";
        }
    }

    std::string encode() const {
        std::ostringstream oss;
        oss << busy_times.size() << "\n";
        for(const auto& busy_time : busy_times) {
            oss << busy_time.encode() << "\n";
        }
        return oss.str();
    }

    // Example of a method to decode Calendar objects
    static Status decode(std::istream& s, Calendar& cal) {
        size_t numBlocks;
        s >> numBlocks; // Assumes the first line is the number of time blocks
        for (size_t i = 0; i < numBlocks; ++i) {
            TimeBlock block = TimeBlock::decode(s);
            cal.add_time(block);
        }
        return Success;
    }
};

#endif // RCR_CALENDAR_DEFINITIONS
