#include "pfUI.hpp"
#include "pfGame.hpp"
#include "pfUiCtrl.hpp"
#include "pfLocale.hpp"
#include "pfExtFtxui.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "pfConsole.hpp"
#include <future>
#include <string>
#include <thread>

using namespace std;

extern std::string mapEdge[256];
extern std::mt19937 rng;

// legacy UI 1

void DrawPlane(int x, int y, int d) {
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}
void DrawPlane(int x, int y, int d, int bx, int by, int bw, int bh) {
	for(const auto &ps: plShape[d])
		if(x + ps.dx * 2 >= bx + bw * 2 || x + ps.dx * 2 < bx || y + ps.dy >= by + bh || y + ps.dy < by)
			return;
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}
void DrawPlaneCw(int x, int y, int d, int bx, int by, int bw, int bh) {
	for(const auto &ps: plShape[d]) {
		int tx = x + ps.dx * 2, ty = y + ps.dy;
		if(tx < bx) tx += bw * 2;
		if(tx >= bx + bw * 2) tx -= bw * 2;
		if(ty < by) ty += bh * 2;
		if(ty >= by + bh) ty -= bh;
		gotoXY(tx, ty);
		cout << ps.ch;
	}
}

///////////////////////////////////////////////////////////////////////////////

extern bool isFirst;

namespace pfui {

	ftxui::ScreenInteractive scr = ftxui::ScreenInteractive::Fullscreen();
	ftxui::Component ui;

	int p2IsNetworkGame;
	bool p2ShowBanner;

	std::string AttackIndicatorInfo::MakeCoordStr() const {
		using namespace std::string_literals;
		return (signDir ? ">>>>  "s : "<<<<  "s)
		       + "(" + std::to_string(x) + "," + std::to_string(y) + ")"
			   + (signDir ? "  >>>>" : "  <<<<");
	}

	AttackIndicatorInfo p5AttackInfo;

