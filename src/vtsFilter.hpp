#pragma once
#include <deque>
#include <future>
#include <iostream>
#include <vector>
#include <sstream>
#include <functional>
#include <iomanip>
#include <algorithm>
#ifdef WIN32
#	include <windows.h>
#endif

#define ESC "\x1b"

class VtsInputFilter {
	std::deque<char> buf;
	std::future<void> futMouseHandler;
	std::future<void> futKeyHandler;
	std::promise<std::pair<int, int>> curPosPromise;
#ifdef WIN32
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
#endif

	std::string _readNextCodept();
	void _readEscSeq();
	void _CallMouseHandler(int, int, int, bool);
	void _CallKeyHander(const std::string &, const std::vector<int> &);

	public:
	bool fWork;
	bool fTextInputMode;
	std::function<void(int, int, int, bool)> mouseHandler;
	std::function<void(const std::string &, const std::vector<int> &)> keyHandler;

	VtsInputFilter() = default;
	void Work();
	std::string ReadLine();
	std::string PeekLine();
	void WaitForHandlerThreads();
	std::pair<int, int> GetLastCurPos();
};


#define SET_X10_MOUSE 9
#define SET_VT200_MOUSE 1000
#define SET_VT200_HIGHLIGHT_MOUSE 1001
#define SET_BTN_EVENT_MOUSE 1002
#define SET_ANY_EVENT_MOUSE 1003

#define SET_FOCUS_EVENT_MOUSE 1004

#define SET_ALTERNATE_SCROLL 1007

#define SET_EXT_MODE_MOUSE 1005
#define SET_SGR_EXT_MODE_MOUSE 1006
#define SET_URXVT_EXT_MODE_MOUSE 1015
#define SET_PIXEL_POSITION_MOUSE 1016

void VtEnableMouseTrackingAny();
void VtDisableMouseTracking();