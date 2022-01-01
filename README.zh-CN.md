![planeFight introduction image](https://i.loli.net/2020/08/29/GsaIE34g5zuV7BX.png)

[English](README.md) | 简体中文

**关于游戏规则的介绍，请前往 [Wiki 页面](https://github.com/Zjl37/planeFight2/wiki/%E6%B8%B8%E6%88%8F%E4%BB%8B%E7%BB%8D)查看。**

## PlaneFight Tui 游戏

planeFight2 是用 C++ 编写 tui 游戏（tui 是文本用户界面。与窗口程序相对，这类程序常被称为控制台程序）的综合实践，也是我的 GitHub 入门项目。由于飞机的形状适合用[制表符](https://unicode-table.com/cn/blocks/box-drawing/)表现，tui 加上现代化的终端成为了实现这个游戏的最好方式。

本程序支持人机对战和局域网内的联机功能，支持键盘和鼠标操作。这主要是通过 FTXUI 界面库实现的。跨平台，在 Windows 和 Linux 上都能运行。

特别注意：

- 在 Windows 上不支持也不打算支持[旧版控制台](https://go.microsoft.com/fwlink/?LinkId=871150)，也就是需要较新版本的 Windows 10。

- 在 Windows 上，如果你的 conhost 控制台字体是默认的新宋体等，将会显示错位。这是因为这些字体将某些特殊字符（具体的，是 Unicode 标准中 east asian width 属性为模糊的字符）显示为全宽。推荐使用新的现代化的终端 Windows Terminal 或使用其他字体。

## 构建指南

用 cmake 构建。编译器应当支持 C++17 语言标准。

依赖 [boost](https://www.boost.org/)。Boost 的 locale 模块需要构建。如果 cmake 找不到 boost 在哪，定义 `BOOST_ROOT` 这个环境变量。

Cmake 在配置阶段会把 FTXUI 仓库从 GitHub 上下载下来，不用你再手动下载。

经测试过的构建环境有 Linux 上的 gcc，Windows 上 MinGW64 的 gcc。

## 贡献

本程序是以 GNU GPL 的第 3 版或更新的版本许可的**自由软件**。

你也许可以

- 翻译界面文字和所有文档。

- 按照[通信协议](doc/networkProtocol.md)写个服务器或客户端？
