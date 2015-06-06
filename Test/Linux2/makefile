# This file is generated from "./makefile.debug.makegen" by Vczh Makefile Generator

# Config
.PHONY : all clean

# Targets
bin_TARGET = ././Bin/
obj_TARGET = ././Obj/

# Folders
GACUI_IMPORT_DIR = ./../../Import/
GACUI_IMPORT_cpp = $(wildcard $(GACUI_IMPORT_DIR)*.cpp)
GACUI_IMPORT_h = $(wildcard $(GACUI_IMPORT_DIR)*.h)
GACUI_MAIN_DIR = ././
GACUI_MAIN_cpp = $(GACUI_MAIN_DIR)Main.cpp
GACUI_RELEASE_DIR = ./../../Release/
GACUI_RELEASE_cpp = $(GACUI_RELEASE_DIR)GacUI.cpp $(GACUI_RELEASE_DIR)GacUIReflection.cpp
GACUI_RELEASE_h = $(GACUI_RELEASE_DIR)GacUI.h $(GACUI_RELEASE_DIR)GacUIReflection.h

# Output Categories
GACUI_IMPORT_o = $(patsubst $(GACUI_IMPORT_DIR)%.cpp, $(obj_TARGET)%.o, $(GACUI_IMPORT_cpp))
GACUI_MAIN_o = $(patsubst $(GACUI_MAIN_DIR)%.cpp, $(obj_TARGET)%.o, $(GACUI_MAIN_cpp))
GACUI_RELEASE_o = $(patsubst $(GACUI_RELEASE_DIR)%.cpp, $(obj_TARGET)%.o, $(GACUI_RELEASE_cpp))

# All
ALL_o = $(GACUI_IMPORT_o) $(GACUI_MAIN_o) $(GACUI_RELEASE_o)
all : $(ALL_o)
	clang++ -std=c++11 -pthread -g -o $(bin_TARGET)UnitTest $(ALL_o)

# Dependencies
# $(GACUI_RELEASE_h) : $(GACUI_IMPORT_h)
# $(GACUI_IMPORT_cpp) : $(GACUI_IMPORT_h)
# $(GACUI_MAIN_cpp) : $(GACUI_RELEASE_h)
# $(GACUI_RELEASE_cpp) : $(GACUI_RELEASE_h)

# Rules
$(GACUI_IMPORT_o) : $(obj_TARGET)%.o : $(GACUI_IMPORT_DIR)%.cpp $(GACUI_IMPORT_h)
	clang++ -std=c++11 -g -I ../../Import -I ../../Release -o $@ -c $<
$(GACUI_MAIN_o) : $(obj_TARGET)%.o : $(GACUI_MAIN_DIR)%.cpp $(GACUI_IMPORT_h) $(GACUI_RELEASE_h)
	clang++ -std=c++11 -g -I ../../Import -I ../../Release -o $@ -c $<
$(GACUI_RELEASE_o) : $(obj_TARGET)%.o : $(GACUI_RELEASE_DIR)%.cpp $(GACUI_IMPORT_h) $(GACUI_RELEASE_h)
	clang++ -std=c++11 -g -I ../../Import -I ../../Release -o $@ -c $<

# Clean
clean:
	rm $(bin_TARGET)* -rf
	rm $(obj_TARGET)* -rf
