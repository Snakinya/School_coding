#include <stdio.h>
#include <time.h> 
#include<conio.h>
#include<windows.h>
#include<mmsystem.h>
#include<stdlib.h>


#pragma comment(lib,"winmm.lib")
#define UP    1
#define DOWN  2
#define LEFT  3
#define RIGHT 4
#define MAX_LEVEL 3
#define BULLET_NUM 6
#define MAX_LIFE 3

typedef struct                //这里的出现次序指的是一个AI_stick变量中的次序，游戏共有四个AI_stick变量
{
    int fast_stick_order;   //fast_stick出现的次序(在第fast_stick_order次复活出现,从第0次开始)，且每个AI_stick只出现一次
    int firm_stick_order;   //firm_stick出现的次序，同上
} LevInfo;                 //关卡信息(准确说是该关出现的火柴人信息)
LevInfo level_info [MAX_LEVEL] = {{-1,-1},{1,1},{2,3}};  //初始化，-1代表没有该类型火柴人

typedef struct      //子弹结构体
{
    int x,y;        //子弹坐标
    int direction;  //子弹方向变量
    bool exist;     //子弹存在变量
    bool initial;   //子弹是否处于建立初状态的变量
    bool my;        //区分AI子弹与玩家子弹的标记,0为AI子弹，1为玩家子弹
} Bullet;
Bullet bullet [BULLET_NUM];

typedef struct      //火柴人结构体
{
    int x,y;        //火柴人中心坐标
    int direction;  //火柴人方向
    int color;      //颜色参数
    int model;      //火柴人图案模型，值为1,2,3，分别代表不同的火柴人图案,0为我的火柴人图案，AI不能使用
    int stop;       //只能是AI火柴人使用的参数，非0代表火柴人停止走动,0为可以走动
    int revive;     //火柴人复活次数
    int num;        //AI火柴人编号0~3
    int CD;         //发射子弹冷却计时
    bool my;        //是否敌方火柴人参数，我的火柴人此参数为1,为常量
    bool alive;     //存活为1，不存活为0
}  Stick;
Stick AI_stick[4] ,my_stick1 ,my_stick2;  //my_stick为我的火柴人，AI_stick 代表AI火柴人

//基本操作与游戏辅助函数
void Loading();            //装载界面 
char ShowMainMenu();       //开始界面 
void GoToxy(int x,int y);  //光标移动
void HideCursor();  //隐藏光标
void keyboard();  //接收键盘输入
void Getmap();  //地图数据存放与获取
void Initialize();  //初始化(含有对多个数据的读写)
void Stop();  //暂停
void Frame ();  //打印游戏主体框架
void PrintMap();  //打印地图
void SideScreen ();  //副屏幕打印
void GameCheak();  //检测游戏输赢
void GameOver( bool home );  //游戏结束
void ClearMainScreen();  //主屏幕清屏函数
void ColorChoose(int color);  //颜色选择函数
void NextLevel();  //下一关(含有对level全局变量的读写)

//子弹部分
void BuildAIBullet(Stick *stick);  //AI火柴人发射子弹
void BuildBullet(Stick stick);  //子弹发射（建立）（人机共用）
void BulletFly(Bullet bullet[BULLET_NUM]);  //子弹移动和打击（人机共用）
void BulletHit(Bullet* bullet);  //子弹碰撞（人机共用）
void PrintBullet(int x,int y,int T);  //打印子弹（人机共用）
void ClearBullet(int x,int y,int T);  //清除子弹（人机共用）
int  BulletCheak(int x,int y);  //判断子弹前方情况（人机共用）
 
//火柴人部分
void BuildAIStick (int* position, Stick* AI_stick);  //建立AI火柴人
void BuildMyStick1 (Stick* my_stick1);  //建立我的火柴人
void BuildMyStick2 (Stick* my_stick2);
void MoveAIStick  (Stick* AI_stick);  //AI火柴人移动
void MoveMyStick1  (int turn);  //我的火柴人移动
void MoveMyStick2  (int turn);
void ClearStick   (int x,int y);  //清除火柴人（人机共用）
void PrintStick   (Stick stick);  //打印火柴人（人机共用）
bool StickCheak   (Stick stick,int direction);  //检测火柴人dirtection方向的障碍物,返值1阻碍,0 畅通
int  AIPositionCheak (int position);  //检测AI火柴人建立位置是否有障碍物AIPositionCheak
//存档功能
FILE * mem;     //游戏存档文件句柄
char dir = UP;  //因为在save函数里要用到，把原来定义在主函数里的dir移出来，并初始化为向上
void save(void);    //游戏存档函数
void load(void);	//读档
//egg
int high,width; // 游戏画面大小
int bird_x,bird_y; // 小鸟的坐标
int bar1_y,bar1_xDown,bar1_xTop; // 障碍物1的相关坐标
int score1; // 得分，经过障碍物的个数 
//全局变量
int map[41][41];  //地图二维数组
int bul_num;  //子弹编号
int position;  //位置计数,对应AI火柴人生成位置,-1为左位置,0为中间,1为右,2为我的火柴人位置
int speed=7;  //游戏速度,调整用
int level=1;  //游戏关卡数
int score=0;  //游戏分数
int remain_enemy;  //剩余敌人
 
