#include <string>
#include "pfUI.hpp"
#include "pfLang.hpp"
using namespace std;

pfTextElem operator+(const pfTextElem &t1, const pfTextElem &t2) {
	return pfTextElem(t1.s+t2.s,t1.d+t2.d);
}

const int ntext=256;
pfTextElem text[ntext];
pfTextElem text_zh_Hans[ntext] = {
	" 飞机 - 控制台游戏", // 0
	"飞机 - 控制台游戏",
	"欢迎来到 飞机 控制台游戏！",
	"错误：无法读取控制台输入信息。",
	" 简体中文 ",
	"正在加载语言文件，请稍后……", // 5
	"请输入用户名：",
	" 确定(Enter) ",
	"  ※人机对战",
	"  ※远程联机",
	"  ※游戏玩法/关于", // 10
	"<<返回",
	"planeFight ",
	" by Zjl37 ",
	" 本项目即将部署到 Github 上，请访问 https://github.com/Zjl37/planeFight2 以获取游戏玩法和其他信息。",
	"", // 15, reserved
	"",
	"",
	"",
	" 访问 Github 仓库 ",
	"> 发起一场游戏（将本机IP地址告诉你的好友）", // 20
	"> 加入一场游戏（需要知道对方IP地址）",
	" 布置 ",
	" 游戏设置 ",
	" 偏好设置 ",
	"  开始  ", // 25
	" 清除 ",
	"不能在此处放置飞机：飞机不能重叠",
	"不能在此处放置飞机：飞机不能重叠，且不能超出边界",
	"□",
	"√", // 30
	"启用贪吃蛇模式",
	"－",
	"飞机数量 ",
	"＋",
	"←↑请调整窗口大小↓→", // 35
	"战斗已打响！",
	"AI",
	"<<投降",
	" 攻击 ",
	"×", // 40
	" 空 ",
	" 中 ",
	" 击毁 ",
	"   胜利   ",
	" 你失败了 ", // 45
	" 你已投降，游戏结束 ",
	" 对方投降，游戏结束 ",
	"  回到开始页面  ",
	" 标记 ",
	" 擦除 ", // 50
	"当贪吃蛇模式启用时，飞机可以穿越边界放置，溢出部分显示在地图的另一端。",
	"错误：WSAStartup 失败。",
	"[i] WSAStartup 成功。",
	"[i]将本机IP地址告诉你的好友：",
	"[i]在命令行中运行ipconfig，查看本机的 IP 地址，并告诉你的好友。", // 55
	"错误：创建服务器 Socket 失败。",
	"[i]创建服务器 Socket 成功。",
	"错误：Socket 绑定失败。",
	"[i] Socket 绑定成功。",
	"错误：Socket 监听失败。", // 60
	"[i]服务器已进入监听状态。等待客户端连接……",
	"错误：无法接受客户端请求。",
	"[i]客户端已连接。",
	"错误：客户端发送的游戏版本不正确，为 ",
	"<!>警告：客户端发送的游戏版本与当前版本有差异，为 ", // 65
	"错误：创建客户端 Socket 失败。",
	"[i]创建客户端 Socket 成功。请输入服务器 IP 地址：",
	"错误：无法连接到服务器。请确认输入的 IP 地址正确且服务器开启。",
	"[i]已连接到服务器。",
	"错误：当前游戏版本与服务器不兼容。请前往本项目的 Github 仓库下载最新版本。", // 70
	"错误：对方发送了错误的游戏信息。",
	"",
	"",
	"√贪吃蛇模式已启用",
	"飞机数量：", // 75
	"<<放弃",
	"  准备好了  ",
	"你不是房主，改不了游戏设置。",
	"多人游戏的设置已确定，不能更改。",
	"对方放弃了本局游戏。", // 80
	"错误：对方发送了错误的准备信息。",
	"等待对方准备完毕……",
	"启用完全摧毁",
	"√已启用完全摧毁",
	"发送数据时出现错误：连接已断开。", // 85
	"接收数据时出现错误：连接已断开。",
	"我先手",
	"对方先手",
	"○"
};

