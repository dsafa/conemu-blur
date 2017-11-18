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

enum AccentPolicyFlags : unsigned int
{
	APPLY_TINT = 1 << 1,	//Applies tint using Color in AccentPolicy
	APPLY_DESKTOP = 1 << 2,	//Applies to whole desktop, for blur, only applies to the right and bottom of the window
};

struct AccentPolicy
{
	AccentState AccentState;
	unsigned int Flags;
	unsigned int Color;
	int AnimationId;
};

enum class CompositionAttribute
{
	ACCENT_POLICY = 19
};

struct WindowCompositionAttributeData
{
	CompositionAttribute Attribute;
	PVOID Data;
	ULONG DataSize;
};

using SetWindowCompositionAttribute_t = bool (WINAPI*)(HWND, WindowCompositionAttributeData*);

int main(int argc, char* argv[]) 
{
	HWND conEmuHwnd = FindWindowW(L"VirtualConsoleClass", nullptr);
	if (conEmuHwnd == nullptr) {
		std::cerr << "Could not find ConEmu window" << std::endl;
		return EXIT_FAILURE;
	}

	HMODULE module = LoadLibrary("user32.dll");
	if (module == nullptr) {
		std::cerr << "Error loading user32.dll" << std::endl;
		return EXIT_FAILURE;
	}

	auto SetWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttribute_t>(GetProcAddress(module, "SetWindowCompositionAttribute"));
	if (SetWindowCompositionAttribute == nullptr) {
		std::cerr << "Error getting function SetWindowCompositionAttribute" << std::endl;
		FreeLibrary(module);
		return EXIT_FAILURE;
	}

	unsigned int percent = 90;
	if (argc > 1) {
		std::istringstream iss{argv[1]};
		if (!(iss >> percent) || percent > 100) {
			std::cerr << "Argument should be a percentage 0 - 100 for the opacity" << std::endl;
			FreeLibrary(module);
			return EXIT_FAILURE;
		}
	}

	auto alpha = static_cast<unsigned int>(std::round((percent / 100.0f) * 255));
	auto colour = alpha << 24;

	AccentPolicy policy = { AccentState::BLURBEHIND, APPLY_TINT, colour, 0 };
	WindowCompositionAttributeData attribute = { CompositionAttribute::ACCENT_POLICY, &policy, sizeof(AccentPolicy) };
	if (!SetWindowCompositionAttribute(conEmuHwnd, &attribute)) {
		std::cerr << "Error setting window composition" << std::endl;
	}

	FreeLibrary(module);
}