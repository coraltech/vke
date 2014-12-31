#-------------------------------------------------------------------------------

COMPILER=gcc
COMPILER_GLOBAL_FLAGS=

BUILD_PATH=build
SOURCE_PATH=lib
INCLUDE_PATH=include

#-------------------------------------------------------------------------------

all: vke

vke: vke.o
	 $(COMPILER) -o $(BUILD_PATH)/vke $(BUILD_PATH)/vke.o -I$(INCLUDE_PATH) $(COMPILER_GLOBAL_FLAGS)

vke.o:
	 $(COMPILER) -o $(BUILD_PATH)/vke.o -c $(SOURCE_PATH)/vke.c -I$(INCLUDE_PATH) $(COMPILER_GLOBAL_FLAGS)
     
clean:
	 rm $(BUILD_PATH)/vke $(BUILD_PATH)/*.o