	void Build() {
		using namespace ftxui;

		auto p0InputOpt = InputOption();
		p0InputOpt.on_enter = ctrl::P0InputOK;

		auto p0BtnRow = Container::Horizontal({
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 2); }),
			pfext::FlatButton(TT(" Confirm ").str(), ctrl::P0InputOK, bgcolor(Color::Yellow)),
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 2); }),
			pfext::FlatButton(TT(" Exit ").str(), scr.ExitLoopClosure(), bgcolor(Color::Red)),
		});

		static auto pfTitle = text(TT(" PlaneFight - Console Game")) | bgcolor(Color::Blue);

		/* clang-format off */

		auto btnBackLn = []() {
			return Container::Horizontal({
				pfext::FlatButton(TT("<<Back").str(), PrevPage, bgcolor(Color::Yellow)),
				Renderer([]() { return filler(); })
			});
		};

		auto p0Fg = Container::Vertical({
			Renderer([]() {
				return vbox({
					pfTitle | clear_under,
					filler() | size(HEIGHT, EQUAL, 3),
					text(TT("Welcome to planeFight Console Game!")) | center | size(HEIGHT, GREATER_THAN, 3)
						| bgcolor(Color::Purple) | clear_under,
					filler() | size(HEIGHT, EQUAL, 1),
				});
			}),
			Container::Horizontal({
				Renderer([]() {
					return hbox({filler() | size(WIDTH, EQUAL, 2), text(TT("Enter your username: "))});
				}),
				Input(&playername, "", p0InputOpt),
				Renderer([]() {
					return filler() | size(WIDTH, EQUAL, 2);
				}),
			}),
			Renderer([]() { return filler() | size(HEIGHT, EQUAL, 1); }),
			Renderer(p0BtnRow, [=]() {
				return p0BtnRow->Render() | clear_under;
			}),
			// TODO: language lists here
		});

		static bool p1ShowRgMenu = false;

		auto p1Fg = Container::Vertical({
			Renderer([]() {
				return vbox({
					pfTitle | clear_under,
					filler() | size(HEIGHT, EQUAL, 1),
				});
			}),
			Button(TT(" ※ Play against computer").str(), ctrl::P1PlayLocal),
			Button(TT(" ※ Multiplayer game").str(), []() { p1ShowRgMenu = !p1ShowRgMenu; }),
			Maybe(Container::Horizontal({
				Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
				Container::Vertical({
					pfext::FlatButton(TT("> Start a server").str(), []() {
						isFirst = rng() & 1;
						NextPage(PfPage::gamerule_setting_server);
					}),
					pfext::FlatButton(TT("> Join a game").str(), []() { NextPage(PfPage::client_init); }),
				})
			}), &p1ShowRgMenu),
			Button(TT(" ※ Gamerules / About").str(), []() { NextPage(PfPage::about); }),
			Container::Horizontal({
				Button(TT("  Exit  ").str(), scr.ExitLoopClosure()),
				Button(TT("  Back  ").str(), PrevPage)
			})
		});

		auto p2BfRow = Container::Horizontal({
			Container::Vertical({
				Renderer([]() {
					return text(player[0] ? player[0]->GetName() : "???");
				}),
				pfext::PfBattleFieldPrepare(bf1, curGame, ctrl::p2SelectedFacing),
			}),
			Renderer([]() { return filler(); }),
			Container::Tab({
				/* local game */
				pfext::GameInfoInteractive(curGame, bf1),
				/* network game */
				Renderer([]() { return pfext::GameInfoStatic(curGame); })
			}, &p2IsNetworkGame),
		});

		auto p2Banner = Renderer([]() {
			return text(TT("Game starting...")) | center | size(HEIGHT, EQUAL, 3) | bgcolor(Color::Magenta) | clear_under | vcenter;
		});

		auto p2BtnReady = Button(TT("  I'm ready  ").str(), ctrl::P2Ready);
		auto p2BtnClear = Button(TT(" CLEAR ").str(), ctrl::P2Clear);

		static Box p3Box;

		auto p5BfRow = Container::Horizontal({
			Renderer([]() {
				return vbox({
					text(player[0] ? player[0]->GetName() : "???"),
					pfext::PfBattleFieldStatic(player[0]->GetMyBF())
				});
			}),
			Renderer([]() { return filler(); }),
			Container::Vertical({
				Renderer([]() {
					return text(player[1] ? player[1]->GetName() : "???");
				}),
				pfext::PfBattleFieldGame(),
			})
		});

		auto p5AttackIndicator = Renderer([]() {
			return p5AttackInfo.state != AttackIndicatorInfo::none ?
				vbox({
					text(p5AttackInfo.MakeCoordStr()) | (
						p5AttackInfo.state == AttackIndicatorInfo::blink1 ?
						color(Color::Grey70) :
						p5AttackInfo.state == AttackIndicatorInfo::blink2 ?
						color(Color::Black) :
						p5AttackInfo.state == AttackIndicatorInfo::blink3 ?
						color(Color::Grey30) :
						color(Color::White)
					),
					text(""),
					p5AttackInfo.state == AttackIndicatorInfo::resulted ?
						(p5AttackInfo.res == PfAtkRes::destroy ?
							text(TT(" DESTROY ")) | bgcolor(Color::DarkRed) :
							p5AttackInfo.res == PfAtkRes::hit ?
							text(TT(" HIT ")) | bgcolor(Color::Red) :
							text(TT(" VOID ")) | bgcolor(Color::Green)) | hcenter :
						text(""),
				}) | center :
				filler();
		});

		auto p6BfRow = Container::Horizontal({
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
			Renderer([]() {
				return vbox({
					text(player[0] ? player[0]->GetName() : "???"),
					pfext::PfBattleFieldStatic(player[0]->GetMyBF())
				});
			}),
			Renderer([]() { return filler(); }),
			Renderer([]() {
				return vbox({
					text(player[1] ? player[1]->GetName() : "???"),
					pfext::PfBattleFieldStatic(player[0]->GetOthersBF())
				});
			}),
			Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
		});

		#ifdef _WIN32
		static std::string p8CheckIpMsg = TT("Run ipconfig in command line to check your IP address and tell your friend.");
		#else
		static std::string p8CheckIpMsg = TT("Run ifconfig in command line to check your IP address and tell your friend.");
		#endif

		auto p9InputOpt = InputOption();
		p9InputOpt.on_enter = ctrl::P9InputOK;

		ui = Container::Tab({
			/* page 0 welcome */
			Renderer(p0Fg, [=]() {
				return dbox({
					pfext::BackgroundWithScatteredPlane(),
					p0Fg->Render()
				});
			}),

			/* page 1 main */
			Renderer(p1Fg, [=]() {
				return dbox({
					pfext::BackgroundWithScatteredPlane(),
					p1Fg->Render()
				});
			}),

			/* page 2 prepare */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				Container::Horizontal({
					Container::Tab({
						pfext::FlatButton(TT("<<Back").str(), PrevPage, bgcolor(Color::Yellow)),
						pfext::FlatButton(TT("<<Give up").str(), []() {
							player[0]->Giveup();
							PrevPage();
						}, bgcolor(Color::Yellow)),
					}, &p2IsNetworkGame),
					Renderer([]() { return filler(); })
				}),
				Container::Horizontal({
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
					Container::Vertical({
						Renderer(p2BfRow, [=]() {
							return p2ShowBanner ? dbox({
								p2BfRow->Render(),
								p2Banner->Render()
							}) : p2BfRow->Render();
						}),
						Renderer([]() { return filler(); }),
						Container::Horizontal({
							pfext::Park(ctrl::p2SelectedFacing),
							Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
							Container::Vertical({
								Renderer(p2BtnClear, [=]() {
									return player[0]->GetGame().state & PfGame::me_ready ? emptyElement() : p2BtnClear->Render();
								}),
								Renderer(p2BtnReady, [=]() {
									return bf1.nPlaced == curGame.n ? p2BtnReady->Render() : emptyElement();
								})
							})
						})
					}),
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
				}),
			}),

			/* page 3 adjust map */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				Container::Horizontal({
					pfext::FlatButton(TT("<<Back").str(), []() {
						if(bf1.w != curGame.w || bf1.h != curGame.h) {
							bf1.resize(curGame.w, curGame.h);
						}
						PrevPage();
					}, bgcolor(Color::Yellow)),
					Renderer([]() { return filler(); })
				}),
				Container::Horizontal({
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
					CatchEvent(
						Renderer([]() {
							return vbox({
								text(TT("Map size: ").str() + to_string(curGame.h) + "x" + to_string(curGame.w)),
								hbox(
									filler() | size(WIDTH, EQUAL, 1),
									filler() | size(WIDTH, EQUAL, curGame.w*2) | size(HEIGHT, EQUAL, curGame.h) | reflect(p3Box)
								) | borderDouble
							});
						}),
						[](Event e) {
							if(e.is_mouse() && e.mouse().motion == Mouse::Pressed && e.mouse().button == Mouse::Left) {
								if(e.mouse().x >= p3Box.x_min && e.mouse().y >= p3Box.y_min) {
									int bx = (e.mouse().x - p3Box.x_min) / 2 + 1, by = e.mouse().y - p3Box.y_min + 1;
									if(bx >= 5 && by >= 5) {
										curGame.w = bx, curGame.h = by;
									}
									return true;
								}
							}
							return false;
						}
					)
				})
			}),

			/* page 4 about */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				btnBackLn(),
				Renderer([]() {
					// ftxui doesn't support multi-paragraph text with word wrap currently.
					// use a fixed width of 80 characters instead.
					return vbox(
						pfext::splitlines(TT(
							"    For an introduction to the gamerules, please visit\n"
							"<https://github.com/Zjl37/planeFight2/wiki/Game-Introduction>."
						).str()),
						text(""),
						pfext::splitlines(TT(
							"    The source code of this program is hosted on GitHub. For more information,\n"
							"see <https://github.com/Zjl37/planeFight2>."
						).str())
					) | size(WIDTH, LESS_THAN, 80) | hcenter;
				}),
			}),

			/* page 5 game */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				Container::Horizontal({
					pfext::FlatButton(TT("<<Surrender").str(), ctrl::P5Surrender, bgcolor(Color::Yellow)),
					Renderer([]() { return filler(); })
				}),
				Container::Horizontal({
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
					Renderer(p5BfRow, [=]() {
						return dbox({
							p5BfRow->Render(),
							p5AttackIndicator->Render()
						}) | flex_grow;
					}),
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
				}),
			}),

			/* page 6 game over */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				btnBackLn(),
				Renderer(p6BfRow, [&, p6BfRow]() {
					return dbox({
						p6BfRow->Render(),
						border({
							(player[0]->GetGame().state & PfGame::other_surrender                       ?
								text(TT(" The other player surrendered. ")) | bgcolor(Color::DarkGreen) :
								player[0]->GetGame().state & PfGame::me_surrender                       ?
								text(TT(" You surrendered. ")) | bgcolor(Color::DarkRed)                :
								player[0]->GetGame().nDestroyedMine == curGame.n                        ?
								text(TT(" You lose. ")) | bgcolor(Color::Red)                           :
								player[0]->GetGame().nDestroyedOthers == curGame.n                      ?
								text(TT(" You won! ")) | bgcolor(Color::Green)                          :
								text(TT(" Gameover. ")) | bgcolor(Color::Blue))
						}) | center
					});
				})
			}),

			/* page 7 gamerule setting (server) */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				btnBackLn(),
				Container::Horizontal({
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
					Container::Vertical({
						Renderer([]() { return text(TT("Please set gamerules before starting a server.")); }),
						Renderer([]() { return text(""); }),
						Container::Horizontal({
							pfext::FlatButton(TT("－").str(), [&]() {
								curGame.n = std::max(curGame.n - 1, 1);
							}, bgcolor(Color::Yellow)),
							Renderer([&]() {
								return text("  "s + TT("Number of planes: ").str() + std::to_string(curGame.n) + "  ");
							}),
							pfext::FlatButton(TT("＋").str(), [&]() {
								++curGame.n;
							}, bgcolor(Color::Yellow)),
						}),
						Renderer([]() { return text(""); }),
						Checkbox(TT("Enable cross-border mode").str(), &curGame.cw),
						Renderer([]() { return text(""); }),
						Checkbox(TT("Enable completely destroy").str(), &curGame.cd),
						Renderer([]() { return text(""); }),
						Container::Horizontal({
							Renderer([&]() {
								return text(TT("Map size: ").str() + std::to_string(curGame.h) + "x" + std::to_string(curGame.w));
							}),
							Renderer([&]() {
								return filler();
							}),
							pfext::FlatButton(
								TT("[adjust]").str(),
								[]() { NextPage(PfPage::adjust_map); },
								bgcolor(Color::Yellow)
							)
						}),
						Renderer([]() { return text(""); }),
						pfext::FirstPlayerToggle(isFirst),
						Renderer([]() { return text(""); }),
						Container::Horizontal({
							Button(TT("  START  ").str(), ctrl::P7StartServer),
						})
					}),
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
				})
			}),

			/* page 8 server init */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				Container::Horizontal({
					pfext::FlatButton(TT("<<Back").str(), ctrl::P8ServerStop, bgcolor(Color::Yellow)),
					Renderer([]() { return filler(); })
				}),
				Renderer([]() {
					return hbox({
						filler() | size(WIDTH, EQUAL, 4),
						vbox({
							border(hflow(paragraph(p8CheckIpMsg)) | vcenter) | size(HEIGHT, EQUAL, 7),
							vbox({
								text(TT("Waiting for a player to join in..."))
							})
						}) | flex_grow,
						filler() | size(WIDTH, EQUAL, 4),
					});
				})
			}),

			/* page 9 client init */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				Container::Horizontal({
					pfext::FlatButton(TT("<<Back").str(), ctrl::P9ClientStop, bgcolor(Color::Yellow)),
					Renderer([]() { return filler(); })
				}),
				Container::Horizontal({
					Renderer([]() { return filler() | size(WIDTH, EQUAL, 4); }),
					Container::Vertical({
						Renderer([]() { return text(TT("Server IP address:")); }),
						Input(&ctrl::ipAddr, "", p9InputOpt)
					}),
				})
			}),

			/* page 10 error */
			Container::Vertical({
				Renderer([]() { return pfTitle; }),
				btnBackLn(),
				Renderer([]() {
					return hbox({
						filler() | size(WIDTH, EQUAL, 4),
						border(hflow(paragraph(ctrl::errMsg)) | vcenter) | flex_grow | bgcolor(Color::DarkRed),
						filler() | size(WIDTH, EQUAL, 4),
					});
				})
			})
		}, &pageNum);
		/* clang-format on */
	}

	void Loop() {
		scr.Loop(ui);
	}
}