pfTextElem text_en[ntext] = {
	" PlaneFight - Console Game", // 0
	"PlaneFight - Console Game",
	"Welcome to planeFight Console Game!",
	"Error: Cannot read console input.",
	" English ",
	"Loading language files, please wait……", // 5
	"Enter your username: ",
	" Confirm(Enter) ",
	" ※ Play against computer",
	" ※ Multiplayer game",
	" ※ Gamerules / About", // 10
	"<<Back",
	"planeFight ",
	" by Zjl37 ",
	" This program is open source on Github, Go to https://github.com/Zjl37/planeFight2 for gamerules and more info.",
	"", // 15, reserved
	"",
	"",
	"",
	" Visit Github repo ",
	"> Start a game (by telling your friend your IP address)", // 20
	"> Join a game (given the other players's IP address)",
	" PARK ",
	" GAMERULES ",
	" PREFERENCES ",
	"  PLAY  ", // 25
	" CLEAR ",
	"Cannot place here: planes cannot overlap",
	"Cannot place hare: planes cannot overlap nor be placed out of border",
	"□",
	"√", // 30
	"Enable cross-border mode",
	"－",
	"Number of planes ",
	"＋",
	"←↑ Resize the window to resume the game ↓→", // 35
	"Game starting...",
	"AI",
	"<<Surrender",
	" ATTACK ",
	"×", // 40
	" VOID ",
	" HIT ",
	" DESTROY ",
	" You won! ",
	" You lose. ", // 45
	" You surrendered. ",
	" The other player surrendered. ",
	" Back to main page ",
	" MARK ",
	" ERASE ", // 50
	"With cross-border mode enabled, planes can be placed across the border, the overflow shown on the other side of the map.",
	"Error：WSAStartup failed.",
	"[i] WSAStartup succeeded.",
	"[i] Tell your friend your IP address: ",
	"[i] Run ipconfig in command line to check your IP address and tell your friend.", // 55
	"Error: failed to create server Socket.",
	"[i] Successfully created server Socket.",
	"Error: Socket bind failed.",
	"[i] Socket bind succeeded",
	"Error: Socket listen failed.", // 60
	"[i] Server is now listening and waiting for connections from client...",
	"Error: Cannot accept client.",
	"[i] Client connected.",
	"Error: Wrong game version sent from client, which is ",
	"<!>Warning: There are differences between current game version and that sent from client, which is ", // 65
	"Error: failed to create client Socket.",
	"[i] Successfully created client Socket. Please enter server IP address: ",
	"Error: Cannot connect to server. Please check if the IP is correct and if the server is running.",
	"[i] Connected to server.",
	"Error: Current game version is not compatible with the server. Go to the Github repo of this program for the lateset release.", // 70
	"Error: Bad game message sent from the other player.",
	"",
	"",
	"√ Cross-border mode enabled",
	"Number of planes: ", // 75
	"<<Give up",
	"  I'm ready  ",
	"You're not the host of this game and cannot change the gamerules.",
	"Current gamerule cannot be changed during a multiplayer game.",
	"The other player gave up this game.", // 80
	"Error: Bad ready message sent from the other player.",
	"Waiting the other player to get ready...",
	"Enable completely-destroy",
	"√ Completely-destroy enabled",
	"Failed to send message: connection lost.", // 85
	"Failed to receive message: connection lost.",
	"I am the first",
	"The other player is the first",
	"○"
};

void clear();
extern SMALL_RECT winr;

char buf1[65536];