char* stick_figure[4][3][4]=
{
  {
    {"◢┃◣", "◢━◣", "◢┳◣", "◢┳◣"},
    {"┣●┫", "┣●┫", "━●┃", "┃●━"},
    {"◥━◤", "◥┃◤", "◥┻◤", "◥┻◤"}
  },
  {
    {"┏┃┓", "┏┳┓", "┏┳┓", "┏┳┓"},
    {"┣●┫", "┣●┫", "━●┫", "┣●━"},
    {"┗┻┛", "┗┃┛", "┗┻┛", "┗┻┛"}
  },
  {
    {"┏┃┓", "◢━◣", "┏┳◣", "◢┳┓"},
    {"┣●┫", "┣●┫", "━●┃", "┃●━"},
    {"◥━◤", "┗┃┛", "┗┻◤", "◥┻┛"}
  },
  {
    {"╔┃╗", "╔╦╗", "╔╦╗", "╔╦╗"},
    {"╠●╣", "╠●╣", "━●╣", "╠●━"},
    {"╚╩╝", "╚┃╝", "╚╩╝", "╚╩╝"}
  }
};



void GoToxy(int x,int y)  //光标移动函数，X表示横坐标，Y表示纵坐标。
{
    COORD  coord;         //使用头文件自带的坐标结构
    coord.X=x;            //这里将int类型值传给short,不过程序中涉及的坐标值均不会超过short范围
    coord.Y=y;
    HANDLE a=GetStdHandle(STD_OUTPUT_HANDLE);  //获得标准输出句柄
    SetConsoleCursorPosition(a,coord);         //以标准输出的句柄为参数设置控制台光标坐标
}

void HideCursor()  //隐藏光标
{
    CONSOLE_CURSOR_INFO cursor_info={1,0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cursor_info);
}

void keyboard ()
{
    int count=0;
    if (GetAsyncKeyState(VK_UP)& 0x8000)
        MoveMyStick1( UP );
    if (GetAsyncKeyState(VK_DOWN)& 0x8000)
        MoveMyStick1( DOWN );
    if (GetAsyncKeyState(VK_LEFT)& 0x8000)
        MoveMyStick1( LEFT );
    if (GetAsyncKeyState(VK_RIGHT)& 0x8000)
        MoveMyStick1( RIGHT );
    if (GetAsyncKeyState(0x57)& 0x8000)
        MoveMyStick2( UP );
    if (GetAsyncKeyState(0x53)& 0x8000)
        MoveMyStick2( DOWN );
    if (GetAsyncKeyState(0x41)& 0x8000)
        MoveMyStick2( LEFT );
    if (GetAsyncKeyState(0x44)& 0x8000)
        MoveMyStick2( RIGHT );
    else if (GetAsyncKeyState( 0x1B )& 0x8000){
    	 save();   //存档函数 
    	 system("cls");
		 printf("\n\n\n\n\t\t\t\tARCHIVING。。。") ;
		 Sleep(1000); 
         exit(0);  //退出程序函数
	}     
    else if (GetAsyncKeyState( 0x20 )& 0x8000)
        Stop();
    else if (count++%7==0)
    {
        if (speed>1 && GetAsyncKeyState( 0x6B )& 0x8000)   // +键
        {
            speed--;
            GoToxy(102,11);           //在副屏幕打印出当前速度
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_RED);
            printf("%d ",21-speed);
        }
        else if (speed<20 && GetAsyncKeyState( 0x6D )& 0x8000)  // - 键
        {
            speed++;
            GoToxy(102,11);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_RED);
            printf("%d ",21-speed);
        }
    }
    if(my_stick1.CD==7)
    {
        if(GetAsyncKeyState( 0x4A )& 0x8000)
        {
            BuildBullet(my_stick1);
            my_stick1.CD=0;
        }
    }
    else
        my_stick1.CD++;
    if(my_stick2.CD==7)
    {
        if(GetAsyncKeyState( 0x46 )& 0x8000)
        {
            BuildBullet(my_stick2);
            my_stick2.CD=0;
        }
    }
    else
        my_stick2.CD++;
}

void GetMap()      //地图存放函数
{                //map里的值: 个位数的值为地图方块部分，百位数的值为火柴人
    int i ,j;      //map里的值: 0通路，1黄砖，5水，6铁砖，100为敌方火柴人，200为我的火柴人，
    int Map[3][41][41]=
    {
        {
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,6,6,6,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,6,6,6,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,6,6,6,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,4},
            {4,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,4},
            {4,6,6,6,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,6,6,6,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,6,6,6,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,6,6,6,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,6,6,6,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}
        },
        {
			{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,4},
			{4,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,4},
			{4,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,1,1,1,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,1,1,1,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,4},
			{4,5,5,5,5,5,5,0,0,0,5,5,5,5,5,5,0,0,0,5,5,5,1,1,1,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,0,0,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,4},
			{4,0,0,0,5,5,5,5,5,5,5,5,5,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,5,5,5,0,0,0,5,5,5,5,5,5,4},
			{4,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,0,0,0,5,5,5,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
			{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}
		},
        {
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,6,6,6,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,1,1,1,0,0,0,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,6,6,6,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,5,5,5,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,5,5,5,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,5,5,5,5,5,5,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,6,6,6,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,6,6,6,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,6,6,6,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,1,1,1,5,5,5,5,5,5,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,6,6,6,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,6,6,6,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,6,6,6,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,5,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
            {4,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,1,1,1,6,6,6,0,0,0,4},
            {4,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,1,1,1,6,6,6,0,0,0,4},
            {4,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,9,9,9,1,1,0,0,0,0,0,0,0,1,1,1,6,6,6,0,0,0,4},
            {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}
        },
    };
        for(i=0;i<41;i++)
            for(j=0;j<41;j++)
                    map[i][j]=Map[level-1][i][j];
    PrintMap();         //打印地图
}

