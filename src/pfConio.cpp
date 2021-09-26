#include "pfConio.hpp"
#include "pfConsole.hpp"

/** NOTE:
 ** Before https://github.com/microsoft/terminal/issues/4551 is solved,
 ** it is impossible to read UTF-8 input from standard input.
 **/

#ifdef _WIN32
#	include <windows.h>

extern HANDLE hIn, hOut;

std::string PfConioContext::_readNextCodept() {
	static wchar_t res[3];
	static char u8res[5];
	static INPUT_RECORD e;
	static DWORD dw = 0;

	u8res[0] = 0;
	while(!u8res[0]) {
		if(!ReadConsoleInputW(hIn, &e, 1, &dw)) {
			std::clog << "[!] in " << __PRETTY_FUNCTION__ << " ReadConsoleInputW failed with error " << GetLastError() << std::endl;
			return {};
		}
		if(e.EventType == KEY_EVENT) {
			if(e.Event.KeyEvent.bKeyDown) {
				const auto &wch = e.Event.KeyEvent.uChar.UnicodeChar;
				if(IS_HIGH_SURROGATE(wch)) {
					res[0] = wch;
				} else if(IS_LOW_SURROGATE(wch)) {
					res[1] = wch;
					res[2] = 0;
					WideCharToMultiByte(65001, 0, res, -1, u8res, 5, NULL, NULL);
				} else {
					res[0] = wch;
					res[1] = 0;
					WideCharToMultiByte(65001, 0, res, -1, u8res, 5, NULL, NULL);
				}
			}
		} else if(e.EventType == WINDOW_BUFFER_SIZE_EVENT) {
			// If on Windows, we detect window event here.
			if(resizeHandler) _CallResizeHandler(GetConScrSize());
		}
	}
	return u8res;
}
#else
std::string PfConioContext::_readNextCodept() {
	char res[5]="\0\0\0\0";
	res[0] = std::cin.get();
	if((res[0] & 0x80) == 0) return res;
	res[1] = std::cin.get();
	if((res[0] & 0x20) == 0) return res;
	res[2] = std::cin.get();
	if((res[0] & 0x10) == 0) return res;
	res[3] = std::cin.get();
	return res;
}
#endif

template <typename Container>
void pop_back_utf8(Container &s) {
	while(!s.empty() && (s.back() & 0xc0) == 0x80)
		s.pop_back();
	if(!s.empty())
		s.pop_back();
}

int scrW, scrH;

void PfConioContext::Work() {
	fWork = 1;
	while(fWork) {
		std::string s = _readNextCodept();
		if(s == "\x1b") { // ESC
			_readEscSeq();
		} else if(fTextInputMode) {
			if(s == "\x7f") { // Backspace
				pop_back_utf8(buf);
				_CallKeyHandler(s, {});
			} else if(s == "\r" || s == "\n") {
				buf.emplace_back('\n');
				_CallKeyHandler("\n", {});
			} else {
				for(char ch: s)
					buf.emplace_back(ch);
				std::cout << s;
			}
		} else {
			if(s <= "\x7f") _CallKeyHandler(s == "\r" ? "\n" : s, {});
		}
	}
}

inline void print_detail(char ch) {
	if(ch < 32 || ch == 127) {
		std::clog << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (unsigned)(unsigned char)ch;
	} else {
		std::clog << ch;
	}
}

void PfConioContext::_readEscSeq() {
	std::string s = "", t = "0";
	auto isEscSeqTermChar = [](std::string c) {
		return c <= " " || (c >= "A" && c <= "Z") || c >= "a";
	};
	while(!isEscSeqTermChar(t)) {
		s += t = _readNextCodept();
	}
	
	char ch;
	if(s[0] == '[') { // CSI goes here
		if(s[1] == '<') {
			int state, x, y;
			std::stringstream(s) >> ch >> ch >> state >> ch >> x >> ch >> y >> ch;
			_CallMouseHandler(state, x - 1, y - 1, ch == 'M');
		} else if(s.back() == 'R') {
			int x, y;
			std::stringstream(s) >> ch >> y >> ch >> x >> ch;
			curPosPromise.set_value({ x - 1, y - 1 });
		} else if(s.back() == '~') {
			std::vector<int> v;
			std::stringstream ss(s);
			ss >> ch;
			while(ch != '~') {
				int nv;
				ss >> nv >> ch;
				v.push_back(nv);
			}
			_CallKeyHandler("", v);
		} else {
			std::clog << "[INFO] vtIn read unhandled Escape Sequence: `";
			for(char ch: s) print_detail(ch);
			std::clog << "`" << std::endl;
		}
	} else {
		std::clog << "[INFO] vtIn read unhandled Escape Sequence: `";
		for(char ch: s) print_detail(ch);
		std::clog << "`" << std::endl;
	}
}

void PfConioContext::_CallMouseHandler(int st, int x, int y, bool b) {
	if(mouseHandler) {
		if(futMouseHandler.valid()) {
			using namespace std::chrono;
			if(futMouseHandler.wait_for(0s) != std::future_status::ready)
				return;
		}
		futMouseHandler = std::async(mouseHandler, st, x, y, b);
	}
}

void PfConioContext::_CallKeyHandler(const std::string &s, const std::vector<int> &v) {
	if(keyHandler) {
		if(futKeyHandler.valid()) {
			using namespace std::chrono;
			if(futKeyHandler.wait_for(0s) != std::future_status::ready)
				return;
		}
		futKeyHandler = std::async(keyHandler, s, v);
	}
}

void PfConioContext::_CallResizeHandler(std::pair<int, int> size) {
	static std::future<void> futRszHandler;
	static std::atomic_bool fUpdate;
	static std::atomic_int nw, nh;
	std::tie(nw, nh) = size;
	fUpdate = 1;
	if(futRszHandler.valid()) {
		using namespace std::chrono;
		if(futRszHandler.wait_for(0s) != std::future_status::ready) return;
	}
	futRszHandler = std::async([&]() {
		while(fUpdate) {
			fUpdate = false;
			resizeHandler({nw, nh});
		}
	});
}

std::string PfConioContext::ReadLine() {
	std::string res;
	while(!buf.empty() && buf.front() != '\n') {
		res += buf.front();
		buf.pop_front();
	}
	if(!buf.empty()) buf.pop_front();
	return res;
}

std::string PfConioContext::PeekLine() {
	std::string res;
	copy(buf.begin(),
	     min(buf.end(), find(buf.begin(), buf.end(), '\n')),
	     std::back_inserter(res));
	return res;
}

void PfConioContext::WaitForHandlerThreads() {
	if(futMouseHandler.valid())
		futMouseHandler.get();
	if(futKeyHandler.valid())
		futKeyHandler.get();
}

std::pair<int, int> PfConioContext::GetLastCurPos() {
	auto fut = curPosPromise.get_future();
	if(fut.wait_for(std::chrono::seconds(2)) == std::future_status::timeout) {
		return {-1, -1};
	}
	curPosPromise = std::promise<std::pair<int, int>>();
	return fut.get();
}

void VtEnableMouseTrackingAny() {
	std::cout << ESC "[?1003;1006h" << std::flush;
}

void VtDisableMouseTracking() {
	std::cout << ESC "[?1003;1006l" << std::flush;
}