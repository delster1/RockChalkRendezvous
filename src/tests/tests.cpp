
#include "../shared/timeanddate.hpp"

int main() {
	let a = TimeAndDate::now();
	printf("%s\n", a.add_minutes(0).to_string().c_str());
	printf("%s\n", a.add_minutes(1).to_string().c_str());
	printf("%s\n", a.add_minutes(2).to_string().c_str());
	printf("%s\n", a.add_minutes(3).to_string().c_str());
	printf("%s\n", a.add_days(0).to_string().c_str());
	printf("%s\n", a.add_days(1).to_string().c_str());
	printf("%s\n", a.add_days(2).to_string().c_str());
	printf("%s\n", a.add_days(3).to_string().c_str());
	printf("%s\n", a.add_months(0).to_string().c_str());
	printf("%s\n", a.add_months(1).to_string().c_str());
	printf("%s\n", a.add_months(2).to_string().c_str());
	printf("%s\n", a.add_months(3).to_string().c_str());
	printf("%s\n", a.add_years(0).to_string().c_str());
	printf("%s\n", a.add_years(1).to_string().c_str());
	printf("%s\n", a.add_years(2).to_string().c_str());
	printf("%s\n", a.add_years(3).to_string().c_str());
}

