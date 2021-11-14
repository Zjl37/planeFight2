#include "pfUiCtrl.hpp"
#include "pfUI.hpp"
#include "pfGame.hpp"
#include "pfAI.hpp"

extern std::mt19937 rng;

void StartLocalGame();
void p2Ready();
void StartServer();
void PfServerStop();
void PfStopConnect();
void StartClient();
// TODO: move more implementation here

namespace pfui {

	int pageNum;
	std::string playername;

	namespace ctrl {
		std::string ipAddr;
		std::string errMsg;
		int p2SelectedFacing;

		void P0InputOK() {
			if(playername.empty()) return;
			NextPage(PfPage::main);
		}
		void P1PlayLocal() {
			StartLocalGame();
		}
		void P2Clear() {
			if(player[0]->GetGame().state & PfGame::me_ready) return;
			bf1.clear();
		}
		void P2Ready() {
			if(bf1.nPlaced != curGame.n) return;
			p2Ready();
		}
		void P5Surrender() {
			player[0]->Surrender();
		}
		void P7StartServer() {
			StartServer();
		}
		void P8ServerStop() {
			player[1].reset();
			PrevPage();
			PfServerStop();
		}
		void P9ClientStop() {
			player[1].reset();
			PrevPage();
			PfStopConnect();
		}
		void P9InputOK() {
			StartClient();
		}
	}
}