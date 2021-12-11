![planeFight introduction image](https://i.loli.net/2020/08/29/GsaIE34g5zuV7BX.png)

English | [简体中文](README.zh-CN.md)

**If you're looking for gamerules introduction, go to the [wiki page](https://github.com/Zjl37/planeFight2/wiki/Game-Introduction).**

## PlaneFight Tui Game

planeFight2 is a tui (terminal user interface) game and also my very first project. With modern terminals and [box drawing characters](https://unicode-table.com/en/blocks/box-drawing/), tui is considered the best way this game (the shape of the plane) presents.

The program allows you to play with computer (a simple AI) or another player over local network. You can interact with both keyboard and mouse, thanks to the amazing [FTXUI](https://github.com/ArthurSonzogni/FTXUI) library.

It is cross-platform, and is tested to run on Windows and Linux. (Will somebody test mac for me?) Note that for Windows [Legacy Console](https://go.microsoft.com/fwlink/?LinkId=871150) is not supported, so you will need at least Windows 10 as new as 1803.

## Build Instruction

Build with cmake. The C++ compiler should support C++17.

[boost](https://www.boost.org/) is needed. Boost locale should be built first. If cmake cannot find boost, define environment variable `BOOST_ROOT`.

In cmake configuration process, cmake will clone the FTXUI repository from GitHub, so you don't need to manually download it.

It is tested to build on Linux with GCC and Windows with MinGW64 GCC.

## Contribution

You're welcomed to

- Translate the game and all documents.
	- All English translations were done by myself. Limited by my English proficiency, there may be mistakes or not authentic expressions. Any English translation suggestions is welcomed.

- Make another client or server that follows the same [network protocol](doc/networkProtocol.md).
