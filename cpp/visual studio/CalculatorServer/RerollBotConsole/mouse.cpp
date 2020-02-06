#include "clicker.hpp"

#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <sstream>


mouse::mouse(const cv::Rect2i& desktop, const cv::Rect2i& window)
	:
	desktop(desktop),
	window(window.area() ? window : desktop)
{
	//if (!desktop.contains(window.tl() + cv::Point2i(1, 1)) || !desktop.contains(window.br() - cv::Point2i(-1,-1)))
	//	throw std::invalid_argument("Window not in desktop area");
}


void mouse::click(const cv::Point2f& point)
{
	INPUT Inputs[3] = { 0 };
		
	cv::Point2i ndc = get_ndc_position(point);

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = ndc.x;
	Inputs[0].mi.dy = ndc.y;
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	Inputs[1].type = INPUT_MOUSE;
	Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	Inputs[2].type = INPUT_MOUSE;
	Inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(3, Inputs, sizeof(INPUT));
}

void mouse::move(const cv::Point2f& point)
{
	INPUT Inputs[1] = { 0 };

	cv::Point2i ndc = get_ndc_position(point);

	Inputs[0].type = INPUT_MOUSE;
	Inputs[0].mi.dx = ndc.x;
	Inputs[0].mi.dy = ndc.y;
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;


	SendInput(1, Inputs, sizeof(INPUT));
}

cv::Point2i mouse::get_ndc_position(const cv::Point2f relative_window_position) const
{
	int ndc_x = (window.x + window.width * relative_window_position.x) / desktop.width * 65535;
	int ndc_y = (window.y + window.height * relative_window_position.y) / desktop.height * 65535;
	return cv::Point2i(ndc_x, ndc_y);
}

void mouse::click_hotkey(int x, int y)
{

	std::stringstream command;
	command << "/f test.ahk click " << x << " " << y;

	execute_hotkey_script(command.str());

}

void mouse::move_hotkey(int x, int y)
{
	std::stringstream command;
	command << "/f test.ahk move " << x << " " << y;

	execute_hotkey_script(command.str());
}

void mouse::execute_hotkey_script(const std::string& console)
{
	// additional information
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcessA
	(
		"C:\Program Files\AutoHotkey\AutoHotKeyU64_UIA.exe",   // the path
		(LPSTR) console.c_str(),                // Command line
		NULL,                   // Process handle not inheritable
		NULL,                   // Thread handle not inheritable
		FALSE,                  // Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,     // Opens file in a separate console
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi           // Pointer to PROCESS_INFORMATION structure
	);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

