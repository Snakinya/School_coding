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

typedef struct                //����ĳ��ִ���ָ����һ��AI_stick�����еĴ�����Ϸ�����ĸ�AI_stick����
{
    int fast_stick_order;   //fast_stick���ֵĴ���(�ڵ�fast_stick_order�θ������,�ӵ�0�ο�ʼ)����ÿ��AI_stickֻ����һ��
    int firm_stick_order;   //firm_stick���ֵĴ���ͬ��
} LevInfo;                 //�ؿ���Ϣ(׼ȷ˵�Ǹùس��ֵĻ������Ϣ)
LevInfo level_info [MAX_LEVEL] = {{-1,-1},{1,1},{2,3}};  //��ʼ����-1����û�и����ͻ����

typedef struct      //�ӵ��ṹ��
{
    int x,y;        //�ӵ�����
    int direction;  //�ӵ��������
    bool exist;     //�ӵ����ڱ���
    bool initial;   //�ӵ��Ƿ��ڽ�����״̬�ı���
    bool my;        //����AI�ӵ�������ӵ��ı��,0ΪAI�ӵ���1Ϊ����ӵ�
} Bullet;
Bullet bullet [BULLET_NUM];

typedef struct      //����˽ṹ��
{
    int x,y;        //�������������
    int direction;  //����˷���
    int color;      //��ɫ����
    int model;      //�����ͼ��ģ�ͣ�ֵΪ1,2,3���ֱ����ͬ�Ļ����ͼ��,0Ϊ�ҵĻ����ͼ����AI����ʹ��
    int stop;       //ֻ����AI�����ʹ�õĲ�������0��������ֹͣ�߶�,0Ϊ�����߶�
    int revive;     //����˸������
    int num;        //AI����˱��0~3
    int CD;         //�����ӵ���ȴ��ʱ
    bool my;        //�Ƿ�з�����˲������ҵĻ���˴˲���Ϊ1,Ϊ����
    bool alive;     //���Ϊ1�������Ϊ0
}  Stick;
Stick AI_stick[4] ,my_stick1 ,my_stick2;  //my_stickΪ�ҵĻ���ˣ�AI_stick ����AI�����

//������������Ϸ��������
void Loading();            //װ�ؽ��� 
char ShowMainMenu();       //��ʼ���� 
void GoToxy(int x,int y);  //����ƶ�
void HideCursor();  //���ع��
void keyboard();  //���ռ�������
void Getmap();  //��ͼ���ݴ�����ȡ
void Initialize();  //��ʼ��(���жԶ�����ݵĶ�д)
void Stop();  //��ͣ
void Frame ();  //��ӡ��Ϸ������
void PrintMap();  //��ӡ��ͼ
void SideScreen ();  //����Ļ��ӡ
void GameCheak();  //�����Ϸ��Ӯ
void GameOver( bool home );  //��Ϸ����
void ClearMainScreen();  //����Ļ��������
void ColorChoose(int color);  //��ɫѡ����
void NextLevel();  //��һ��(���ж�levelȫ�ֱ����Ķ�д)

//�ӵ�����
void BuildAIBullet(Stick *stick);  //AI����˷����ӵ�
void BuildBullet(Stick stick);  //�ӵ����䣨���������˻����ã�
void BulletFly(Bullet bullet[BULLET_NUM]);  //�ӵ��ƶ��ʹ�����˻����ã�
void BulletHit(Bullet* bullet);  //�ӵ���ײ���˻����ã�
void PrintBullet(int x,int y,int T);  //��ӡ�ӵ����˻����ã�
void ClearBullet(int x,int y,int T);  //����ӵ����˻����ã�
int  BulletCheak(int x,int y);  //�ж��ӵ�ǰ��������˻����ã�
 
