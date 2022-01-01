/**
 * Copyright © 2021 Zjl37 <2693911885@qq.com>
 *
 * This file is part of Zjl37/planeFight2.
 *
 * PlaneFight2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "pfExtFtxui.hpp"
#include "uiCtrl.hpp"
#include "ui.hpp"
#include "pfLocale.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include <memory>
#include <numeric>
#include <random>
#include <string>

extern std::mt19937 rng;

extern bool isFirst;

/* clang-format off */
namespace ftxui::pfext { // planeFight's extension
	Component FlatButton(ConstStringRef label, std::function<void()> on_click, Decorator dec) {
		static ButtonOption btnNoBorder{false};
		auto btn = Button(label, on_click, btnNoBorder);
		return Renderer(btn, [=]() { return btn->Render() | dec; });
	}
	Component FlatButton(ConstStringRef label, std::function<void()> on_click) {
		static ButtonOption btnNoBorder{false};
		auto btn = Button(label, on_click, btnNoBorder);
		return Renderer(btn, [=]() { return btn->Render(); });
	}
	Element BasicPfBattleField(const PfBF &bf, Box *box) {
		const int PFBF_CELL_WIDTH = 2;
		Elements lines;
		for(int i = 0; i < bf.h; i++) {
			Elements cells;
			for(int j = i*bf.w; j < (i+1)*bf.w; ++j) {
				Decorator dec = size(WIDTH, EQUAL, PFBF_CELL_WIDTH);
				if(bf.mk[j] == PfBF::empty) {
					dec = dec | bgcolor(Color::Green);
				} else if(bf.mk[j] == PfBF::hit) {
					dec = dec | bgcolor(Color::Red);
				} else if(bf.mk[j] == PfBF::destroy) {
					dec = dec | bgcolor(Color::DarkRed);
				}
				cells.push_back(text(bf.ch[j]) | dec);
			}
			lines.push_back(hbox(cells));
		}
		auto dec = size(WIDTH, EQUAL, bf.w * 2) | size(HEIGHT, EQUAL, bf.h);
		if(box) {
			dec = dec | reflect(*box);
		}
		return hbox({
			filler() | size(WIDTH, EQUAL, 1),
			vbox(lines) | dec
		});
	}
	Element PfBattleFieldStatic(const PfBF &bf, Box *box) {
		// cut tail zeros off and return the last digit.
		auto SigDigit = [](unsigned x) {
			if(x == 0) return x;
			while(x % 10 == 0) {
				x /= 10;
			}
			return x % 10;
		};
		std::string xscale(6 + 2*bf.w, ' ');
		for(int i = 0; i < bf.w; ++i) {
			xscale[4 + 2*i] = '0' + SigDigit(i);
		}
		Elements yscale(1, text(""));
		for(int j = 0; j < bf.h; ++j) {
			using namespace std::string_literals;
			yscale.push_back(text(char('0' + SigDigit(j)) + " "s));
		}
		return vbox({
			text(xscale),
			hbox({
				vbox(yscale),
				borderDouble(BasicPfBattleField(bf, box))
			})
		});
	}
	Component PfBattleFieldPrepare(PfBF &bf, const pfGameInfo &gamerules, int &selectedFacing) {
		bf.clear();
		struct BfInteractInfo {
			Box boxBf;
			bool focused;
			struct {
				int x = 0, y = 0;
			} cur;
		};
		auto ii = std::make_shared<BfInteractInfo>();
		return CatchEvent(
			Renderer([&, ii](bool focus) {
				ii->focused = focus;
				if(focus) {
					return dbox({
						PfBattleFieldStatic(bf, &ii->boxBf),
						vbox({
							filler() | size(HEIGHT, EQUAL, 2 + ii->cur.y),
							hbox({
								filler() | size(WIDTH, EQUAL, 4 + ii->cur.x * 2),
								// cursor
								bf.pl[ii->cur.x + ii->cur.y * bf.w] & 8 ?
									text("×") | bgcolor(Color::Red) :
									bf.TestPlace(ii->cur.x, ii->cur.y, selectedFacing, gamerules.cw) ?
									text("□") | color(Color::White) :
									text("×") | color(Color::Red),
							})
						})
					});
				}
				return PfBattleFieldStatic(bf, &ii->boxBf);
			}),
			[&, ii](Event e) {
				if(e.is_mouse()) {
					if(!ii->boxBf.Contain(e.mouse().x, e.mouse().y)) return false;
					int bx = (e.mouse().x - ii->boxBf.x_min) / 2, by = e.mouse().y - ii->boxBf.y_min;
					if(e.mouse().button == Mouse::WheelUp) {
						++selectedFacing %= 4;
					} else if(e.mouse().button == Mouse::WheelDown) {
						(selectedFacing += 3) %= 4;
					} else if(e.mouse().button == Mouse::None) {
						ii->cur.x = bx;
						ii->cur.y = by;
					} else if(e.mouse().motion == Mouse::Pressed) {
						if(e.mouse().button == Mouse::Left && bf.nPlaced < gamerules.n) {
							bf.placeplane(bx, by, selectedFacing, gamerules.cw);
						}
					}
					return true;
				} else if(e.is_character()) {
					switch(e.character()[0]) {
					case '+':
					case '=':
					case ']':
						++selectedFacing %= 4;
						break;
					case '-':
					case '[':
						(selectedFacing += 3) %= 4;
						break;
					case 'h':
						(ii->cur.x += bf.w - 1) %= bf.w;
						break;
					case 'j':
						++ii->cur.y %= bf.h;
						break;
					case 'k':
						(ii->cur.y += bf.h - 1) %= bf.h;
						break;
					case 'l':
						++ii->cur.x %= bf.w;
						break;
					default:
						return false;
					}
					return true;
				} else {
					if(e == e.ArrowUp) {
						if(ii->cur.y <= 0) {
							ii->cur.y = 0;
							return false;
						}
						--ii->cur.y;
					} else if(e == e.ArrowDown) {
						if(ii->cur.y >= bf.h - 1) {
							ii->cur.y = bf.h - 1;
							return false;
						}
						++ii->cur.y;
					} else if(e == e.ArrowLeft) {
						if(ii->cur.x <= 0) {
							ii->cur.x = 0;
							return false;
						}
						--ii->cur.x;
					} else if(e == e.ArrowRight) {
						if(ii->cur.x >= bf.w - 1) {
							ii->cur.x = bf.w - 1;
							return false;
						}
						++ii->cur.x;
					} else if(e == e.Return) {
						int bx = ii->cur.x, by = ii->cur.y;
						if(bf.pl[ii->cur.x + ii->cur.y * bf.w] & 8) {
							bf.RemovePlane(bx, by);
						} else if(bf.nPlaced < gamerules.n) {
							bf.placeplane(bx, by, selectedFacing, gamerules.cw);
						}
					} else if(e.input() == "\x06") { // C-f
						++ii->cur.x %= bf.w;
					} else if(e.input() == "\x02") { // C-b
						(ii->cur.x += bf.w - 1) %= bf.w;
					} else if(e.input() == "\x0e") { // C-n
						++ii->cur.y %= bf.h;
					} else if(e.input() == "\x10") { // C-p
						(ii->cur.y += bf.h - 1) %= bf.h;
					} else if(e.input() == "\x01") { // C-a
						ii->cur.x = 0;
					} else if(e.input() == "\x05") { // C-e
						ii->cur.x = bf.w - 1;
					} else if(e.input() == "\x1b<") { // M-<
						ii->cur.y = 0;
					} else if(e.input() == "\x1b>") { // M->
						ii->cur.y = bf.h - 1;
					} else return false;
					return true;
				}
				return false;
			}
		);
	}
	Element BackgroundWithScatteredPlane() {
		if(pfui::scr.dimx() > 2*bg.w+1 || pfui::scr.dimy() > bg.h) {
			bg.resize(pfui::scr.dimx()/2, pfui::scr.dimy());
			for(int i = 0; i < (int)bg.w * bg.h / 25; i++)
				bg.placeplane(rng() % bg.w, rng() % bg.h, rng() & 3, false);
		}
		return BasicPfBattleField(bg);
	}
	Component Park(int &selectedFacing) {
		return Container::Horizontal({
			Container::Vertical({
				Renderer([]() { return text("Facing") | bold; }),
				Container::Vertical(
					{
						MenuEntry("up"),
						MenuEntry("right"),
						MenuEntry("down"),
						MenuEntry("left"),
					},
					&selectedFacing
				),
			}),
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
			Container::Tab({
				Renderer([]() { return vbox({
					text("              "),
					text("              "),
					text("      ┃       "),
					text("  ━━━━╋━━━━   "),
					text("      ┃       "),
					text("    ━━┻━━     "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("      ┃       "),
					text("  ┃   ┃       "),
					text("  ┣━━━╋━━     "),
					text("  ┃   ┃       "),
					text("      ┃       "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("    ━━┳━━     "),
					text("      ┃       "),
					text("  ━━━━╋━━━━   "),
					text("      ┃       "),
					text("              "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
				Renderer([]() { return vbox({
					text("              "),
					text("      ┃       "),
					text("      ┃   ┃   "),
					text("    ━━╋━━━┫   "),
					text("      ┃   ┃   "),
					text("      ┃       "),
					text("              "),
				}) | color(Color::Black) | bgcolor(Color::White); }),
			}, &selectedFacing)
		});
	}
	Component GameInfoInteractive(pfGameInfo &gamerules, PfBF &bf) {
		using namespace std::string_literals;

		CheckboxOption chkboxToggleCwOpt;
		chkboxToggleCwOpt.on_change = [&]() {
			if(!gamerules.cw) bf.clear();
		};

		auto component = Container::Vertical({
			Renderer([&]() {
				return text(TT("You are going to play with ").str() + (player[1] ? player[1]->GetName() : "???"));
			}),
			
			Container::Horizontal({
				FlatButton(TT("－").str(), [&]() {
					gamerules.n = std::max(gamerules.n - 1, 1);
					if(gamerules.n < bf.nPlaced) bf.clear();
				}, bgcolor(Color::Yellow)),
				Renderer([&]() {
					return text("  "s + TT("Number of planes: ").str() + std::to_string(gamerules.n) + "  ");
				}),
				FlatButton(TT("＋").str(), [&]() {
					++gamerules.n;
				}, bgcolor(Color::Yellow)),
			}),
			Checkbox(TT("Enable cross-border mode").str(), &gamerules.cw, chkboxToggleCwOpt),
			Checkbox(TT("Enable completely destroy").str(), &gamerules.cd),
			Container::Horizontal({
				Renderer([&]() {
					return text(TT("Map size: ").str() + std::to_string(gamerules.h) + "x" + std::to_string(gamerules.w));
				}),
				Renderer([&]() {
					return filler();
				}),
				FlatButton(
					TT("[adjust]").str(),
					[]() { NextPage(PfPage::adjust_map); },
					bgcolor(Color::Yellow)
				)
			}),
			pfext::FirstPlayerToggle(isFirst),
		});
		return Renderer(component, [=]() { return component->Render() | border; });
	}
	Element GameInfoStatic(const pfGameInfo &gamerules) {
		return vbox({
			text(TT("You are going to play with ").str() + (player[1] ? player[1]->GetName() : "???")),
			text(TT("Number of planes: ").str() + std::to_string(gamerules.n)),
			text(gamerules.cw ? TT("✓ Cross-border mode enabled").str() : ""),
			text(gamerules.cd ? TT("✓ Completely-destroy enabled").str() : ""),
			text(TT("Map size: ").str() + std::to_string(gamerules.h) + "x" + std::to_string(gamerules.w)),
			text(isFirst ? TT("I goes first") : TT("The other player goes first")),
		});
	}
	Elements splitlines(const std::string &t) {
		Elements res;
		std::stringstream ss(t);
		std::string s;
		while(std::getline(ss, s)) {
			res.push_back(text(s));
		}
		return res;
	}
	Component PfBattleFieldGame() {
		struct BfInteractInfo {
			Box boxBf;
			bool focused;
			struct {
				int x = 0, y = 0;
			} cur;

			// BfInteractInfo() {
			// 	std::clog<<"BfInteractInfo constructed."<<std::endl;
			// }
			// ~BfInteractInfo() {
			// 	std::clog<<"BfInteractInfo destructed."<<std::endl;
			// }
		};
		auto ii = std::make_shared<BfInteractInfo>();
		return CatchEvent(
			Renderer([&, ii](bool focus) {
				const PfBF &bf = player[0]->GetOthersBF();
				ii->focused = focus;
				if(focus) {
					return dbox({
						PfBattleFieldStatic(bf, &ii->boxBf),
						vbox({
							filler() | size(HEIGHT, EQUAL, 2 + ii->cur.y),
							hbox({
								filler() | size(WIDTH, EQUAL, 3 + ii->cur.x * 2),
								text(">") | color(Color::White),
								filler() | size(WIDTH, EQUAL, 2),
								text("<") | color(Color::White)
							})
						})
					});
				}
				return PfBattleFieldStatic(bf, &ii->boxBf);
			}),
			[&, ii](Event e) {
				const PfBF &bf = player[0]->GetOthersBF();
				auto AdvanceRight = [&bf, &ii]() {
					++ii->cur.x %= bf.w;
				};
				auto AdvanceDown = [&bf, &ii]() {
					++ii->cur.y %= bf.h;
				};

				if(e.is_mouse()) {
					if(!ii->boxBf.Contain(e.mouse().x, e.mouse().y)) return false;
					int bx = (e.mouse().x - ii->boxBf.x_min) / 2, by = e.mouse().y - ii->boxBf.y_min;
					if(e.mouse().button == Mouse::Left) {
						if(e.mouse().motion == Mouse::Pressed) {
							if(player[0]->GetGame().isMyTurn() && bf.mk[bx + by * bf.w] == 0) {
								player[0]->Attack(bx, by);
								// std::clog << "[i] in " << __PRETTY_FUNCTION__ << " event handler returns at " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
							}
						}
					} else if(e.mouse().button == Mouse::None) {
						ii->cur.x = bx;
						ii->cur.y = by;
					}
					return true;
				} else if(e.is_character()) {
					auto &ch = bf.ch[ii->cur.x + ii->cur.y * bf.w];
					switch(e.character()[0]) {
					case '-':
						ch = ch == ""   ?                 "─"  :
						     ch == "─"  ? AdvanceRight(), "──" :
							 ch == "──" ? AdvanceRight(), ""   :
							 ch == "│"  ?                 "┤"  :
							 ch == "┤"  ? AdvanceRight(), "┼─" :
							 ch == "┼─" ?                 "├─" :
							 ch == "├─" ? AdvanceRight(), "│"  :
							 "──";
						break;
					case '|':
						ch = ch == ""   ? AdvanceDown(), "│"  :
						     ch == "│"  ? AdvanceDown(), ""   :
						     ch == "──" ?                "┴─" :
						     ch == "┴─" ? AdvanceDown(), "┼─" :
						     ch == "┼─" ?                "┬─" :
						     ch == "┬─" ? AdvanceDown(), "──" :
							 "│";
						break;
					case '+':
						ch = ch == "┼─" ? "" : "┼─";
						break;
					case 'T':
						ch = ch == "┬─" ? "┤"  :
					         ch == "┤"  ? "┴─" :
					         ch == "┴─" ? "├─" :
					         "┬─";
						break;
					case 't':
						ch = ch == "├─" ? "┴─" :
					         ch == "┴─" ? "┤"  :
					         ch == "┤"  ? "┬─" :
					         "├─";
						break;
					case 'h':
						(ii->cur.x += bf.w - 1) %= bf.w;
						break;
					case 'j':
						AdvanceDown();
						break;
					case 'k':
						(ii->cur.y += bf.h - 1) %= bf.h;
						break;
					case 'l':
						AdvanceRight();
						break;
					default:
						return false;
					}
					return true;
				} else {
					if(e == e.ArrowUp) {
						if(ii->cur.y <= 0) {
							ii->cur.y = 0;
							return false;
						}
						--ii->cur.y;
					} else if(e == e.ArrowDown) {
						if(ii->cur.y >= bf.h - 1) {
							ii->cur.y = bf.h - 1;
							return false;
						}
						++ii->cur.y;
					} else if(e == e.ArrowLeft) {
						if(ii->cur.x <= 0) {
							ii->cur.x = 0;
							return false;
						}
						--ii->cur.x;
					} else if(e == e.ArrowRight) {
						if(ii->cur.x >= bf.w - 1) {
							ii->cur.x = bf.w - 1;
							return false;
						}
						++ii->cur.x;
					} else if(e == e.Return) {
						if(player[0]->GetGame().isMyTurn() && bf.mk[ii->cur.x + ii->cur.y * bf.w] == 0) {
							player[0]->Attack(ii->cur.x, ii->cur.y);
						}
					} else if(e == e.Backspace || e == e.Delete) {
						bf.ch[ii->cur.x + ii->cur.y * bf.w] = "";
					} else if(e.input() == "\x06") { // C-f
						AdvanceRight();
					} else if(e.input() == "\x02") { // C-b
						(ii->cur.x += bf.w - 1) %= bf.w;
					} else if(e.input() == "\x0e") { // C-n
						AdvanceDown();
					} else if(e.input() == "\x10") { // C-p
						(ii->cur.y += bf.h - 1) %= bf.h;
					} else if(e.input() == "\x01") { // C-a
						ii->cur.x = 0;
					} else if(e.input() == "\x05") { // C-e
						ii->cur.x = bf.w - 1;
					} else if(e.input() == "\x1b<") { // M-<
						ii->cur.y = 0;
					} else if(e.input() == "\x1b>") { // M->
						ii->cur.y = bf.h - 1;
					} else return false;
					return true;
				}
				return false;
			}
		);
	}
	Component FirstPlayerToggle(bool &state) {
		const static std::vector<std::string> &entries = {
			TT("The other player goes first"),
			TT("I goes first"),
		};
		struct ToggleInfo {
			int selected;
			bool &isFirst;
			ToggleOption opt;
			Component toggle = Toggle(&entries, &selected, &opt);

			ToggleInfo(bool &isFirst): selected(isFirst), isFirst(isFirst) {
				opt.on_change = [&]() {
					isFirst = selected;
				};
			}
		};
		auto ti = std::make_shared<ToggleInfo>(state);
		return Renderer(ti->toggle, [=]() {
			return ti->toggle->Render();
		});
		// the ToggleInfo will be destroyed when the Renderer is destroyed.
	}
};