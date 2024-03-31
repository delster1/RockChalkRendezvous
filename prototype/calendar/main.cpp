

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <bitset>
#include <algorithm>
#include "../timeanddate.hpp"
#include "../calendar.hpp"

int main () {
    // this main function is horribly messy - it just tests all of the functions above 
    calendar my_cal = {};   
    // testing - making simple block between 6 am - 8 am
    TimeAndDate startTime = TimeAndDate::build(360, 1, 2024); // Assuming 6:00 AM on the first day of the year 2024
    TimeAndDate endTime = TimeAndDate::build(480, 1, 2024); // Assuming 8:00 AM on the same day

    // Adding a non-repeating time block
    time_block nonRepeatingBlock = {startTime, endTime, repeat_attr::build_repeat_attr(RepeatType::NoRepeat, 0)};
    my_cal.add_time(nonRepeatingBlock);

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

    std::string encoded_cal = my_cal.encode();
    std::cout << encoded_cal; 
    std::istringstream encodedCalStream(encoded_cal);
    Status calendar_status = my_cal.decode(encodedCalStream, my_cal);

    std::string test_cal = my_cal.encode();
    std::cout << "CALENDAR: " << test_cal << "\n";
    return 0;
}