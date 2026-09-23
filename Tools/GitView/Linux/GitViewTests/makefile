.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-DVCZH_DEBUG_NO_REFLECTION -I ../../../../Import -I ../../../../Release
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi

all:pre-build ./Bin/GitViewTests

./Bin/GitViewTests:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/VlppRegex.o ./Obj/VlppReflection.o ./Obj/VlppGlrParser.o ./Obj/VlppWorkflowLibrary.o ./Obj/GacUI.o ./Obj/TuiSkin.o ./Obj/GitViewPartialClasses.o ./Obj/GitViewReflection.o ./Obj/GitViewResource.o ./Obj/Git.o ./Obj/Command.Linux.o ./Obj/GitViewModel.o ./Obj/Main.o ./Obj/TestGit.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppRegex.o: ../../../../Import/VlppRegex.cpp
	$(CPP_COMPILE)

./Obj/VlppReflection.o: ../../../../Import/VlppReflection.cpp
	$(CPP_COMPILE)

./Obj/VlppGlrParser.o: ../../../../Import/VlppGlrParser.cpp
	$(CPP_COMPILE)

./Obj/VlppWorkflowLibrary.o: ../../../../Import/VlppWorkflowLibrary.cpp
	$(CPP_COMPILE)

./Obj/GacUI.o: ../../../../Release/GacUI.cpp
	$(CPP_COMPILE)

./Obj/TuiSkin.o: ../../../../Release/TuiSkin.cpp
	$(CPP_COMPILE)

./Obj/GitViewPartialClasses.o: ../../GitView/Source/GitViewPartialClasses.cpp
	$(CPP_COMPILE)

./Obj/GitViewReflection.o: ../../GitView/Source/GitViewReflection.cpp
	$(CPP_COMPILE)

./Obj/GitViewResource.o: ../../GitView/Source/GitViewResource.cpp
	$(CPP_COMPILE)

./Obj/Git.o: ../../GitView/Model/Git.cpp
	$(CPP_COMPILE)

./Obj/Command.Linux.o: ../../GitView/Model/Command.Linux.cpp
	$(CPP_COMPILE)

./Obj/GitViewModel.o: ../../GitView/ViewModel/GitViewModel.cpp
	$(CPP_COMPILE)

./Obj/Main.o: ../../GitViewTests/Main.cpp
	$(CPP_COMPILE)

./Obj/TestGit.o: ../../GitViewTests/TestGit.cpp
	$(CPP_COMPILE)
