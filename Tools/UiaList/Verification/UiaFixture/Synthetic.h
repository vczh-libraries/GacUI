#ifndef UIAFIXTURE_SYNTHETIC
#define UIAFIXTURE_SYNTHETIC

#include <Windows.h>

// The synthetic capability fixture is separate from the ordinary native controls.
HWND CreateSyntheticFixture(HINSTANCE instance, bool stress = false);
void DestroySyntheticFixture(HWND window);

#endif
