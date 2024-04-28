CC := g++
TARGET := ui.exe
FLAGS := -std=c++17 -I../../lib/cpp-httplib -I"../../lib/ncurses/include" -I"../../lib/ncurses/include/ncursesw"
LDLIBS := -L"../../lib/ncurses/lib" -lncurses -DNCURSES_STATIC
SOURCES := main.cpp

build: $(SOURCES)
	$(CC) $(FLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)