///////////////////////////////////////////////////////////////////////////////

std::stack<PfPage> stPage;

void showErrorMsg(const std::string &t, PfPage rpage) {
	pfui::ctrl::errMsg = t;
	while(stPage.size() > 1) stPage.pop();
	stPage.push(rpage);
	stPage.push(PfPage::error);
	pfui::pageNum = int(stPage.top());
}
void showErrorMsg(const std::string &t) {
	pfui::ctrl::errMsg = t;
	stPage.push(PfPage::error);
	pfui::pageNum = int(stPage.top());
}

void SetPage(PfPage x) {
	if(stPage.size()) stPage.pop();
	stPage.push(x);
	pfui::pageNum = int(stPage.top());
}

void NextPage(PfPage x) {
	stPage.push(x);
	pfui::pageNum = int(stPage.top());
}

void PrevPage() {
	if(!stPage.empty()) {
		stPage.pop();
	}
	if(stPage.empty()) {
		clog << "[i] in " << __PRETTY_FUNCTION__ << " stPage is empty, defaulting to PfPage::main." << endl;
		NextPage(PfPage::main);
	}
	pfui::pageNum = int(stPage.top());
}

///////////////////////////////////////////////////////////////////////////////

// legacy UI 2

void UiGameStart() {
	static std::future<void> fut;
	if(fut.valid()) fut.wait();
	fut = std::async(launch::async, []() {
		pfui::p2ShowBanner = true;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(1s);

		pfui::p2ShowBanner = false;
		SetPage(PfPage::game);
		pfui::scr.PostEvent(ftxui::Event::Custom);
	});
}