void Initialize()  //初始化
{
    remain_enemy=16;
    my_stick1.revive=0;  //我的火柴人复活次数为0
    my_stick2.revive=0;
    position=0;
    bul_num=0;
    GetMap();
    BuildMyStick1( &my_stick1 );
    BuildMyStick2( &my_stick2 );
    for(int i=0;i<12;i++)     //子弹初始化
    {
        bullet [i].exist=0;
        bullet [i].initial=0;
    }
    for(int i=0;i<=3;i++)         //AI火柴人初始化
    {
        AI_stick [i].revive=0;
        AI_stick [i].alive=0;  //初始化火柴人全是不存活的，BuildAIStick()会建立重新建立不存活的火柴人
        AI_stick [i].stop=0;
        AI_stick [i].num=i;
        AI_stick [i].my=0;
        AI_stick [i].CD=0;
    }
    GoToxy(97,2);                        //在副屏幕上关卡数
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN);
    printf("%d",level);
    GoToxy(102,5);                       //在副屏幕上打印分数
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("%d   ",score);
    GoToxy(102,7);                       //在副屏幕打印剩余生命值
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("%d", MAX_LIFE-my_stick1.revive);
    GoToxy(102,9);                       //在副屏幕上打印剩余火柴人数
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
    printf("%d ",remain_enemy);
    GoToxy(100,13);                      //在副屏幕上打印状态
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_GREEN);
    printf("Playing");
}

void Stop()    //暂停
{
    int color=1,timing=0;
    while(1)
    {
        if(timing++%30==0)
        {
            ColorChoose(color);   //颜色选择
            GoToxy(100,13);       //副屏幕打印
            printf("Game Stop");
            GoToxy(88,17);
            printf("Press enter to return");
            GoToxy(88,18);
            printf("or press ESC to exit");
            if(++color==8)
                color=1;
        }
        if (GetAsyncKeyState( 0xD )& 0x8000)      //回车键
        {
            GoToxy(100,13);       //副屏幕打印
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
            printf("  Playing");   //覆盖掉原来的提示
            GoToxy(88,17);
            printf("                     ");
            GoToxy(88,18);
            printf("                     ");
            break;
        }
        else if(GetAsyncKeyState( 0x1B )& 0x8000) //Esc键退出
            exit(0);
        Sleep(20);
    }
}

void Frame ()     //打印游戏主体框架
{                 //SetConsoleTextAttribute为设置文本颜色和文本背景颜色函数
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("  ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁  ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
    printf("  ▂▂▂▂▂▂▂▂▂▂▂▂▂ \n");
    for(int i=0;i<14;i++)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
        printf("▕                                                                              ▏");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
        printf(" |                          |\n");
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("▕                                                                              ▏");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
    printf(" |═════════════════════════|\n");
    for(int i=0;i<24;i++)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
        printf("▕                                                                              ▏");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
        printf(" |                          |\n");
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("  ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔  ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY| FOREGROUND_BLUE);
    printf(" ﹊﹊﹊﹊﹊﹊﹊﹊﹊﹊﹊﹊﹊﹊\n");
    SideScreen ();  //打印副屏幕
}

void PrintMap()     // 打印地图(地图既地图障碍物)
{
    for(int j=0;j<41;j++)
        for(int i=0;i<41;i++)
            if(map[i][j]==6)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN
                    |FOREGROUND_RED|FOREGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE);
                GoToxy(2*j,i);
                printf("■");
            }
            else if(map[i][j]==1)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|BACKGROUND_GREEN|BACKGROUND_RED);
                GoToxy(2*j,i);
                printf("▓");
            }
            else if(map[i][j]==5)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|BACKGROUND_BLUE|FOREGROUND_BLUE|FOREGROUND_GREEN);
                GoToxy(2*j,i);
                printf("～");
            }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);
    GoToxy(38,37);     printf("★█★");
    GoToxy(38,38);     printf("█★█");
    GoToxy(38,39);     printf("◣█◢");
}

void SideScreen ()
{                   // |         第  d  关         |   " |                          |\n"
    GoToxy(92,2);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    printf("The     Level");
    GoToxy(92,5);
    //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("SCORE：");
    GoToxy(92,7);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("LIFE：");
    GoToxy(86,9);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
    printf("Remaining enemy：");
    GoToxy(86,11);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("SPEED：  %d",21-speed);
    GoToxy(86,13);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
    printf("Game status：");
    GoToxy(94,19);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    GoToxy(94,24);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    printf("HELP");
    GoToxy(88,21);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("WASD ↑←↓→ Move");
    GoToxy(91,23);
    printf("F J  Shoot");
    GoToxy(89,25);
    printf("+ - Adjust speed");
    GoToxy(90,27);
    printf("Game speed 1~20");
    GoToxy(90,29);
    printf("ENTER Stop game");
    GoToxy(90,31);
    printf("Esc  Exit gamwe");
    GoToxy(86,33);
    printf("Kill all enemies to pass");
    GoToxy(87,34);
    printf("Your stick's life is 0 ");
    GoToxy(86,35);
    printf("Or the base be destroyed ");
    GoToxy(84,36);
    printf("        Game over!");
    GoToxy(85,37);

}

void GameCheak()
{
    if(remain_enemy<=0 && !AI_stick[0].alive && !AI_stick[1].alive && !AI_stick[2].alive && !AI_stick[3].alive )
        NextLevel();
    if(my_stick1.revive>=MAX_LIFE)
        GameOver(0);
}

void GameOver(bool home)
{
    int timing=0,color=1;
    while(1)
    {
        if(timing++%30==0)        //游戏结束原因为生命值为0
        {
            ColorChoose(color);    //颜色选择
            if(home)
            {
                GoToxy(37,19);     //主屏幕中心打印
                printf("G    G！");
            }
            GoToxy(37,20);         //主屏幕中心打印
            printf("Game over!");
            GoToxy(100,13);        //副屏幕打印
            printf("Game over!");
            GoToxy(88,17);
            printf("Press enter to return!");
            GoToxy(88,18);
            printf("Or press Esc to exit!");
            if(++color==8)
                color=1;
        }
        if (GetAsyncKeyState( 0xD )& 0x8000)  //回车键
        {
            score-=500;  //分数-500
            ClearMainScreen();  //主屏清屏函数，无需再次打印框架
            Initialize();  //从本关重新开始
            break;
        }
        else if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc键退出
            exit(0);
        Sleep(20);
    }
}

