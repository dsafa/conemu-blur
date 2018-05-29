#include "CLI11.hpp"
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
	ACRYLIC
};

enum AccentPolicyFlags : unsigned int
{
	APPLY_TINT = 1 << 1,	//Applies tint using Color in AccentPolicy
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
	const auto conEmuHwnd = FindWindowW(L"VirtualConsoleClass", nullptr);
	if (conEmuHwnd == nullptr) {
		std::cerr << "Could not find ConEmu window" << std::endl;
		return EXIT_FAILURE;
	}

	const auto module = LoadLibrary("user32.dll");
	if (module == nullptr) {
		std::cerr << "Error loading user32.dll" << std::endl;
		return EXIT_FAILURE;
	}

	const auto SetWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttribute_t>(GetProcAddress(module, "SetWindowCompositionAttribute"));
	if (SetWindowCompositionAttribute == nullptr) {
		std::cerr << "Error getting function SetWindowCompositionAttribute" << std::endl;
		FreeLibrary(module);
		return EXIT_FAILURE;
	}

	const auto OPTION_BLURBEHIND = "normal";
	const auto OPTION_ACRYLIC = "acrylic";
	std::string type;
	unsigned int percent = 90;

	CLI::App app{ "ConEmu blur" };
	app.add_set("-t,--type", type, { OPTION_BLURBEHIND, OPTION_ACRYLIC }, "Blur type");
	app.add_option("-o, --opacity", percent, "Opacity from 0 - 100");
	CLI11_PARSE(app, argc, argv);

	if (percent > 100) {
		std::cerr << "Argument should be a percentage 0 - 100 for the opacity" << std::endl;
		FreeLibrary(module);
		return EXIT_FAILURE;
	}

	const auto alpha = static_cast<unsigned int>(std::round((percent / 100.0f) * 255));
	const auto colour = alpha << 24;

	AccentState state = AccentState::BLURBEHIND;
	if (type == OPTION_BLURBEHIND) {
		state = AccentState::BLURBEHIND;
	} else if (type == OPTION_ACRYLIC) {
		state = AccentState::ACRYLIC;
	}

	AccentPolicy policy = { state, APPLY_TINT, colour, 0 };
	WindowCompositionAttributeData attribute = { CompositionAttribute::ACCENT_POLICY, &policy, sizeof(AccentPolicy) };
	if (!SetWindowCompositionAttribute(conEmuHwnd, &attribute)) {
		std::cerr << "Error setting window composition" << std::endl;
	}

	FreeLibrary(module);
}