![planeFight introduction image](https://i.loli.net/2020/08/29/GsaIE34g5zuV7BX.png)

[English](README.md) | 简体中文

**关于游戏规则的介绍，请前往 [Wiki 页面](https://github.com/Zjl37/planeFight2/wiki/%E6%B8%B8%E6%88%8F%E4%BB%8B%E7%BB%8D)查看。**

## PlaneFight 控制台程序

PlaneFight 是用 C++ 编写控制台游戏的综合实践。这也是我的 github 入门项目。

目前，planeFight 基于 Windows 控制台。高度依赖 **Win32 API** 来实现鼠标交互等功能，通过 **Winsock** 实现了联机游戏。

未来我们打算通过 VT sequence 等技术让它变得跨平台，但这任重道远。

## 开始游戏

1. 去 release 页面下载最新版本。

2. 或者自己编译：

	g++ src/main.cpp src/pfUI.cpp src/pfLang.cpp src/pfAI.cpp -DUNICODE -o planeFight.exe -static -lwsock32

为确保最佳游戏体验， 

- 建议在最新版本的 Windows 10 上运行。本程序将不会支持[旧版控制台](https://go.microsoft.com/fwlink/?LinkId=871150)。

- 一定要在 Windows 默认的控制台窗口（conhost）里运行，不要在 IDE 或编辑器的集成控制台中运行，不然程序收不到鼠标事件。

- 建议在控制台中使用 [制表符](https://unicode-table.com/cn/blocks/box-drawing/)与汉字同宽 的等宽字体。对于简体中文用户来说，你的控制台字体一般就是新宋体，所以无需注意这点。

## 贡献

你也许可以

- 翻译游戏和所有文档。

- 写个 AI 服务器？
