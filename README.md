![planeFight introduction image](https://i.loli.net/2020/08/29/GsaIE34g5zuV7BX.png)

English | [简体中文](README_zh_Hans.md)

## PlaneFight Introduction

PlaneFight is a two-player turn-based mini-game that you can play using only paper and pen. The game deals with certain pixel pattern called "plane" and its objective is to determine the position of all the other player's planes and destroy them.

### Basic gamerules

A typical game has these rules:

Each player begins with a 10x10 gird called "battle field", invisible to the other, on which three planes is to be placed. A plane is a 4x5 pixel pattern which can rotate in four directions, as shown below.

![what is a plane image](https://i.loli.net/2020/08/29/WdzxaIkBpTghqSn.png)

In the above picture, blue girds are called **portion of a plane**, while white ones are not. Especially, the gird marked by red X are called **head of a plane**. Planes cannot overlap, nor be placed out of border. The following picture shows a valid arrangement.

![valid bf example](https://i.loli.net/2020/08/29/YMQgi6EomTcNJdx.png)

Game starts when both players are ready. In each turn, one player sends a "targeted missile" to the other's battle field by saying a coordinate. The other player must reply by saying one of these words: "VOID", "HIT", or "DESTROY", denoting that the target point is _not a portion of a plane_ / _a portion of a plane but not head_ / _head of a plane_, as the return message of that missile. The two players take action alternately.

A plane is destroyed if its head is hit. A player who first destory all three planes of the other wins the game. Of course players should keep a record of all return messages so that they can accurately determine the position of the other player's plane.

### Optional gamerules

- Number of planes

	Suggest 2~5 in 10x10 battle field.

- Cross-border mode

	With this option enabled, planes can be placed across the border, the overflow shown on the other side of the field.   
	An example:

	![valid bf cw example](https://i.loli.net/2020/08/29/RSfxZ1MNETDyFva.png)

- Battle field size

- Completely-destroy

	With this option disabled, missiles targeted at a portion of a destroyed plane returns "HIT", otherwise, "VOID".

### Game history

The game was originally invented by students of _Wenzhou Experimental Middle School, Zhejiang_, and is honored as one of "the three cultural treasures of game of WEMS".

Although I'm not a student of that school, I'm glad to witness its spread throughout other high schools of the city as graduates from WEMS went to different high schools. Thus I decided to contribute my force and find a small corner for it in the wide open source world. That's the birth of PlaneFight Console Game.

## PlaneFight Console Application

PlaneFight Console Game is an implementation of the above game based on Windows Console, written in C++. It **read**s **console input** so you can just interact with mouse click. It offers 2 game experience: "play against computer" and "multiplayer game". **Winsock** is used in the for communication functionality between computers in local network.

This repo is also the very first one through which I learn github.

## How to play

1. Go to release page and download the latest version.

2. Or download the source code and compile it yourself.

### Tips

To get the best game experience, 

- Be sure to run this program in default console window (conhost), not in integrated console of some IDE / code editors, otherwise the program won't receive mouse event.

- Suggest using a monospace font that shows [box drawing characters](https://unicode-table.com/en/blocks/box-drawing/) full-width.

- Suggest running on latest version of Windows 10, as the program doesn't support [Legacy Console](https://go.microsoft.com/fwlink/?LinkId=871150). 

## Contribution

You're welcomed to

- Translate the game and all documents.
	- All English translations were done by myself. Limited by my English proficiency, there may be grammar mistakes or improper / not idiomatic expressions. Any English translation suggestions is welcomed.

- Maybe make a bot server (with a smart AI)?

## Plan

See [plan](plan.md).