//����˲���
void BuildAIStick (int* position, Stick* AI_stick);  //����AI�����
void BuildMyStick1 (Stick* my_stick1);  //�����ҵĻ����
void BuildMyStick2 (Stick* my_stick2);
void MoveAIStick  (Stick* AI_stick);  //AI������ƶ�
void MoveMyStick1  (int turn);  //�ҵĻ�����ƶ�
void MoveMyStick2  (int turn);
void ClearStick   (int x,int y);  //�������ˣ��˻����ã�
void PrintStick   (Stick stick);  //��ӡ����ˣ��˻����ã�
bool StickCheak   (Stick stick,int direction);  //�������dirtection������ϰ���,��ֵ1�谭,0 ��ͨ
int  AIPositionCheak (int position);  //���AI����˽���λ���Ƿ����ϰ���AIPositionCheak
//�浵����
FILE * mem;     //��Ϸ�浵�ļ����
char dir = UP;  //��Ϊ��save������Ҫ�õ�����ԭ�����������������dir�Ƴ���������ʼ��Ϊ����
void save(void);    //��Ϸ�浵����
void load(void);	//����
//egg
int high,width; // ��Ϸ�����С
int bird_x,bird_y; // С�������
int bar1_y,bar1_xDown,bar1_xTop; // �ϰ���1���������
int score1; // �÷֣������ϰ���ĸ��� 
//ȫ�ֱ���
int map[41][41];  //��ͼ��ά����
int bul_num;  //�ӵ����
int position;  //λ�ü���,��ӦAI���������λ��,-1Ϊ��λ��,0Ϊ�м�,1Ϊ��,2Ϊ�ҵĻ����λ��
int speed=7;  //��Ϸ�ٶ�,������
int level=1;  //��Ϸ�ؿ���
int score=0;  //��Ϸ����
int remain_enemy;  //ʣ�����
 
char* stick_figure[4][3][4]=
{
  {
    {"������", "������", "���ר�", "���ר�"},
    {"�ǡ��", "�ǡ��", "����", "����"},
    {"������", "������", "���ߨ�", "���ߨ�"}
  },
  {
    {"������", "���ש�", "���ש�", "���ש�"},
    {"�ǡ��", "�ǡ��", "�����", "�ǡ�"},
    {"���ߩ�", "������", "���ߩ�", "���ߩ�"}
  },
  {
    {"������", "������", "���ר�", "���ש�"},
    {"�ǡ��", "�ǡ��", "����", "����"},
    {"������", "������", "���ߨ�", "���ߩ�"}
  },
  {
    {"�X���[", "�X�j�[", "�X�j�[", "�X�j�["},
    {"�d��g", "�d��g", "����g", "�d��"},
    {"�^�m�a", "�^���a", "�^�m�a", "�^�m�a"}
  }
};



void GoToxy(int x,int y)  //����ƶ�������X��ʾ�����꣬Y��ʾ�����ꡣ
{
    COORD  coord;         //ʹ��ͷ�ļ��Դ�������ṹ
    coord.X=x;            //���ｫint����ֵ����short,�����������漰������ֵ�����ᳬ��short��Χ
    coord.Y=y;
    HANDLE a=GetStdHandle(STD_OUTPUT_HANDLE);  //��ñ�׼������
    SetConsoleCursorPosition(a,coord);         //�Ա�׼����ľ��Ϊ�������ÿ���̨�������
}

void HideCursor()  //���ع��
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
    	 save();   //�浵���� 
    	 system("cls");
		 printf("\n\n\n\n\t\t\t\tARCHIVING������") ;
		 Sleep(1000); 
         exit(0);  //�˳�������
	}     
    else if (GetAsyncKeyState( 0x20 )& 0x8000)
        Stop();
    else if (count++%7==0)
    {
        if (speed>1 && GetAsyncKeyState( 0x6B )& 0x8000)   // +��
        {
            speed--;
            GoToxy(102,11);           //�ڸ���Ļ��ӡ����ǰ�ٶ�
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_RED);
            printf("%d ",21-speed);
        }
        else if (speed<20 && GetAsyncKeyState( 0x6D )& 0x8000)  // - ��
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

void GetMap()      //��ͼ��ź���
{                //map���ֵ: ��λ����ֵΪ��ͼ���鲿�֣���λ����ֵΪ�����
    int i ,j;      //map���ֵ: 0ͨ·��1��ש��5ˮ��6��ש��100Ϊ�з�����ˣ�200Ϊ�ҵĻ���ˣ�
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
    PrintMap();         //��ӡ��ͼ
}

void Initialize()  //��ʼ��
{
    remain_enemy=16;
    my_stick1.revive=0;  //�ҵĻ���˸������Ϊ0
    my_stick2.revive=0;
    position=0;
    bul_num=0;
    GetMap();
    BuildMyStick1( &my_stick1 );
    BuildMyStick2( &my_stick2 );
    for(int i=0;i<12;i++)     //�ӵ���ʼ��
    {
        bullet [i].exist=0;
        bullet [i].initial=0;
    }
    for(int i=0;i<=3;i++)         //AI����˳�ʼ��
    {
        AI_stick [i].revive=0;
        AI_stick [i].alive=0;  //��ʼ�������ȫ�ǲ����ģ�BuildAIStick()�Ὠ�����½��������Ļ����
        AI_stick [i].stop=0;
        AI_stick [i].num=i;
        AI_stick [i].my=0;
        AI_stick [i].CD=0;
    }
    GoToxy(97,2);                        //�ڸ���Ļ�Ϲؿ���
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN);
    printf("%d",level);
    GoToxy(102,5);                       //�ڸ���Ļ�ϴ�ӡ����
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("%d   ",score);
    GoToxy(102,7);                       //�ڸ���Ļ��ӡʣ������ֵ
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("%d", MAX_LIFE-my_stick1.revive);
    GoToxy(102,9);                       //�ڸ���Ļ�ϴ�ӡʣ��������
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
    printf("%d ",remain_enemy);
    GoToxy(100,13);                      //�ڸ���Ļ�ϴ�ӡ״̬
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_GREEN);
    printf("Playing");
}

