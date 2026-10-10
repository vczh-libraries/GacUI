.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_TARGET=./Bin/GacBuild
CPP_OBJECTS=./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppGlrParser.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/VlppWorkflowCompiler.o ./Obj/VlppWorkflowLibrary.o ./Obj/VlppWorkflowRuntime.o ./Obj/GacBuild.o ./Obj/Main.o ./Obj/Native.Windows.o ./Obj/Native.Linux.o

CPP_COMPILE_OPTIONS=-DVCZH_DEBUG_METAONLY_REFLECTION -I ../../Import -I ../../Release
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi

all:pre-build ./Bin/GacBuild

$(CPP_TARGET) $(CPP_OBJECTS): | pre-build

$(CPP_TARGET):$(CPP_OBJECTS)
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppGlrParser.o: ../../Import/VlppGlrParser.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../Import/VlppReflection.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowCompiler.o: ../../Import/VlppWorkflowCompiler.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowLibrary.o: ../../Import/VlppWorkflowLibrary.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowRuntime.o: ../../Import/VlppWorkflowRuntime.cpp
	$(CPP_COMPILE)

./Obj/GacBuild.o: GacBuild/GacBuild.cpp
	$(CPP_COMPILE)

./Obj/Main.o: GacBuild/Main.cpp
	$(CPP_COMPILE)

./Obj/Native.Windows.o: GacBuild/Native.Windows.cpp
	$(CPP_COMPILE)

./Obj/Native.Linux.o: GacBuild/Native.Linux.cpp
	$(CPP_COMPILE)
