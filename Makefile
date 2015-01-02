#-------------------------------------------------------------------------------

COMPILER=gcc
COMPILER_GLOBAL_FLAGS=

BUILD_PATH=build
OBJECT_PATH=$(BUILD_PATH)
SOURCE_PATH=lib
INCLUDE_PATH=include

EXECUTABLE=vke

PREFIX=$(DEST_DIR)/usr/local
BIN_PATH=$(PREFIX)/bin

#-------------------------------------------------------------------------------

all: $(EXECUTABLE)

debug: COMPILER_GLOBAL_FLAGS += -g -Wall -Wshadow -Werror
debug: $(EXECUTABLE)

clean:
	rm -f $(BUILD_PATH)/$(EXECUTABLE) $(OBJECT_PATH)/*.o
	 
install: $(EXECUTABLE)
	install -D $(BUILD_PATH)/$(EXECUTABLE) $(BIN_PATH)/$(EXECUTABLE)
	
memory: debug
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $(BUILD_PATH)/$(EXECUTABLE) --quiet samples/source.txt samples/key.txt "key string" prompt --dry_run

profile: debug
	valgrind --tool=callgrind --auto=yes --inclusive=yes --tree=both $(BUILD_PATH)/$(EXECUTABLE) --quiet samples/source.txt samples/key.txt "key string" prompt --dry_run

#---	

.PHONY: all debug clean install memory profile

#-------------------------------------------------------------------------------

vpath %.o $(OBJECT_PATH)
vpath %.c $(SOURCE_PATH)
vpath %.h $(INCLUDE_PATH)

$(EXECUTABLE): $(OBJECT_PATH)/*.o
	$(COMPILER) -o $(BUILD_PATH)/$@ $^ -I$(INCLUDE_PATH) $(COMPILER_GLOBAL_FLAGS)

$(OBJECT_PATH)/*.o: $(SOURCE_PATH)/*.c

$(SOURCE_PATH)/*.c: $(SOURCE_PATH)/%.c
	$(COMPILER) -o $(OBJECT_PATH)/$(strip $(subst .c,, $(subst $(SOURCE_PATH)/,, $@))).o -c $@ -I$(INCLUDE_PATH) $(COMPILER_GLOBAL_FLAGS)
	
$(SOURCE_PATH)/%.c:
	