void Stop()    //��ͣ
{
    int color=1,timing=0;
    while(1)
    {
        if(timing++%30==0)
        {
            ColorChoose(color);   //��ɫѡ��
            GoToxy(100,13);       //����Ļ��ӡ
            printf("Game Stop");
            GoToxy(88,17);
            printf("Press enter to return");
            GoToxy(88,18);
            printf("or press ESC to exit");
            if(++color==8)
                color=1;
        }
        if (GetAsyncKeyState( 0xD )& 0x8000)      //�س���
        {
            GoToxy(100,13);       //����Ļ��ӡ
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
            printf("  Playing");   //���ǵ�ԭ������ʾ
            GoToxy(88,17);
            printf("                     ");
            GoToxy(88,18);
            printf("                     ");
            break;
        }
        else if(GetAsyncKeyState( 0x1B )& 0x8000) //Esc���˳�
            exit(0);
        Sleep(20);
    }
}

void Frame ()     //��ӡ��Ϸ������
{                 //SetConsoleTextAttributeΪ�����ı���ɫ���ı�������ɫ����
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("  �x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x�x  ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
    printf("  �y�y�y�y�y�y�y�y�y�y�y�y�y \n");
    for(int i=0;i<14;i++)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
        printf("��                                                                              ��");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
        printf(" |                          |\n");
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("��                                                                              ��");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
    printf(" |�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T|\n");
    for(int i=0;i<24;i++)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
        printf("��                                                                              ��");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY|FOREGROUND_BLUE);
        printf(" |                          |\n");
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_INTENSITY);
    printf("  ������������������������������������������������������������������������������  ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY| FOREGROUND_BLUE);
    printf(" �i�i�i�i�i�i�i�i�i�i�i�i�i�i\n");
    SideScreen ();  //��ӡ����Ļ
}

void PrintMap()     // ��ӡ��ͼ(��ͼ�ȵ�ͼ�ϰ���)
{
    for(int j=0;j<41;j++)
        for(int i=0;i<41;i++)
            if(map[i][j]==6)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN
                    |FOREGROUND_RED|FOREGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE);
                GoToxy(2*j,i);
                printf("��");
            }
            else if(map[i][j]==1)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|BACKGROUND_GREEN|BACKGROUND_RED);
                GoToxy(2*j,i);
                printf("��");
            }
            else if(map[i][j]==5)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|BACKGROUND_BLUE|FOREGROUND_BLUE|FOREGROUND_GREEN);
                GoToxy(2*j,i);
                printf("��");
            }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);
    GoToxy(38,37);     printf("�切��");
    GoToxy(38,38);     printf("���切");
    GoToxy(38,39);     printf("������");
}

void SideScreen ()
{                   // |         ��  d  ��         |   " |                          |\n"
    GoToxy(92,2);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    printf("The     Level");
    GoToxy(92,5);
    //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("SCORE��");
    GoToxy(92,7);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("LIFE��");
    GoToxy(86,9);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
    printf("Remaining enemy��");
    GoToxy(86,11);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
    printf("SPEED��  %d",21-speed);
    GoToxy(86,13);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
    printf("Game status��");
    GoToxy(94,19);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    GoToxy(94,24);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_RED);
    printf("HELP");
    GoToxy(88,21);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
    printf("WASD �������� Move");
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
        if(timing++%30==0)        //��Ϸ����ԭ��Ϊ����ֵΪ0
        {
            ColorChoose(color);    //��ɫѡ��
            if(home)
            {
                GoToxy(37,19);     //����Ļ���Ĵ�ӡ
                printf("G    G��");
            }
            GoToxy(37,20);         //����Ļ���Ĵ�ӡ
            printf("Game over!");
            GoToxy(100,13);        //����Ļ��ӡ
            printf("Game over!");
            GoToxy(88,17);
            printf("Press enter to return!");
            GoToxy(88,18);
            printf("Or press Esc to exit!");
            if(++color==8)
                color=1;
        }
        if (GetAsyncKeyState( 0xD )& 0x8000)  //�س���
        {
            score-=500;  //����-500
            ClearMainScreen();  //�������������������ٴδ�ӡ���
            Initialize();  //�ӱ������¿�ʼ
            break;
        }
        else if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc���˳�
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

