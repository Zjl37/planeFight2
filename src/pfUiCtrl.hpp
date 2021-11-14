#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/util/ref.hpp"
#include "pfCommon.hpp"

namespace pfui {

	extern int pageNum;
	extern std::string playername;
	extern ftxui::ScreenInteractive scr;
	extern int p2IsNetworkGame;
	extern bool p2ShowBanner;

	struct AttackIndicatorInfo {
		int x, y;
		bool signDir;
		enum {
			none,
			blink1,
			blink2,
			blink3,
			blink4,
			resulted
		} state = none;
		PfAtkRes res;

		std::string MakeCoordStr() const;
	};
	extern AttackIndicatorInfo p5AttackInfo;

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