void ClearMainScreen()
{
    for(int i=1;i<40;i++)
    {
        GoToxy(2,i);
        printf("                                                                              ");
    }
}

void ColorChoose(int color)   //颜色选择函数
{
    switch(color)
    {
        case 1:               //蓝色(己火柴人颜色1)
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
            break;
        case 2:               //绿色
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
            break;
        case 3:               //黄色
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN);
            break;
        case 4:               //红色
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
            break;
        case 5:               //紫色(己火柴人眼色2)
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
            break;
        case 6:               //白色
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
            break;
        case 7:               //深蓝色
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE);
            break;
    }
}

void NextLevel()
{
    int timing=0,color=1;
    level++;
    if(level<=MAX_LEVEL)
        while(1)
        {
            if(timing++%10==0)
            {
                ColorChoose(color);   //颜色选择
                GoToxy(37,20);        //主屏幕中心打印
                printf("Congratulations!");
                GoToxy(87,17);
                printf("Press enter to next!");
                GoToxy(88,18);
                printf("Or press exit to exit!");
                if(++color==8)
                    color=1;
            }
            if (GetAsyncKeyState( 0xD )& 0x8000)  //回车键
            {
                GoToxy(88,17);        //抹除副屏幕中的提示
                printf("                     ");
                GoToxy(88,18);
                printf("                     ");
                ClearMainScreen();   //主屏清屏函数，无需再次打印框架
                Initialize();       //初始化从下一关开始,level已++
                break;
            }
            else if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc键退出
                exit(0);
            Sleep(20);
        }
    else   //level>3 通关
        while(1)
        {
            if(timing++%5==0)
            {
                ColorChoose(color);
                GoToxy(33,20);        //主屏幕中心打印
                printf("Pass all the levels!");
                GoToxy(100,13);       //副屏幕打印
                printf("Pass all");
                GoToxy(88,17);
                printf("Congratulations!!");
                GoToxy(88,19);
                printf("Press ESC to exit!");
                if(++color==8)
                    color=1;
            }
            if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc键退出
                exit(0);
            Sleep(10);
        }
}


void BuildAIBullet(Stick *stick)   //AI子弹发射含有对my_stick的读取
{
    if(stick->CD==15)
    {
        if(!(rand()%11))     //冷却结束后在随后的每个游戏周期中有10分之一的可能发射子弹
        {
            BuildBullet(*stick);
            stick->CD=0;
        }
    }
    else
        stick->CD++;
    if(stick->CD >= 14)
    {
        if(stick->y==38 )
        {
            if(stick->x < 20) //在基地左边
            {
                if(stick->direction==RIGHT)  //火柴人方向朝左
                {
                    BuildBullet(*stick);     //发射子弹
                    stick->CD=0;
                }
            }
            else             //在基地右边
                if(stick->direction==LEFT)   //火柴人方向朝右
                {
                    BuildBullet(*stick);     //发射子弹
                    stick->CD=0;
                }
        }
        else if(stick->x==my_stick1.x+1 || stick->x==my_stick1.x || stick->x==my_stick1.x-1)  //AI火柴人在纵向上"炮口"对准我的火柴人
        {
            if(stick->direction==DOWN && my_stick1.y > stick->y || stick->direction==UP && my_stick1.y < stick->y)
            {  //若是AI朝下并且我的火柴人在AI火柴人下方或者AI朝上我的火柴人在AI上方
                int big=my_stick1.y , small=stick->y , i;
                if(my_stick1.y < stick->y)
                {
                    big=stick->y;
                    small=my_stick1.y;
                }
                for(i=small+2;i<=big-2;i++)  //判断AI炮口的直线上两火柴人间有无障碍
                    if(map[i][stick->x]!=0 || map[i][stick->x]!=5)  //若有障碍
                        break;
                if(i==big-1)  //若i走到big-1说明无障碍
                {
                    BuildBullet(*stick);  //则发射子弹
                    stick->CD=0;
                }
            }
        }
        else if(stick->y==my_stick1.y+1 || stick->y==my_stick1.y || stick->y==my_stick1.y-1)  //AI火柴人在横向上"炮口"对准我的火柴人
        {
            if(stick->direction==RIGHT && my_stick1.x > stick->x || stick->direction==LEFT && my_stick1.x < stick->x)
            {  //若是AI朝右并且我的火柴人在AI火柴人右方(数值大的在下面)或者AI朝左我的火柴人在AI左方
                int big=my_stick1.y , smal=stick->y , i;
                if(my_stick1.x < stick->x)
                {
                    big=stick->x;
                    smal=my_stick1.x;
                }
                for(i=smal+2;i<=big-2;i++)
                    if(map[stick->y][i]!=0 || map[stick->y][i]!=5)
                        break;
                if(i==big-1)
                {
                    BuildBullet(*stick);
                    stick->CD=0;
                }
            }
        }
        else if(stick->x==my_stick2.x+1 || stick->x==my_stick2.x || stick->x==my_stick2.x-1)
        {
            if(stick->direction==DOWN && my_stick2.y > stick->y || stick->direction==UP && my_stick2.y < stick->y)
            {
                int big=my_stick2.y , small=stick->y , i;
                if(my_stick2.y < stick->y)
                {
                    big=stick->y;
                    small=my_stick2.y;
                }
                for(i=small+2;i<=big-2;i++)
                    if(map[i][stick->x]!=0 || map[i][stick->x]!=5)
                        break;
                if(i==big-1)
                {
                    BuildBullet(*stick);
                    stick->CD=0;
                }
            }
        }
        else if(stick->y==my_stick2.y+1 || stick->y==my_stick2.y || stick->y==my_stick2.y-1)
        {
            if(stick->direction==RIGHT && my_stick2.x > stick->x || stick->direction==LEFT && my_stick2.x < stick->x)
            {
                int big=my_stick2.y , smal=stick->y , i;
                if(my_stick2.x < stick->x)
                {
                    big=stick->x;
                    smal=my_stick2.x;
                }
                for(i=smal+2;i<=big-2;i++)
                    if(map[stick->y][i]!=0 || map[stick->y][i]!=5)
                        break;
                if(i==big-1)
                {
                    BuildBullet(*stick);
                    stick->CD=0;
                }
            }
        }
    }
}

