![pf-2.7-intro.png](https://s2.loli.net/2022/02/20/XNugP8YHGcepD54.png)

English | [简体中文](README.zh-CN.md)

**If you're looking for gamerules introduction, go to the [wiki page](https://github.com/Zjl37/planeFight2/wiki/Game-Introduction).**

## PlaneFight TUI Game

This project "planeFight2" is an effort to implement a game that was played with paper and pen, and is also my very first project. With modern terminals and [box drawing characters](https://unicode-table.com/en/blocks/box-drawing/), TUI is considered the best way this game presents.

PlaneFight TUI Game allows you to play with computer (a simple AI) or another player over local network. You can interact with both keyboard and mouse, thanks to the amazing [FTXUI](https://github.com/ArthurSonzogni/FTXUI) library.

It is cross-platform, and is tested to run on Windows and Linux. (Will somebody test mac for me?) Note that if you use Windows, [Legacy Console](https://go.microsoft.com/fwlink/?LinkId=871150) is not supported, so your Windows version should be newer than Windows 10 Fall Creators Update.

## Build Instruction

You'll need a C++17 compiler, cmake, and the [boost](https://www.boost.org/) library.

Boost locale should be built. If cmake cannot find boost, define environment variable `BOOST_ROOT`.

You don't need to manually download FTXUI, cmake will clone the FTXUI repository from GitHub in cmake configuration process.

It is tested to build on Linux with GCC and Windows with MinGW64 GCC.

## Contribution

This program is **free software** licensed under GNU GPL version 3 or later.

You're welcomed to

- Translate the game and all documents.
	- All English translations were done by myself. Limited by my English proficiency, there may be mistakes or not authentic expressions. Any English translation suggestions is welcomed.

- Make another client or server that follows the same [network protocol](doc/networkProtocol.md).
