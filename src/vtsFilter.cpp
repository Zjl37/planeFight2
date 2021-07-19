#include "vtsFilter.hpp"

/** NOTE:
 ** Before https://github.com/microsoft/terminal/issues/4551 is solved,
 ** it is impossible to read UTF-8 input from standard input.
 **/

#ifdef WIN32
std::string VtsInputFilter::_readNextCodept() {
	wchar_t res[3] = L"\0\0";
	char u8res[10];
	DWORD dw;

	ReadConsoleW(hIn, res, 1, &dw, nullptr);
	if(IS_HIGH_SURROGATE(res[0])) {
		ReadConsoleW(hIn, res + 1, 1, &dw, nullptr);
		WideCharToMultiByte(65001, 0, res, -1, u8res, 10, nullptr, nullptr);
	} else {
		WideCharToMultiByte(65001, 0, res, -1, u8res, 10, nullptr, nullptr);
	}
	return u8res;
}
#else
std::string VtsInputFilter::_readNextCodept() {
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

void VtsInputFilter::Work() {
	fWork = 1;
	while(fWork) {
		std::string s = _readNextCodept();
		if(s == "\x1b") { // ESC
			_readEscSeq();
		} else if(fTextInputMode) {
			if(s == "\x7f") { // Backspace
				pop_back_utf8(buf);
				if(keyHandler) keyHandler(s);
			} else if(s == "\r" || s == "\n") {
				buf.emplace_back('\n');
				if(keyHandler) keyHandler("\n");
			} else {
				for(char ch: s)
					buf.emplace_back(ch);
				std::cout << s;
			}
		} else {
			if(s <= "\x7f" &&  keyHandler) keyHandler(s == "\r" ? "\n" : s);
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

void VtsInputFilter::_readEscSeq() {
	std::string s = "", t = "0";
	auto isEscSeqTermChar = [](std::string c) {
		return c <= " " || (c >= "A" && c <= "Z") || c >= "a";
	};
	while(!isEscSeqTermChar(t)) {
		s += t = _readNextCodept();
	}
	// std::clog << "[INFO] in " << __PRETTY_FUNCTION__ << " read: `";
	// for(char ch: s) print_detail(ch);
	// std::clog << "`" << std::endl;
	
	if(s[0] == '[') { // CSI goes here
		if(s[1] == '<') {
			int state, x, y;
			char ch;
			std::stringstream(s) >> ch >> ch >> state >> ch >> x >> ch >> y >> ch;
			if(mouseHandler) mouseHandler(state, x - 1, y - 1, ch == 'M');
		}
	}
}

std::string VtsInputFilter::ReadLine() {
	std::string res;
	while(!buf.empty() && buf.front() != '\n') {
		res += buf.front();
		buf.pop_front();
	}
	if(!buf.empty()) buf.pop_front();
	return res;
}

std::string VtsInputFilter::PeekLine() {
	std::string res;
	copy(buf.begin(),
	     min(buf.end(), find(buf.begin(), buf.end(), '\n')),
	     std::back_inserter(res));
	return res;
}

void VtEnableMouseTrackingAny() {
	std::cout << ESC "[?1003;1006h" << std::flush;
}