void UiGameover() {
	SetPage(PfPage::gameover);
	pfui::scr.PostEvent(ftxui::Event::Custom);
}

std::promise<PfAtkRes> promP5Attack;

void UiShowAtkRes(PfAtkRes res) {
	promP5Attack.set_value(res);
}

void BlinkCoord(short ax, short ay, bool signDir) {
	pfui::p5AttackInfo.x = ax, pfui::p5AttackInfo.y = ay;
	pfui::p5AttackInfo.signDir = signDir;

	static std::future<void> fut;
	if(fut.valid()) fut.wait();

	promP5Attack = std::promise<PfAtkRes>();

	fut = std::async(launch::async, []() {
		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::blink1;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(chrono::milliseconds(rng() % 250));

		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::blink2;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(chrono::milliseconds(rng() % 250));

		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::blink3;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(chrono::milliseconds(rng() % 250));

		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::blink4;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(chrono::milliseconds(500 + rng() % 250));

		pfui::p5AttackInfo.res = promP5Attack.get_future().get();
		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::resulted;
		pfui::scr.PostEvent(ftxui::Event::Custom);
		this_thread::sleep_for(1s);
		
		pfui::p5AttackInfo.state = pfui::AttackIndicatorInfo::none;
		pfui::scr.PostEvent(ftxui::Event::Custom);
	});
}
