.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../Import -I ../../Release
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi

all:pre-build ./Bin/UnitTest

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/VlppWorkflowCompiler.o ./Obj/VlppWorkflow.o ./Obj/GacUI.o ./Obj/GacUIReflection.o ./Obj/GacUICompiler.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppWorkflowCompiler.o: ../../Import/VlppWorkflowCompiler.cpp ../../Import/VlppWorkflowCompiler.h ../../Import/Vlpp.h ../../Import/VlppWorkflow.h
	$(CPP_COMPILE)

./Obj/VlppWorkflow.o: ../../Import/VlppWorkflow.cpp ../../Import/VlppWorkflow.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/GacUI.o: ../../Release/GacUI.cpp ../../Release/GacUI.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/GacUIReflection.o: ../../Release/GacUIReflection.cpp ../../Release/GacUIReflection.h ../../Import/Vlpp.h ../../Import/VlppWorkflow.h ../../Release/GacUI.h
	$(CPP_COMPILE)

./Obj/GacUICompiler.o: ../../Release/GacUICompiler.cpp ../../Release/GacUICompiler.h ../../Import/Vlpp.h ../../Import/VlppWorkflow.h ../../Import/VlppWorkflowCompiler.h ../../Release/GacUI.h ../../Release/GacUIReflection.h
	$(CPP_COMPILE)

./Obj/Main.o: Main.cpp ../../Release/GacUI.h ../../Import/Vlpp.h
	$(CPP_COMPILE)
