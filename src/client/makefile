CC := g++
TARGET := ui
FLAGS := -std=c++17 -I../../lib/cpp-httplib -I"../../lib/ncurses/include" -I"../../lib/ncurses/include/ncursesw"
LINUX_LDLIBS := -lncursesw -pthread
WIN_LDLIBS := -lWs2_32 -lssp -L"../../lib/ncurses/lib" -lncurses -DNCURSES_STATIC
SOURCES := login.cpp

ifeq ($(shell echo "check_quotes"),"check_quotes")
    WINDOWS := yes
else
    WINDOWS := no
endif

build: $(SOURCES)
ifeq ($(WINDOWS), yes)
	$(CC) $(FLAGS) $(SOURCES) -o $(TARGET).exe $(WIN_LDLIBS)
else
	$(CC) $(FLAGS) $(SOURCES) -o $(TARGET) $(LINUX_LDLIBS)
endif

clean:
	rm -f $(TARGET)
