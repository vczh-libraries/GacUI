#include "GacGen.h"

void PrintErrorMessage(const WString& message)
{
	Console::SetColor(true, false, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}

void PrintSuccessMessage(const WString& message)
{
	Console::SetColor(false, true, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}

void PrintInformationMessage(const WString& message)
{
	Console::SetColor(true, true, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}