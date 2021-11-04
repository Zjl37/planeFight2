#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/util/ref.hpp"

namespace pfui {

	extern int pageNum;
	extern std::string playername;
	extern ftxui::ScreenInteractive scr;
	extern int p2IsNetworkGame;

	namespace ctrl {
		extern std::string ipAddr;
		extern std::string errMsg;
		extern int p2SelectedFacing;

		void P0InputOK();
		void P1PlayLocal();
		void P2Clear();
		void P2Ready();
		void P5Surrender();
		void P7StartServer();
		void P8ServerStop();
		void P9ClientStop();
		void P9InputOK();
	}

	void Build();
	void Loop();
}