void ColorChoose(int color)   //��ɫѡ����
{
    switch(color)
    {
        case 1:               //��ɫ(���������ɫ1)
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN|FOREGROUND_BLUE);
            break;
        case 2:               //��ɫ
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
            break;
        case 3:               //��ɫ
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN);
            break;
        case 4:               //��ɫ
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
            break;
        case 5:               //��ɫ(���������ɫ2)
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
            break;
        case 6:               //��ɫ
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN);
            break;
        case 7:               //����ɫ
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
                ColorChoose(color);   //��ɫѡ��
                GoToxy(37,20);        //����Ļ���Ĵ�ӡ
                printf("Congratulations!");
                GoToxy(87,17);
                printf("Press enter to next!");
                GoToxy(88,18);
                printf("Or press exit to exit!");
                if(++color==8)
                    color=1;
            }
            if (GetAsyncKeyState( 0xD )& 0x8000)  //�س���
            {
                GoToxy(88,17);        //Ĩ������Ļ�е���ʾ
                printf("                     ");
                GoToxy(88,18);
                printf("                     ");
                ClearMainScreen();   //�������������������ٴδ�ӡ���
                Initialize();       //��ʼ������һ�ؿ�ʼ,level��++
                break;
            }
            else if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc���˳�
                exit(0);
            Sleep(20);
        }
    else   //level>3 ͨ��
        while(1)
        {
            if(timing++%5==0)
            {
                ColorChoose(color);
                GoToxy(33,20);        //����Ļ���Ĵ�ӡ
                printf("Pass all the levels!");
                GoToxy(100,13);       //����Ļ��ӡ
                printf("Pass all");
                GoToxy(88,17);
                printf("Congratulations!!");
                GoToxy(88,19);
                printf("Press ESC to exit!");
                if(++color==8)
                    color=1;
            }
            if(GetAsyncKeyState( 0x1B )& 0x8000)  //Esc���˳�
                exit(0);
            Sleep(10);
        }
}