void BuildBullet(Stick stick)
{
    switch(stick.direction)
    {
        case UP :
                bullet [bul_num].x = stick.x;
                bullet [bul_num].y = stick.y-2;
                bullet [bul_num].direction=1;
                break;
        case DOWN :
                bullet [bul_num].x = stick.x;
                bullet [bul_num].y = stick.y+2;
                bullet [bul_num].direction=2;
                break;
        case LEFT :
                bullet [bul_num].x = stick.x-2;
                bullet [bul_num].y = stick.y;
                bullet [bul_num].direction=3;
                break;
        case RIGHT :
                bullet [bul_num].x = stick.x+2;
                bullet [bul_num].y = stick.y;
                bullet [bul_num].direction=4;
                break;
    }
    bullet [bul_num].exist = 1;  //子弹被建立,此值为1则此子弹存在
    bullet [bul_num].initial = 1;  //子弹处于初建立状态
    bullet [bul_num].my=stick.my;  //如果是我的火柴人发射的子弹bullet.my=1，否则为0
    bul_num++;
    if(bul_num==BULLET_NUM)
        bul_num=0;
}

void BulletFly(Bullet bullet[BULLET_NUM]) //子弹移动和打击
{
    for(int i =0; i<BULLET_NUM;i++)
    {
        if(bullet [i].exist)  //如果子弹存在
        {
            if(bullet [i].initial==0)  //如果子弹不是初建立的
            {
                if(map[bullet[i].y] [bullet[i].x]==0 || map[bullet[i].y] [bullet[i].x]==5)  //如果子弹坐标当前位置无障碍
                    ClearBullet( bullet[i].x , bullet[i].y , BulletCheak(bullet[i].x , bullet[i].y ));  //抹除子弹图形
                switch(bullet [i].direction)  //然后子弹坐标变化（子弹变到下一个坐标）
                {
                    case UP    :(bullet [i].y)--;break;
                    case DOWN  :(bullet [i].y)++;break;
                    case LEFT  :(bullet [i].x)--;break;
                    case RIGHT :(bullet [i].x)++;break;
                }
            }
            int collide = BulletCheak ( bullet [i].x , bullet [i].y );  //判断子弹当前位置情况,判断子弹是否碰撞
            if( collide )   //如果检测到当前子弹坐标无障碍
                PrintBullet( bullet[i].x , bullet[i].y , collide);       //则打印子弹，若有碰撞则不打印
            else
                BulletHit( & bullet [i] );  //若有碰撞则执行子弹碰撞函数
            if(bullet [i].initial)  //若子弹初建立，则把初建立标记去除
                bullet [i].initial = 0;
            for(int j=0; j< BULLET_NUM ; j++)  //子弹间的碰撞判断,若是我方子弹和敌方子弹碰撞则都删除,若为两敌方子弹则无视
                if(bullet [j].exist && j!=i && (bullet[i].my || bullet[j].my) && bullet[i].x==bullet[j].x && bullet[i].y==bullet[j].y)
                {  //同样的两颗我方子弹不可能产生碰撞
                    bullet [j].exist=0;
                    bullet [i].exist=0;
                    ClearBullet( bullet[j].x , bullet[j].y , BulletCheak(bullet[j].x , bullet[j].y ));  //抹除j子弹图形,子弹i图形已被抹除
                    break;
                }
        }
    }
}

void BulletHit(Bullet* bullet)
{
    int x=bullet->x;
    int y=bullet->y;
    int i;
    if(map[y][x]==1 || map[y][x]==2)  //子弹碰到砖块
    {
        if(bullet->direction==UP || bullet->direction==DOWN)   //如果子弹是纵向的
            for(i = -1 ; i<=1 ; i++)
                if(map[y][x+i]==1 || map[y][x+i]==2)  //如果子弹打中砖块两旁为砖块,则删除砖,若不是(一旁为火柴人或其他地形)则忽略
                {
                    map[y][x+i]=0;  //砖块碎
                     GoToxy(2*x+2*i,y);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED); //背景黑色
                     printf("  ");
                }
        if(bullet->direction==LEFT || bullet->direction==RIGHT)     //若子弹是横向的
            for(i = -1 ; i<=1 ; i++)
                if(map[y+i][x]==1 || map[y+i][x]==2)
                {
                    map[y+i][x]=0;
                     GoToxy(2*x,y+i);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED); //背景黑色
                     printf("  ");
                }
        bullet->exist=0;  //这颗子弹消失
    }
    else if(map[y][x]==4 || map[y][x]==6 )  //子弹碰到边框或者不可摧毁方块
        bullet->exist=0;
    else if(bullet->my && map[y][x]>=100 && map[y][x]<104 )  //若我的子弹碰到了敌方火柴人
    {
        int num = map[y][x]%100;   //读取该火柴人信息
        if(AI_stick[num].model==3 && AI_stick[num].color==2)   //若为firm stick,且color==2。该火柴人为绿色,表明没有受到伤害
                AI_stick[num].color=3;                        //则变成黄色，color=3为黄色
        else if (AI_stick[num].model==3 && AI_stick[num].color==3)
                AI_stick[num].color=4;                        //4为红色
        else  //其他类型的火柴人或者firm stick为红色的情况
        {
            AI_stick[num].alive=0;
            ClearStick(AI_stick[num].x , AI_stick[num].y);      //清除该火柴人
        }
        bullet->exist=0;
        score+=100;
        GoToxy(102,5);  //在副屏幕上打印出分数
        //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
        printf("%d ",score);
    }
    else if(map[y][x]==200 && bullet->my==0 )  //若敌方子弹击中我的火柴人
    {
        my_stick1.alive=0;
        my_stick2.alive=0;
        ClearStick(my_stick1.x , my_stick1.y);
        ClearStick(my_stick2.x , my_stick2.y);
        bullet->exist=0;
        my_stick1.revive++;  //我的火柴人复活次数+1
        my_stick2.revive++;
        score-=100;  //分数减少
        GoToxy(102,5);  //在副屏幕上打印出分数
        //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
        printf("%d   ",score);
        GoToxy(102,7);  //在副屏幕打印出我的剩余生命值
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
        printf("%d   ", MAX_LIFE - my_stick1.revive);
    }
    else if(map[y][x]==9)  //子弹碰到基地(无论是谁的子弹)
    {
        bullet->exist=0;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);
        GoToxy(38,37);     printf("      ");
        GoToxy(38,38);     printf("◢◣◢ ");
        GoToxy(38,39);     printf("███");
        GameOver(1);           //游戏结束,传入1代表基地被毁
    }
}