void pfLangRead(const char Lang[]) {
	GetPrivateProfileStringA("main","inner_title","Lang pack missing",buf1,65536,Lang), text[0]=buf1;
	GetPrivateProfileStringA("main","title","Lang pack missing",buf1,65536,Lang), text[1]=buf1;
	GetPrivateProfileStringA("main","welcome","Lang pack missing",buf1,65536,Lang), text[2]=buf1;
	GetPrivateProfileStringA("main","error","Lang pack missing",buf1,65536,Lang), text[3]=buf1;
	GetPrivateProfileStringA("main","lang_name","Lang pack missing",buf1,65536,Lang), text[4]=buf1;
	GetPrivateProfileStringA("main","loading","Lang pack missing",buf1,65536,Lang), text[5]=buf1;
	GetPrivateProfileStringA("main","input_username","Lang pack missing",buf1,65536,Lang), text[6]=buf1;
	GetPrivateProfileStringA("main","enter","Lang pack missing",buf1,65536,Lang), text[7]=buf1;
	GetPrivateProfileStringA("main","start1","Lang pack missing",buf1,65536,Lang), text[8]=buf1;
	GetPrivateProfileStringA("main","start2","Lang pack missing",buf1,65536,Lang), text[9]=buf1;
	GetPrivateProfileStringA("main","start3","Lang pack missing",buf1,65536,Lang), text[10]=buf1;
	GetPrivateProfileStringA("main","back","Lang pack missing",buf1,65536,Lang), text[11]=buf1;
	GetPrivateProfileStringA("main","start21","Lang pack missing",buf1,65536,Lang), text[20]=buf1;
	GetPrivateProfileStringA("main","start22","Lang pack missing",buf1,65536,Lang), text[21]=buf1;
	GetPrivateProfileStringA("main","resize_win_msg","Lang pack missing",buf1,65536,Lang), text[35]=buf1;
	GetPrivateProfileStringA("about","msg","Lang pack missing",buf1,65536,Lang), text[14]=buf1;
	GetPrivateProfileStringA("about","repo","Lang pack missing",buf1,65536,Lang), text[19]=buf1;
	GetPrivateProfileStringA("game_init","tab0","Lang pack missing",buf1,65536,Lang), text[22]=buf1;
	GetPrivateProfileStringA("game_init","tab1","Lang pack missing",buf1,65536,Lang), text[23]=buf1;
	GetPrivateProfileStringA("game_init","tab2","Lang pack missing",buf1,65536,Lang), text[24]=buf1;
	GetPrivateProfileStringA("game_init","play","Lang pack missing",buf1,65536,Lang), text[25]=buf1;
	GetPrivateProfileStringA("game_init","clear","Lang pack missing",buf1,65536,Lang), text[26]=buf1;
	GetPrivateProfileStringA("game_init","wrong_plane1","Lang pack missing",buf1,65536,Lang), text[27]=buf1;
	GetPrivateProfileStringA("game_init","wrong_plane2","Lang pack missing",buf1,65536,Lang), text[28]=buf1;
	GetPrivateProfileStringA("game_init","button_unselect","Lang pack missing",buf1,65536,Lang), text[29]=buf1;
	GetPrivateProfileStringA("game_init","button_select","Lang pack missing",buf1,65536,Lang), text[30]=buf1;
	GetPrivateProfileStringA("game_init","button_dec","Lang pack missing",buf1,65536,Lang), text[32]=buf1;
	GetPrivateProfileStringA("game_init","button_inc","Lang pack missing",buf1,65536,Lang), text[34]=buf1;
	GetPrivateProfileStringA("game_init","button_switch","Lang pack missing",buf1,65536,Lang), text[89]=buf1;
	GetPrivateProfileStringA("game_init","cross_border_mode","Lang pack missing",buf1,65536,Lang), text[31]=buf1;
	GetPrivateProfileStringA("game_init","plane_num","Lang pack missing",buf1,65536,Lang), text[33]=buf1;
	GetPrivateProfileStringA("game","game_starting","Lang pack missing",buf1,65536,Lang), text[36]=buf1;
	GetPrivateProfileStringA("game","surrender","Lang pack missing",buf1,65536,Lang), text[38]=buf1;
	GetPrivateProfileStringA("game","attack","Lang pack missing",buf1,65536,Lang), text[39]=buf1;
	GetPrivateProfileStringA("game","cursor","Lang pack missing",buf1,65536,Lang), text[40]=buf1;
	GetPrivateProfileStringA("game","void","Lang pack missing",buf1,65536,Lang), text[41]=buf1;
	GetPrivateProfileStringA("game","hit","Lang pack missing",buf1,65536,Lang), text[42]=buf1;
	GetPrivateProfileStringA("game","destroy","Lang pack missing",buf1,65536,Lang), text[43]=buf1;
	GetPrivateProfileStringA("game","victory","Lang pack missing",buf1,65536,Lang), text[44]=buf1;
	GetPrivateProfileStringA("game","lose","Lang pack missing",buf1,65536,Lang), text[45]=buf1;
	GetPrivateProfileStringA("game","lose2","Lang pack missing",buf1,65536,Lang), text[46]=buf1;
	GetPrivateProfileStringA("game","victory2","Lang pack missing",buf1,65536,Lang), text[47]=buf1;
	GetPrivateProfileStringA("game","back_to_main","Lang pack missing",buf1,65536,Lang), text[48]=buf1;
	GetPrivateProfileStringA("game","mark","Lang pack missing",buf1,65536,Lang), text[49]=buf1;
	GetPrivateProfileStringA("game","erase","Lang pack missing",buf1,65536,Lang), text[50]=buf1;
	GetPrivateProfileStringA("game","msg_cross_border_mode","Lang pack missing",buf1,65536,Lang), text[74]=buf1;
	GetPrivateProfileStringA("game","msg_plane_num","Lang pack missing",buf1,65536,Lang), text[75]=buf1;
	GetPrivateProfileStringA("game","give_up","Lang pack missing",buf1,65536,Lang), text[76]=buf1;
	GetPrivateProfileStringA("game","ready","Lang pack missing",buf1,65536,Lang), text[77]=buf1;
	GetPrivateProfileStringA("game","setting_info1","Lang pack missing",buf1,65536,Lang), text[78]=buf1;
	GetPrivateProfileStringA("game","setting_info2","Lang pack missing",buf1,65536,Lang), text[79]=buf1;
	GetPrivateProfileStringA("game","msg_give_up","Lang pack missing",buf1,65536,Lang), text[80]=buf1;
	GetPrivateProfileStringA("game","err_bad_ready","Lang pack missing",buf1,65536,Lang), text[81]=buf1;
	GetPrivateProfileStringA("game","wait_ready","Lang pack missing",buf1,65536,Lang), text[82]=buf1;
	GetPrivateProfileStringA("game","completely_destroy","Lang pack missing",buf1,65536,Lang), text[83]=buf1;
	GetPrivateProfileStringA("game","msg_completely_destroy","Lang pack missing",buf1,65536,Lang), text[84]=buf1;
	GetPrivateProfileStringA("game","connection_lost1","Lang pack missing",buf1,65536,Lang), text[85]=buf1;
	GetPrivateProfileStringA("game","connection_lost2","Lang pack missing",buf1,65536,Lang), text[86]=buf1;
	GetPrivateProfileStringA("game","first1","Lang pack missing",buf1,65536,Lang), text[87]=buf1;
	GetPrivateProfileStringA("game","first2","Lang pack missing",buf1,65536,Lang), text[88]=buf1;
	GetPrivateProfileStringA("socket","err1","Lang pack missing",buf1,65536,Lang), text[52]=buf1;
	GetPrivateProfileStringA("socket","msg1","Lang pack missing",buf1,65536,Lang), text[53]=buf1;
	GetPrivateProfileStringA("socket","msg2","Lang pack missing",buf1,65536,Lang), text[54]=buf1;
	GetPrivateProfileStringA("socket","msg3","Lang pack missing",buf1,65536,Lang), text[55]=buf1;
	GetPrivateProfileStringA("socket","err4","Lang pack missing",buf1,65536,Lang), text[56]=buf1;
	GetPrivateProfileStringA("socket","msg4","Lang pack missing",buf1,65536,Lang), text[57]=buf1;
	GetPrivateProfileStringA("socket","err5","Lang pack missing",buf1,65536,Lang), text[58]=buf1;
	GetPrivateProfileStringA("socket","msg5","Lang pack missing",buf1,65536,Lang), text[59]=buf1;
	GetPrivateProfileStringA("socket","err6","Lang pack missing",buf1,65536,Lang), text[60]=buf1;
	GetPrivateProfileStringA("socket","msg6","Lang pack missing",buf1,65536,Lang), text[61]=buf1;
	GetPrivateProfileStringA("socket","err7","Lang pack missing",buf1,65536,Lang), text[62]=buf1;
	GetPrivateProfileStringA("socket","msg7","Lang pack missing",buf1,65536,Lang), text[63]=buf1;
	GetPrivateProfileStringA("socket","bad_ver1","Lang pack missing",buf1,65536,Lang), text[64]=buf1;
	GetPrivateProfileStringA("socket","bad_ver2","Lang pack missing",buf1,65536,Lang), text[65]=buf1;
	GetPrivateProfileStringA("socket","err8","Lang pack missing",buf1,65536,Lang), text[66]=buf1;
	GetPrivateProfileStringA("socket","msg8","Lang pack missing",buf1,65536,Lang), text[67]=buf1;
	GetPrivateProfileStringA("socket","err9","Lang pack missing",buf1,65536,Lang), text[68]=buf1;
	GetPrivateProfileStringA("socket","msg9","Lang pack missing",buf1,65536,Lang), text[69]=buf1;
	GetPrivateProfileStringA("socket","err10","Lang pack missing",buf1,65536,Lang), text[70]=buf1;
	GetPrivateProfileStringA("socket","bad_msg","Lang pack missing",buf1,65536,Lang), text[71]=buf1;
	// for(int i=0;i<ntext;i++) if(text[i].s.size())
	// 	text[i].s.pop_back();
	text[12]="planeFight ";
	text[13]=" by Zjl37 ";
	text[37]="AI";

	clear();
	pfLangInit(winr.Right); 
}

void pfLangDetect() {
	LANGID lid = GetSystemDefaultLangID();
	if((lid&0xff)==0x04) pfLangRead("lang/zh-Hans.txt");
	else pfLangRead("lang/en.txt");
}

void pfLangInit(int winw) {
	gotoXY(2,2);
	cout<<text[5].s<<endl;
	for(int i=0; i<ntext; i++) {
		bool isAscii=true;
		for(unsigned j=0; j<text[i].s.length(); j++)
			if(text[i].s[j]&128) {
				isAscii=false; break;
			}
		if(isAscii) {
			text[i].d=0; continue;
		}
		gotoXY(0,4);
		cout<<text[i].s;
		text[i].d=text[i].s.length()-(getY()-4)*winw-getX();
	}
	SetConsoleTitleA(text[1].s.c_str());
}