void BuildAIBullet(Stick *stick)   //AI�ӵ����京�ж�my_stick�Ķ�ȡ
{
    if(stick->CD==15)
    {
        if(!(rand()%11))     //��ȴ������������ÿ����Ϸ��������10��֮һ�Ŀ��ܷ����ӵ�
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
            if(stick->x < 20) //�ڻ������
            {
                if(stick->direction==RIGHT)  //����˷�����
                {
                    BuildBullet(*stick);     //�����ӵ�
                    stick->CD=0;
                }
            }
            else             //�ڻ����ұ�
                if(stick->direction==LEFT)   //����˷�����
                {
                    BuildBullet(*stick);     //�����ӵ�
                    stick->CD=0;
                }
        }
        else if(stick->x==my_stick1.x+1 || stick->x==my_stick1.x || stick->x==my_stick1.x-1)  //AI�������������"�ڿ�"��׼�ҵĻ����
        {
            if(stick->direction==DOWN && my_stick1.y > stick->y || stick->direction==UP && my_stick1.y < stick->y)
            {  //����AI���²����ҵĻ������AI������·�����AI�����ҵĻ������AI�Ϸ�
                int big=my_stick1.y , small=stick->y , i;
                if(my_stick1.y < stick->y)
                {
                    big=stick->y;
                    small=my_stick1.y;
                }
                for(i=small+2;i<=big-2;i++)  //�ж�AI�ڿڵ�ֱ����������˼������ϰ�
                    if(map[i][stick->x]!=0 || map[i][stick->x]!=5)  //�����ϰ�
                        break;
                if(i==big-1)  //��i�ߵ�big-1˵�����ϰ�
                {
                    BuildBullet(*stick);  //�����ӵ�
                    stick->CD=0;
                }
            }
        }
        else if(stick->y==my_stick1.y+1 || stick->y==my_stick1.y || stick->y==my_stick1.y-1)  //AI������ں�����"�ڿ�"��׼�ҵĻ����
        {
            if(stick->direction==RIGHT && my_stick1.x > stick->x || stick->direction==LEFT && my_stick1.x < stick->x)
            {  //����AI���Ҳ����ҵĻ������AI������ҷ�(��ֵ���������)����AI�����ҵĻ������AI��
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
    bullet [bul_num].exist = 1;  //�ӵ�������,��ֵΪ1����ӵ�����
    bullet [bul_num].initial = 1;  //�ӵ����ڳ�����״̬
    bullet [bul_num].my=stick.my;  //������ҵĻ���˷�����ӵ�bullet.my=1������Ϊ0
    bul_num++;
    if(bul_num==BULLET_NUM)
        bul_num=0;
}

void BulletFly(Bullet bullet[BULLET_NUM]) //�ӵ��ƶ��ʹ��
{
    for(int i =0; i<BULLET_NUM;i++)
    {
        if(bullet [i].exist)  //����ӵ�����
        {
            if(bullet [i].initial==0)  //����ӵ����ǳ�������
            {
                if(map[bullet[i].y] [bullet[i].x]==0 || map[bullet[i].y] [bullet[i].x]==5)  //����ӵ����굱ǰλ�����ϰ�
                    ClearBullet( bullet[i].x , bullet[i].y , BulletCheak(bullet[i].x , bullet[i].y ));  //Ĩ���ӵ�ͼ��
                switch(bullet [i].direction)  //Ȼ���ӵ�����仯���ӵ��䵽��һ�����꣩
                {
                    case UP    :(bullet [i].y)--;break;
                    case DOWN  :(bullet [i].y)++;break;
                    case LEFT  :(bullet [i].x)--;break;
                    case RIGHT :(bullet [i].x)++;break;
                }
            }
            int collide = BulletCheak ( bullet [i].x , bullet [i].y );  //�ж��ӵ���ǰλ�����,�ж��ӵ��Ƿ���ײ
            if( collide )   //�����⵽��ǰ�ӵ��������ϰ�
                PrintBullet( bullet[i].x , bullet[i].y , collide);       //���ӡ�ӵ���������ײ�򲻴�ӡ
            else
                BulletHit( & bullet [i] );  //������ײ��ִ���ӵ���ײ����
            if(bullet [i].initial)  //���ӵ�����������ѳ��������ȥ��
                bullet [i].initial = 0;
            for(int j=0; j< BULLET_NUM ; j++)  //�ӵ������ײ�ж�,�����ҷ��ӵ��͵з��ӵ���ײ��ɾ��,��Ϊ���з��ӵ�������
                if(bullet [j].exist && j!=i && (bullet[i].my || bullet[j].my) && bullet[i].x==bullet[j].x && bullet[i].y==bullet[j].y)
                {  //ͬ���������ҷ��ӵ������ܲ�����ײ
                    bullet [j].exist=0;
                    bullet [i].exist=0;
                    ClearBullet( bullet[j].x , bullet[j].y , BulletCheak(bullet[j].x , bullet[j].y ));  //Ĩ��j�ӵ�ͼ��,�ӵ�iͼ���ѱ�Ĩ��
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
    if(map[y][x]==1 || map[y][x]==2)  //�ӵ�����ש��
    {
        if(bullet->direction==UP || bullet->direction==DOWN)   //����ӵ��������
            for(i = -1 ; i<=1 ; i++)
                if(map[y][x+i]==1 || map[y][x+i]==2)  //����ӵ�����ש������Ϊש��,��ɾ��ש,������(һ��Ϊ����˻���������)�����
                {
                    map[y][x+i]=0;  //ש����
                     GoToxy(2*x+2*i,y);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED); //������ɫ
                     printf("  ");
                }
        if(bullet->direction==LEFT || bullet->direction==RIGHT)     //���ӵ��Ǻ����
            for(i = -1 ; i<=1 ; i++)
                if(map[y+i][x]==1 || map[y+i][x]==2)
                {
                    map[y+i][x]=0;
                     GoToxy(2*x,y+i);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED); //������ɫ
                     printf("  ");
                }
        bullet->exist=0;  //����ӵ���ʧ
    }
    else if(map[y][x]==4 || map[y][x]==6 )  //�ӵ������߿���߲��ɴݻٷ���
        bullet->exist=0;
    else if(bullet->my && map[y][x]>=100 && map[y][x]<104 )  //���ҵ��ӵ������˵з������
    {
        int num = map[y][x]%100;   //��ȡ�û������Ϣ
        if(AI_stick[num].model==3 && AI_stick[num].color==2)   //��Ϊfirm stick,��color==2���û����Ϊ��ɫ,����û���ܵ��˺�
                AI_stick[num].color=3;                        //���ɻ�ɫ��color=3Ϊ��ɫ
        else if (AI_stick[num].model==3 && AI_stick[num].color==3)
                AI_stick[num].color=4;                        //4Ϊ��ɫ
        else  //�������͵Ļ���˻���firm stickΪ��ɫ�����
        {
            AI_stick[num].alive=0;
            ClearStick(AI_stick[num].x , AI_stick[num].y);      //����û����
        }
        bullet->exist=0;
        score+=100;
        GoToxy(102,5);  //�ڸ���Ļ�ϴ�ӡ������
        //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
        printf("%d ",score);
    }
    else if(map[y][x]==200 && bullet->my==0 )  //���з��ӵ������ҵĻ����
    {
        my_stick1.alive=0;
        my_stick2.alive=0;
        ClearStick(my_stick1.x , my_stick1.y);
        ClearStick(my_stick2.x , my_stick2.y);
        bullet->exist=0;
        my_stick1.revive++;  //�ҵĻ���˸������+1
        my_stick2.revive++;
        score-=100;  //��������
        GoToxy(102,5);  //�ڸ���Ļ�ϴ�ӡ������
        //SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED|FOREGROUND_BLUE);
        printf("%d   ",score);
        GoToxy(102,7);  //�ڸ���Ļ��ӡ���ҵ�ʣ������ֵ
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN);
        printf("%d   ", MAX_LIFE - my_stick1.revive);
    }
    else if(map[y][x]==9)  //�ӵ���������(������˭���ӵ�)
    {
        bullet->exist=0;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_GREEN);
        GoToxy(38,37);     printf("      ");
        GoToxy(38,38);     printf("������ ");
        GoToxy(38,39);     printf("������");
        GameOver(1);           //��Ϸ����,����1������ر���
    }
}

void PrintBullet (int x,int y,int T)   //��ǰ����BulletCheak ��ֵ������ T
{
    if(T==1)          //  T==1 ��ʾ�ӵ���ǰ������½����
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
    GoToxy(2*x,y);
    printf("��");
}

void ClearBullet(int x,int y,int T)   //��ǰ����BulletCheak ��ֵ������ T
{
    GoToxy(2*x,y);
    if(T==1)   //  T==1 ��ʾ�ӵ���ǰ������½����
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE);
        printf("  ");
    }
}