void PrintBullet (int x,int y,int T)   //当前坐标BulletCheak 的值做参量 T
{
    if(T==1)          //  T==1 表示子弹当前坐标在陆地上
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
    GoToxy(2*x,y);
    printf("☉");
}

void ClearBullet(int x,int y,int T)   //当前坐标BulletCheak 的值做参量 T
{
    GoToxy(2*x,y);
    if(T==1)   //  T==1 表示子弹当前坐标在陆地上
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE);
        printf("  ");
    }
}

int BulletCheak (int x,int y)  //判断子弹当前位置情况,判断子弹是否碰撞
{
    if(map[y][x]==0)
        return 1;
    else
        return 0;
}


//position为火柴人生成位置,-1为左位置,0为中间,1为右,2为我的火柴人位置
void BuildAIStick(int* position, Stick* AI_stick)   //执行一次该函数只建立一个火柴人
{
       if(AIPositionCheak(*position))        //若此位置无障碍,可生成。
    {
        AI_stick->x= 20 + 18*(*position);  //20 + 18 * position 对应三个生成位置的x假坐标
        AI_stick->y=2;
        if(AI_stick->revive==level_info[level-1].firm_stick_order)
        {
            AI_stick->model = 3;           //3为firm stick的模型(外观)
            AI_stick->color = 2;           //颜色参数2为绿色，具体详见函数ColorChoose
        }
        else if(AI_stick->revive==level_info[level-1].fast_stick_order)  //同上if，这里是fast_stick的
        {
            AI_stick->model = 2;
            AI_stick->color = rand()%6+1;
        }
        else      //普通火柴人
        {
            AI_stick->model = 1;
               AI_stick->color = rand()%6+1;  //若不是firm stick则随机颜色
        }
        AI_stick->alive = 1;       //火柴人变为存在
        AI_stick->direction = 2 ;  //方向朝下
        AI_stick->revive++;        //复活次数+1
        PrintStick(*AI_stick);
        (*position)++;
        remain_enemy--;
        GoToxy(102,9);            //在副屏幕上打印剩余火柴人数
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
        printf("%d ",remain_enemy);
        if(*position==2)
            *position = -1;
             return ;
    }
}

void BuildMyStick1 (Stick* my_stick1) //建立我的火柴人
{
    my_stick1->x=25;
    my_stick1->y=38;
    my_stick1->stop=NULL;
    my_stick1->direction=1;
    my_stick1->model=0;
    my_stick1->color=1;
    my_stick1->alive=1;
    my_stick1->my=1;
    my_stick1->CD=7;
    PrintStick (*my_stick1);  //打印我的火柴人
}

void BuildMyStick2 (Stick* my_stick2)
{
    my_stick2->x=15;
    my_stick2->y=38;
    my_stick2->stop=NULL;
    my_stick2->direction=1;
    my_stick2->model=0;
    my_stick2->color=5;
    my_stick2->alive=1;
    my_stick2->my=1;
    my_stick2->CD=7;
    PrintStick (*my_stick2);
}

void MoveAIStick(Stick* AI_stick) //AI专用函数
{
       if(AI_stick->alive)         //如果火柴人活着
    {
        if(AI_stick->stop!=0)   //火柴人是否停止运动的判断，若stop参数不为0
        {
            AI_stick->stop--;   //则此火柴人本回合停止运动
            return;
        }
        if( !(rand()%23) )     //22分之1的概率执行方向重置
        {
            AI_stick->direction = rand()%4+1;
            if( rand()%3 )     //在方向重置后有2分之1的概率停止走动3步的时间
            {
                AI_stick->stop=2;
                return;
            }
        }
        ClearStick (AI_stick->x , AI_stick->y);
        if(StickCheak ( *AI_stick , AI_stick->direction))   //如果前方无障碍
            switch ( AI_stick->direction )
            {
                case UP   : AI_stick->y--; break;  //上前进一格
                case DOWN : AI_stick->y++; break;  //下前进一格
                case LEFT : AI_stick->x--; break;  //左前进一格
                case RIGHT: AI_stick->x++; break;  //右前进一格
            }
        else                     //前方有障碍
        {
            if(!(rand()%4))      //3分之1的概率乱转
            {
                AI_stick->direction=rand()%4+1;
                AI_stick->stop=2; //乱转之后停止走动3步的时间
                PrintStick(*AI_stick);
                return;
            }
            else                 //另外3分之2的几率选择正确的方向
            {
                int j;
                for(j=1;j<=4;j++)
                    if(StickCheak ( *AI_stick , j ))  //循环判断火柴人四周有无障碍,此函数返值1为可通过
                        break;
                if(j==5)         //j==5说明此火柴人四周都有障碍物，无法通行
                {
                    PrintStick(*AI_stick);
                    return;
                }
                while(StickCheak ( *AI_stick , AI_stick->direction) == 0)  //如果前方仍有障碍
                    AI_stick->direction=(rand()%4+1);                    //则换个随机方向检测
            }
        }
        PrintStick(*AI_stick);
    }
}

