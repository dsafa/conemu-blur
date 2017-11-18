#include <Windows.h>
#include <iostream>
#include <sstream>
#include <cmath>

enum class AccentState
{
	DISABLED,
	GRADIENT,
	GRADIENT_TRANSPARENT,
	BLURBEHIND,
};

enum AccentPolicyFlags : int
{
	APPLY_WINDOW = 1,		//Apply to window
	APPLY_TINT = 1 << 1,	//Applies tint using Color in AccentPolicy
	APPLY_DESKTOP = 1 << 2,	//Applies to whole desktop, for blur, only applies to the right and bottom of the window
};

struct AccentPolicy
{
	AccentState AccentState;
	int Flags;
	int Color;
	int AnimationId;
};

enum class CompositionAttribute
{
	ACCENT_POLICY = 19
};

struct WindowsCompositionAttributeData
{
	CompositionAttribute Attribute;
	PVOID Data;
	ULONG DataSize;
};

typedef BOOL(WINAPI *SetWindowCompositionAttribute_t)(HWND, WindowsCompositionAttributeData*);
static SetWindowCompositionAttribute_t SetWindowCompositionAttribute = nullptr;
static HINSTANCE module = nullptr;

int main(int argc, char* argv[]) 
{
	HWND conEmuHwnd = FindWindowW(L"VirtualConsoleClass", NULL);
	if (conEmuHwnd == NULL) {
		std::cerr << "Could not find ConEmu window" << std::endl;
		return EXIT_FAILURE;
	}

	module = LoadLibrary("user32.dll");
	if (module == nullptr) {
		std::cerr << "Error loading user32.dll" << std::endl;
		return EXIT_FAILURE;
	}

	SetWindowCompositionAttribute = (SetWindowCompositionAttribute_t)GetProcAddress(module, "SetWindowCompositionAttribute");
	if (SetWindowCompositionAttribute == nullptr) {
		std::cerr << "Error getting function SetWindowCompositionAttribute" << std::endl;
		FreeLibrary(module);
		return EXIT_FAILURE;
	}

	unsigned int percent = 90;
	if (argc > 1) {
		std::istringstream iss{ argv[1] };
		if (!(iss >> percent) || percent > 100) {
			std::cerr << "Argument should be a percentage 0 - 100 for the transparency" << std::endl;
			FreeLibrary(module);
			return EXIT_FAILURE;
		}
	}

	unsigned int alpha = static_cast<unsigned int>(std::round((percent / 100.0f) * 255));
	int colour = static_cast<int>(alpha << 24 | 0);

	AccentPolicy policy = { AccentState::BLURBEHIND, APPLY_TINT, colour, 0 };
	WindowsCompositionAttributeData attribute = { CompositionAttribute::ACCENT_POLICY, &policy, sizeof(AccentPolicy) };
	SetWindowCompositionAttribute(conEmuHwnd, &attribute);

	FreeLibrary(module);
}