int BulletCheak (int x,int y)  //�ж��ӵ���ǰλ�����,�ж��ӵ��Ƿ���ײ
{
    if(map[y][x]==0)
        return 1;
    else
        return 0;
}


//positionΪ���������λ��,-1Ϊ��λ��,0Ϊ�м�,1Ϊ��,2Ϊ�ҵĻ����λ��
void BuildAIStick(int* position, Stick* AI_stick)   //ִ��һ�θú���ֻ����һ�������
{
       if(AIPositionCheak(*position))        //����λ�����ϰ�,�����ɡ�
    {
        AI_stick->x= 20 + 18*(*position);  //20 + 18 * position ��Ӧ��������λ�õ�x������
        AI_stick->y=2;
        if(AI_stick->revive==level_info[level-1].firm_stick_order)
        {
            AI_stick->model = 3;           //3Ϊfirm stick��ģ��(���)
            AI_stick->color = 2;           //��ɫ����2Ϊ��ɫ�������������ColorChoose
        }
        else if(AI_stick->revive==level_info[level-1].fast_stick_order)  //ͬ��if��������fast_stick��
        {
            AI_stick->model = 2;
            AI_stick->color = rand()%6+1;
        }
        else      //��ͨ�����
        {
            AI_stick->model = 1;
               AI_stick->color = rand()%6+1;  //������firm stick�������ɫ
        }
        AI_stick->alive = 1;       //����˱�Ϊ����
        AI_stick->direction = 2 ;  //������
        AI_stick->revive++;        //�������+1
        PrintStick(*AI_stick);
        (*position)++;
        remain_enemy--;
        GoToxy(102,9);            //�ڸ���Ļ�ϴ�ӡʣ��������
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_RED);
        printf("%d ",remain_enemy);
        if(*position==2)
            *position = -1;
             return ;
    }
}

void BuildMyStick1 (Stick* my_stick1) //�����ҵĻ����
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
    PrintStick (*my_stick1);  //��ӡ�ҵĻ����
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

