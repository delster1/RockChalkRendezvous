#ifndef RCR_CALENDAR_DEFINITIONS
#define RCR_CALENDAR_DEFINITIONS

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <tuple>

#include "core_utils.hpp"
#include "timeanddate.hpp"

enum RepeatType {
    NoRepeat = 'N',
    Daily    = 'D',
    Weekly   = 'W',
    Monthly  = 'M',
    Yearly   = 'Y',
};

// Core functionality of time with a start/end time
struct TimeBlock { // MARK: TimeBlock
    std::string name;
    TimeAndDate start;
    TimeAndDate end;
    RepeatType repeat_period;
    u32 repeat_count;
    
    inline TimeBlock() : name(""), start(TimeAndDate()), end(TimeAndDate()), repeat_period(NoRepeat), repeat_count(0) {}
    inline TimeBlock(std::string name, TimeAndDate start, TimeAndDate end, RepeatType repeat_period, u32 repeat_count) : name(name), start(start), end(end), repeat_period(repeat_period), repeat_count(repeat_count) {
        this->validate();
    }
    
    // checks that the start time is before the end time
    inline bool is_valid() const {
        return this->start <= this->end;
    }
    
    // swaps the start and end times if they are in the wrong order
    void validate() {
        if (!this->is_valid()) {
            TimeAndDate temp = this->start;
            this->start = this->end;
            this->end = temp;
        }
    }
    
    inline i32 duration() const {
        return this->end.minutes_since(this->start);
    }
    
    std::tuple<TimeAndDate, TimeAndDate> get_occurrence(const i32 repeat_number) const {
        TimeAndDate start;
        switch (this->repeat_period) {
            case Daily: start = this->start.add_days(repeat_number); break;
            case Weekly: start = this->start.add_days(repeat_number * 7); break;
            case Monthly: start = this->start.add_months(repeat_number); break;
            case Yearly: start = this->start.add_years(repeat_number); break;
            default: start = this->start;
        }
        return std::make_tuple(start, start.add_minutes(this->duration()));
    }
    
    
    // encoding and decoding

    static inline std::string encode_static(const TimeBlock& block) { return block.encode(); }
    std::string encode() const {
        std::ostringstream oss;
        oss << quote_string(this->name) << " " << this->start.encode() << " " << this->end.encode() << " " << static_cast<char>(this->repeat_period) << " " << this->repeat_count;
        return oss.str();
    }
    
    static inline Status decode_static(std::istream& s, TimeBlock& block) { return block.decode(s); }
    Status decode(std::istream& s) {
        propagate(read_quoted_string(s, this->name));
        propagate(this->start.decode(s));
        propagate(this->end.decode(s));
        
        char c;
        s >> c;
        if (s.fail()) return Failure;
        this->repeat_period = static_cast<RepeatType>(c);
        switch (this->repeat_period) {
            case NoRepeat:
            case Daily:
            case Weekly:
            case Monthly:
            case Yearly:
                break;
            default:
                return Failure;
        }
        
        s >> this->repeat_count;
        if (s.fail()) return Failure;
        
        return Success;
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "\n" << this->name << ":\nStart: " << this->start.to_string() << "\nEnd: " << this->end.to_string() << "\n";
        
        switch (this->repeat_period) {
            case NoRepeat: oss << "Does not repeat\n"; break;
            case Daily: oss << "Every day for " << this->repeat_count << " days\n"; break;
            case Weekly: oss << "Every week for " << this->repeat_count << " weeks\n"; break;
            case Monthly: oss << "Every month for " << this->repeat_count << " months\n"; break;
            case Yearly: oss << "Every year for " << this->repeat_count << " years\n"; break;
            default: oss << "Invalid repeat property\n";
        }
        
        return oss.str();
    }
};

// Each calendar consists of a list of TimeBlocks
struct Calendar { // MARK: Calendar
    std::vector<TimeBlock> busy_times;
    
    inline Calendar() : busy_times(std::vector<TimeBlock>()) {}
    
    // This function returns a vector of TimeBlocks that are busy within a given day.
    // It does this by creating a range from the start of the given day to the start of the next day,
    // and then calling the get_busy_times_in_range function with this range.
    // inline std::vector<const TimeBlock*> get_busy_times_in_day(const TimeAndDate& day) const {
    //     return this->get_busy_times_in_range(
    //         day.replace_time(0),
    //         day.replace_time(0).add_days(1)
    //     );
    // }

    // This function returns a vector of TimeBlocks that are busy within a given interval.
    // std::vector<const TimeBlock*> get_busy_times_in_range(const TimeAndDate& start, const TimeAndDate& end) const {
    //     // todo: make work with repeating blocks
    //     std::vector<const TimeBlock*> out;
    //     for (const TimeBlock& busy_time : this->busy_times) {
    //         if (busy_time.occurs_in_range(start, end)) {
    //             out.push_back(&busy_time);
    //         }
    //     }
    //     return out;
    // }

    // // Takes a time point and checks if it is included in the block's busy times
    // bool is_time_busy(const TimeAndDate& timePoint) const {
    //     for (const TimeBlock& block : this->busy_times) {
    //         if (block.occurs_during_time(timePoint)) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // sorts busy time by ascending start time
    void sort_busy_times() {
        std::sort(this->busy_times.begin(), this->busy_times.end(), [](const TimeBlock& a, const TimeBlock& b) {
            if (a.start != b.start) {
                return a.start < b.start;
            } else {
                return a.end < b.end;
            }
        });
    }
    
    Status add_time(const TimeBlock& block) {
        if (block.start < block.end) {
            this->busy_times.push_back(block);
            return Success;
        }
        return Failure;
    }
    
    static inline std::string encode_static(const Calendar& calendar) { return calendar.encode(); }
    std::string encode() const {
        return encode_vector<TimeBlock>(this->busy_times, TimeBlock::encode_static, true);
    }
    
    static inline Status decode_static(std::istream& s, Calendar& calendar) { return calendar.decode(s); }
    Status decode(std::istream& s) {
        return decode_vector<TimeBlock>(s, this->busy_times, TimeBlock::decode_static);
    }
    
    
    
};



#endif