void MoveMyStick1(int turn)   //玩家专用函数，turn为keyboard函数里因输入不同方向键而传入的不同的值
{
    ClearStick(my_stick1.x , my_stick1.y);
    my_stick1.direction=turn;  //将键盘输入的方向值传入我的火柴人方向值
    if(StickCheak ( my_stick1 , my_stick1.direction ))
        switch (turn)
        {
            case UP : my_stick1.y--; break;  //上前进一格
            case DOWN : my_stick1.y++; break;  //下前进一格
            case LEFT : my_stick1.x--; break;  //左前进一格
            case RIGHT : my_stick1.x++; break;  //右前进一格
    }
    PrintStick (my_stick1);
}

void MoveMyStick2(int turn)
{
    ClearStick(my_stick2.x ,my_stick2.y);
    my_stick2.direction=turn;
    if(StickCheak ( my_stick2 , my_stick2.direction ))
        switch (turn)
        {
            case UP : my_stick2.y--; break;
            case DOWN : my_stick2.y++; break;
            case LEFT : my_stick2.x--; break;
            case RIGHT : my_stick2.x++; break;
    }
    PrintStick (my_stick2);
}

void ClearStick(int x,int y)   //清除火柴人函数
{
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
        {
             map[y+j-1][x+i-1]=0;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN);
            GoToxy(2*x+2*j-2,y+i-1);
            printf("  ");
        }
}

void PrintStick(Stick stick)  //打印火柴人
{
    ColorChoose(stick.color);  //颜色选择函数
    char *(*stickF)[4] = stick_figure[stick.model];
    for(int i = 0; i < 3; i++)
    {
        GoToxy((stick.x-1)*2 , stick.y-1+i);
        printf("%s", stickF[i][stick.direction-1]);
         for(int j=0;j<3;j++)
            if(stick.my)  //若为我的火柴人
                map[stick.y+j-1][stick.x+i-1]=200;  //在map上把"火柴人"九格填满代表敌我火柴人的参数。敌方此值为100~103,我方为200
            else
                map[stick.y+j-1][stick.x+i-1]=100+stick.num;
    }
}


bool StickCheak(Stick stick,int direction)  //检测火柴人前方障碍函数,参量为假坐标。返值1为可通过,返值0为阻挡(人机共用)
{
    switch(direction)
    {
        case UP:
            if (map[stick.y-2][stick.x]==0 && map[stick.y-2][stick.x-1]==0 && map[stick.y-2][stick.x+1]==0)
                return 1;
            else
                return 0;
        case DOWN:
            if (map[stick.y+2][stick.x]==0 && map[stick.y+2][stick.x-1]==0 && map[stick.y+2][stick.x+1]==0)
                return 1;
            else
                return 0;
        case LEFT:
            if (map[stick.y][stick.x-2]==0 && map[stick.y-1][stick.x-2]==0 && map[stick.y+1][stick.x-2]==0)
                return 1;
            else
                return 0;
        case RIGHT:
            if (map[stick.y][stick.x+2]==0 && map[stick.y-1][stick.x+2]==0 && map[stick.y+1][stick.x+2]==0)
                return 1;
            else
                return 0;
        default:
            printf("错误！！");
            Sleep(5000);
            return 0;
    }
}

int AIPositionCheak( int position )    //position为火柴人生成位置2为我的火柴人位置，其余为AI位，-1为左位，0为中间位置，1为右位
{
    int    x,y;
    if(position==2)                    //2为我的火柴人位置，现在暂时用不到
        x=15,y=38;
    else
        y=2 , x= 20 + 18 * position ;  //20 + 18 * position 对应三个生成位置的x假坐标
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            if( map[y+j-1][x+i-1]!=0)  //如果遍历的九宫格里有障碍物
            {
                return 0;              //则返回0，表示此生成位置有阻碍
            }
    return 1;                          //否则生成1，表示此生成位置无阻碍
}

char ShowMainMenu()
{
	char inputChar;

	//清屏 
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN
		|FOREGROUND_RED|FOREGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE);
	
	printf("\n\n\n");
	//显示菜单 
	printf("\t\t\t■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_GREEN);
	printf("\n\t\t\t  _________  __  .__        __      __      __               ");
	Sleep(150);
	printf("\n\t\t\t /   _____/ /  |_|__| ____ |  | __ /  \\    /  \\_____ _______ "); 
	Sleep(150);
	printf("\n\t\t\t\\_____  \\  \\   __\\  |/ ___\\|  |/ / \\   \\/\\/   /\\__  \\_    __\\"); 
	Sleep(150);
	printf("\n\t\t\t /       \\  |  | |  \\ \\___ |    <   \\        /  / __ \\|  | \\/"); 
	Sleep(150);
	printf("\n\t\t\t/_______ /  |__| |__|\\___  >__|_ \\   \\__/\\  /  (____  /__|   "); 
	Sleep(150);
	printf("\n\t\t\t       \\/                \\/     \\/        \\/        \\/       \n"); 
	Sleep(150);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN
		|FOREGROUND_RED|FOREGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE);
	printf("\t\t\t■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n\n\n");
	Sleep(150);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_GREEN);
	printf("\t\t\t1)New Game\n\n");
	Sleep(150);
	printf("\t\t\t2)Continue\n\n");
	
	//获取输入 
	inputChar = _getch();
	return inputChar;
}