void MoveAIStick(Stick* AI_stick) //AIר�ú���
{
       if(AI_stick->alive)         //�������˻���
    {
        if(AI_stick->stop!=0)   //������Ƿ�ֹͣ�˶����жϣ���stop������Ϊ0
        {
            AI_stick->stop--;   //��˻���˱��غ�ֹͣ�˶�
            return;
        }
        if( !(rand()%23) )     //22��֮1�ĸ���ִ�з�������
        {
            AI_stick->direction = rand()%4+1;
            if( rand()%3 )     //�ڷ������ú���2��֮1�ĸ���ֹͣ�߶�3����ʱ��
            {
                AI_stick->stop=2;
                return;
            }
        }
        ClearStick (AI_stick->x , AI_stick->y);
        if(StickCheak ( *AI_stick , AI_stick->direction))   //���ǰ�����ϰ�
            switch ( AI_stick->direction )
            {
                case UP   : AI_stick->y--; break;  //��ǰ��һ��
                case DOWN : AI_stick->y++; break;  //��ǰ��һ��
                case LEFT : AI_stick->x--; break;  //��ǰ��һ��
                case RIGHT: AI_stick->x++; break;  //��ǰ��һ��
            }
        else                     //ǰ�����ϰ�
        {
            if(!(rand()%4))      //3��֮1�ĸ�����ת
            {
                AI_stick->direction=rand()%4+1;
                AI_stick->stop=2; //��ת֮��ֹͣ�߶�3����ʱ��
                PrintStick(*AI_stick);
                return;
            }
            else                 //����3��֮2�ļ���ѡ����ȷ�ķ���
            {
                int j;
                for(j=1;j<=4;j++)
                    if(StickCheak ( *AI_stick , j ))  //ѭ���жϻ�������������ϰ�,�˺�����ֵ1Ϊ��ͨ��
                        break;
                if(j==5)         //j==5˵���˻�������ܶ����ϰ���޷�ͨ��
                {
                    PrintStick(*AI_stick);
                    return;
                }
                while(StickCheak ( *AI_stick , AI_stick->direction) == 0)  //���ǰ�������ϰ�
                    AI_stick->direction=(rand()%4+1);                    //�򻻸����������
            }
        }
        PrintStick(*AI_stick);
    }
}

void MoveMyStick1(int turn)   //���ר�ú�����turnΪkeyboard�����������벻ͬ�����������Ĳ�ͬ��ֵ
{
    ClearStick(my_stick1.x , my_stick1.y);
    my_stick1.direction=turn;  //����������ķ���ֵ�����ҵĻ���˷���ֵ
    if(StickCheak ( my_stick1 , my_stick1.direction ))
        switch (turn)
        {
            case UP : my_stick1.y--; break;  //��ǰ��һ��
            case DOWN : my_stick1.y++; break;  //��ǰ��һ��
            case LEFT : my_stick1.x--; break;  //��ǰ��һ��
            case RIGHT : my_stick1.x++; break;  //��ǰ��һ��
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

void ClearStick(int x,int y)   //�������˺���
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

void PrintStick(Stick stick)  //��ӡ�����
{
    ColorChoose(stick.color);  //��ɫѡ����
    char *(*stickF)[4] = stick_figure[stick.model];
    for(int i = 0; i < 3; i++)
    {
        GoToxy((stick.x-1)*2 , stick.y-1+i);
        printf("%s", stickF[i][stick.direction-1]);
         for(int j=0;j<3;j++)
            if(stick.my)  //��Ϊ�ҵĻ����
                map[stick.y+j-1][stick.x+i-1]=200;  //��map�ϰ�"�����"�Ÿ�����������һ���˵Ĳ������з���ֵΪ100~103,�ҷ�Ϊ200
            else
                map[stick.y+j-1][stick.x+i-1]=100+stick.num;
    }
}


bool StickCheak(Stick stick,int direction)  //�������ǰ���ϰ�����,����Ϊ�����ꡣ��ֵ1Ϊ��ͨ��,��ֵ0Ϊ�赲(�˻�����)
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
            printf("���󣡣�");
            Sleep(5000);
            return 0;
    }
}

int AIPositionCheak( int position )    //positionΪ���������λ��2Ϊ�ҵĻ����λ�ã�����ΪAIλ��-1Ϊ��λ��0Ϊ�м�λ�ã�1Ϊ��λ
{
    int    x,y;
    if(position==2)                    //2Ϊ�ҵĻ����λ�ã�������ʱ�ò���
        x=15,y=38;
    else
        y=2 , x= 20 + 18 * position ;  //20 + 18 * position ��Ӧ��������λ�õ�x������
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            if( map[y+j-1][x+i-1]!=0)  //��������ľŹ��������ϰ���
            {
                return 0;              //�򷵻�0����ʾ������λ�����谭
            }
    return 1;                          //��������1����ʾ������λ�����谭
}

char ShowMainMenu()
{
	char inputChar;

	//���� 
	system("cls");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_GREEN
		|FOREGROUND_RED|FOREGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_BLUE);
	
	printf("\n\n\n");
	//��ʾ�˵� 
	printf("\t\t\t��������������������������������������������������������������");
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
	printf("\t\t\t��������������������������������������������������������������\n\n\n");
	Sleep(150);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY|FOREGROUND_BLUE|FOREGROUND_GREEN);
	printf("\t\t\t1)New Game\n\n");
	Sleep(150);
	printf("\t\t\t2)Continue\n\n");
	
	//��ȡ���� 
	inputChar = _getch();
	return inputChar;
}

