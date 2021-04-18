![planeFight introduction image](https://i.loli.net/2020/08/29/GsaIE34g5zuV7BX.png)

English | [简体中文](README_zh_Hans.md)

**If you're looking for gamerules introduction, go to the [wiki page](https://github.com/Zjl37/planeFight2/wiki/Game-Introduction).**

## PlaneFight Console Application

PlaneFight is a general practice of C++ console programming, and this repo is also the very first one through which I learn GitHub.

Currently, planeFight Console Game is based on Windows Console. It is heavily depended on **Win32 API** to implement mouse interaction and so. Besides playing against computer, we also supports multiplayer game in local network, which is implemented using **Winsock**.

We are planning to make it cross-platform in the future by adopting technologies like VT sequence, but that is a long way to go.

## How to play

1. Go to release page and download the latest version.

2. Or download the source code and compile it yourself.

To get the best experience, we recommend that you

- run this program on latest version of Windows 10, as the program won't support [Legacy Console](https://go.microsoft.com/fwlink/?LinkId=871150). 

- run this program in default console window (conhost), not in integrated console of some IDE / code editors, otherwise the program won't receive mouse event.

- use a monospace font that shows [box drawing characters](https://unicode-table.com/en/blocks/box-drawing/) full-width.

## Contribution

You're welcomed to

- Translate the game and all documents.
	- All English translations were done by myself. Limited by my English proficiency, there may be grammar mistakes or improper / not idiomatic expressions. Any English translation suggestions is welcomed.

- Maybe make a bot server (with a smart AI)?
