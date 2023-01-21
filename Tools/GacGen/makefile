.PHONY: all clean pre-build
.DEFAULT_GOAL := all

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

all:pre-build ./Bin/GacGen

./Bin/GacGen:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppGlrParser.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppParser.o ./Obj/VlppReflection.o ./Obj/VlppRegex.o ./Obj/VlppWorkflowCompiler.o ./Obj/VlppWorkflowLibrary.o ./Obj/VlppWorkflowRuntime.o ./Obj/GacUI.o ./Obj/GacUICompiler.o ./Obj/GacUIReflection.o ./Obj/GacGen.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../Import/Vlpp.Linux.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppGlrParser.o: ../../Import/VlppGlrParser.cpp ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp ../../Import/VlppOS.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../Import/VlppOS.Linux.cpp ../../Import/VlppOS.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppParser.o: ../../Import/VlppParser.cpp ../../Import/VlppParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../Import/VlppReflection.cpp ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../Import/VlppRegex.cpp ../../Import/VlppRegex.h ../../Import/Vlpp.h ../../Import/VlppOS.h
	$(CPP_COMPILE)

./Obj/VlppWorkflowCompiler.o: ../../Import/VlppWorkflowCompiler.cpp ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowRuntime.h ../../Import/VlppGlrParser.h
	$(CPP_COMPILE)

./Obj/VlppWorkflowLibrary.o: ../../Import/VlppWorkflowLibrary.cpp ../../Import/VlppWorkflowLibrary.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/VlppWorkflowRuntime.o: ../../Import/VlppWorkflowRuntime.cpp ../../Import/VlppWorkflowRuntime.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h
	$(CPP_COMPILE)

./Obj/GacUI.o: ../../Release/GacUI.cpp ../../Release/GacUI.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppParser.h ../../Release/GacUIReflection.h ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowRuntime.h
	$(CPP_COMPILE)

./Obj/GacUICompiler.o: ../../Release/GacUICompiler.cpp ../../Release/GacUICompiler.h ../../Release/GacUIReflection.h ../../Release/GacUI.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppParser.h ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowRuntime.h
	$(CPP_COMPILE)

./Obj/GacUIReflection.o: ../../Release/GacUIReflection.cpp ../../Release/GacUIReflection.h ../../Release/GacUI.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppParser.h ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowRuntime.h
	$(CPP_COMPILE)

./Obj/GacGen.o: GacGen/GacGen.cpp GacGen/GacGen.h ../../Release/GacUICompiler.h ../../Release/GacUIReflection.h ../../Release/GacUI.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppParser.h ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowRuntime.h
	$(CPP_COMPILE)

./Obj/Main.o: GacGen/Main.cpp GacGen/GacGen.h ../../Release/GacUICompiler.h ../../Release/GacUIReflection.h ../../Release/GacUI.h ../../Import/VlppGlrParser.h ../../Import/VlppReflection.h ../../Import/VlppOS.h ../../Import/Vlpp.h ../../Import/VlppRegex.h ../../Import/VlppWorkflowLibrary.h ../../Import/VlppParser.h ../../Import/VlppWorkflowCompiler.h ../../Import/VlppWorkflowRuntime.h
	$(CPP_COMPILE)