void Loading(){
	int i;
    unsigned int interval[12]={1,1,1,1,1,1,1,1,1,1,1,1} ;  //�ٶȿ���
    srand(time(NULL)); //�������������
    	system("cls");
	system("mode con cols=112 lines=42"); //���ƴ��ڴ�С
    Frame ();  //��ӡ������
    Initialize();
    for(;;)
    {
        if(interval[0]++%speed==0)
        {
            GameCheak();
            BulletFly ( bullet );
            for(i=0 ; i<=3 ; i++)
            {
                if(AI_stick[i].model==2 && interval[i+1]++%2==0)  //�ĸ�������еĿ��ٻ���˵���ʹ�ü�����1,2,3,4
                    MoveAIStick( & AI_stick[i]);
                if(AI_stick[i].model!=2 && interval[i+5]++%3==0)  //�ĸ�������е����ٻ���˵���ʹ�ü�����5,6,7,8
                    MoveAIStick( & AI_stick[i]);
            }
            for(i=0;i<=3;i++)  //����AI����˲���
                if(AI_stick[i].alive==0 && AI_stick[i].revive<4 && interval[9]++%90==0)  //һ���з������ÿ��ֻ��4����
                {
                    BuildAIStick( &position, & AI_stick[i] );  //����AI����ˣ����
                    break;  //ÿ��ѭ��ֻ����һ�������
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

    //�򿪴浵�ļ�
    if((mem = fopen("D:\\stick_war\\memory.txt","w+")) == NULL){
        fprintf(stderr,"No cache��\n");
        exit(1);
    }
	//����ؿ��� 
	fprintf(mem, "%d",level);
    fclose(mem);
} 

void load(void){
    //��ȡ��Ϸ�浵
    if((mem = fopen("D:\\stick_war\\memory.txt","r")) == NULL){
        fprintf(stderr,"Fail to load!\n");
        exit(2);
    }
    //��ȡ�ؿ��� 
    fread(&level,sizeof(level),1,mem);
    
    fclose(mem);
}

//EGG 
void gotoxy(int x,int y) //����ƶ���(x,y)λ��
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle,pos);
}

void startup()  // ���ݳ�ʼ��
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

void show()  // ��ʾ����
{
	gotoxy(0,0);  // ����ƶ���ԭ��λ�ã������ػ�����	
	int i,j;
	
	for (i=0;i<high;i++)
	{
		for (j=0;j<width;j++)
		{
			if ((i==bird_x) && (j==bird_y))
				printf("@");  //   ���С��
			else if ((j==bar1_y) && ((i<bar1_xDown)||(i>bar1_xTop)) )
				printf("*");  //   ���ǽ��
			else
				printf(" ");  //   ����ո�
		}
		printf("\n");
	}
	printf("�÷֣�%d\n",score1);
}	

void updateWithoutInput()  // ���û������޹صĸ���
{
	bird_x ++;
	bar1_y --;
	if (bird_y==bar1_y)
	{
		if ((bird_x>=bar1_xDown)&&(bird_x<=bar1_xTop))
			score1++;
		else
		{
			printf("��Ϸʧ��\n");
			system("pause");
			exit(0);
		}
	}
	if (bar1_y<=0)  // ��������һ���ϰ���
	{
		bar1_y = width;
		int temp = rand()%int(high*0.8);
		bar1_xDown = temp - high/10;
		bar1_xTop = temp + high/10;
	}
	
	Sleep(150);
}

void updateWithInput()  // ���û������йصĸ���
{	
	char input;
	if(kbhit())  // �ж��Ƿ�������
	{
		input = getch();  // �����û��Ĳ�ͬ�������ƶ�����������س�
		if (input == ' ')  
			bird_x = bird_x - 2;
	}
}


int main ()                            //������ 
{                                           
    EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);     //���ùرռ����Դ˴ﵽʹ��ESC���˳����浵��Ч�� 
    HideCursor();  //���ع��
    system("mode con cols=112 lines=21");  //���ƴ��ڴ�С
    
    PlaySound("D:\\stick_war\\vic.wav",NULL,SND_FILENAME | SND_ASYNC | SND_LOOP);   //�������� 
    
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
		    	startup();  // ���ݳ�ʼ��
				HideCursor();	
	 			system("mode con cols=28 lines=20");  //���ƴ��ڴ�С
				while (1)  //  ��Ϸѭ��ִ��
				{
					show();  // ��ʾ����
					updateWithoutInput();  // ���û������޹صĸ���
	  				updateWithInput();     // ���û������йصĸ���
				}
				return 0;
			}
    
    return 0;
}



