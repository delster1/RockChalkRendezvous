CC := clang++
TARGET := main
FLAGS := -Wall -Wextra -Werror -std=c++17
LDLIBS := -lncurses
SOURCES := main.cpp

build: $(SOURCES)
	$(CC) $(FLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)