void Loading(){
	int i;
    unsigned int interval[12]={1,1,1,1,1,1,1,1,1,1,1,1} ;  //速度控制
    srand(time(NULL)); //设置随机数种子
    	system("cls");
	system("mode con cols=112 lines=42"); //控制窗口大小
    Frame ();  //打印主体框架
    Initialize();
    for(;;)
    {
        if(interval[0]++%speed==0)
        {
            GameCheak();
            BulletFly ( bullet );
            for(i=0 ; i<=3 ; i++)
            {
                if(AI_stick[i].model==2 && interval[i+1]++%2==0)  //四个火柴人中的快速火柴人单独使用计数器1,2,3,4
                    MoveAIStick( & AI_stick[i]);
                if(AI_stick[i].model!=2 && interval[i+5]++%3==0)  //四个火柴人中的慢速火柴人单独使用计数器5,6,7,8
                    MoveAIStick( & AI_stick[i]);
            }
            for(i=0;i<=3;i++)  //建立AI火柴人部分
                if(AI_stick[i].alive==0 && AI_stick[i].revive<4 && interval[9]++%90==0)  //一个敌方火柴人每局只有4条命
                {
                    BuildAIStick( &position, & AI_stick[i] );  //建立AI火柴人（复活）
                    break;  //每次循环只建立一个火柴人
                }
            for(i=0;i<=3;i++)
                if(AI_stick[i].alive)
                    BuildAIBullet(&AI_stick[i]);
            if((my_stick1.alive || my_stick1.alive) && interval[10]++%2==0 )
                 keyboard ();
            if(my_stick1.alive==0 && interval[11]++%30==0 && my_stick1.revive < MAX_LIFE)
                BuildMyStick1( &my_stick1 );
            if(my_stick2.alive==0 && interval[11]++%30==0 && my_stick2.revive < MAX_LIFE)
                BuildMyStick2( &my_stick2 );
        }
        Sleep(5);
    }
    
} 

void save(void){

    //打开存档文件
    if((mem = fopen("D:\\stick_war\\memory.txt","w+")) == NULL){
        fprintf(stderr,"No cache！\n");
        exit(1);
    }
	//保存关卡数 
	fprintf(mem, "%d",level);
    fclose(mem);
} 

void load(void){
    //读取游戏存档
    if((mem = fopen("D:\\stick_war\\memory.txt","r")) == NULL){
        fprintf(stderr,"Fail to load!\n");
        exit(2);
    }
    //读取关卡数 
    fread(&level,sizeof(level),1,mem);
    
    fclose(mem);
}

//EGG 
void gotoxy(int x,int y) //光标移动到(x,y)位置
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle,pos);
}

void startup()  // 数据初始化
{
	high = 20;
	width = 20;
	bird_x = high/2;
	bird_y = 3;
	bar1_y = width;
	bar1_xDown = high/3;
	bar1_xTop = high/2;
	score1 = 0;
}

void show()  // 显示画面
{
	gotoxy(0,0);  // 光标移动到原点位置，以下重画清屏	
	int i,j;
	
	for (i=0;i<high;i++)
	{
		for (j=0;j<width;j++)
		{
			if ((i==bird_x) && (j==bird_y))
				printf("@");  //   输出小鸟
			else if ((j==bar1_y) && ((i<bar1_xDown)||(i>bar1_xTop)) )
				printf("*");  //   输出墙壁
			else
				printf(" ");  //   输出空格
		}
		printf("\n");
	}
	printf("得分：%d\n",score1);
}	

void updateWithoutInput()  // 与用户输入无关的更新
{
	bird_x ++;
	bar1_y --;
	if (bird_y==bar1_y)
	{
		if ((bird_x>=bar1_xDown)&&(bird_x<=bar1_xTop))
			score1++;
		else
		{
			printf("游戏失败\n");
			system("pause");
			exit(0);
		}
	}
	if (bar1_y<=0)  // 再新生成一个障碍物
	{
		bar1_y = width;
		int temp = rand()%int(high*0.8);
		bar1_xDown = temp - high/10;
		bar1_xTop = temp + high/10;
	}
	
	Sleep(150);
}

void updateWithInput()  // 与用户输入有关的更新
{	
	char input;
	if(kbhit())  // 判断是否有输入
	{
		input = getch();  // 根据用户的不同输入来移动，不必输入回车
		if (input == ' ')  
			bird_x = bird_x - 2;
	}
}


int main ()                            //主函数 
{                                           
    EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);     //禁用关闭键，以此达到使用ESC键退出并存档的效果 
    HideCursor();  //隐藏光标
    system("mode con cols=112 lines=21");  //控制窗口大小
    
    PlaySound("D:\\stick_war\\vic.wav",NULL,SND_FILENAME | SND_ASYNC | SND_LOOP);   //播放音乐 
    
   	char inputChar;
	inputChar = tolower(ShowMainMenu());
	switch(inputChar)
		{
			case '1':
				Loading(); 
		    break;
		    case '2':
		    	load();
		    	level-=48;
				Loading(); 
		    break;
		    case '4' :
		    	system("cls");                            
		    	startup();  // 数据初始化
				HideCursor();	
	 			system("mode con cols=28 lines=20");  //控制窗口大小
				while (1)  //  游戏循环执行
				{
					show();  // 显示画面
					updateWithoutInput();  // 与用户输入无关的更新
	  				updateWithInput();     // 与用户输入有关的更新
				}
				return 0;
			}
    
    return 0;
}



