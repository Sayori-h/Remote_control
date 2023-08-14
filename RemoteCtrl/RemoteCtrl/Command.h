#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <corecrt_io.h>
#include "CServerSocket.h"
#include "LockInfoDialog.h"
#include "CHuxlTool.h"

class CCommand
{
public:
	typedef int(CCommand::* CMDFUNC)();//����ָ��
	std::map<int, CMDFUNC>m_mapFunction;// ������ŵ����ܵ�ӳ��
	CLockInfoDialog dlg;
	unsigned threadid;
	//Ĭ�Ϲ��캯����ʹ��ӳ���ʽ������Զ�̲�������ӳ��
	CCommand();
	~CCommand() {};
	int ExcuteCommand(int nCmd);
	//�������̷�������Ϣ�����ڲ鿴�ļ�
	int makeDriverInfo();
	//�鿴ָ��Ŀ¼�µ��ļ�
	int makeDirectoryInfo();
	//�����ļ�
	int runFile();
	//�����ļ�
	int downLoadFile();
	//������
	int mouseEvent();
	//������ĻͼƬ
	int sendScreen();
	/*******************************����***************************************/
	//�������߳��н���ת����������
	static unsigned __stdcall threadLockDlg(void* arg);
	//����ʵ��
	void threadLockDlgMain();
	//��������
	int LockMachine();
	/***************************************************************************/
	//����
	int UnLockMachine();
	//���Ӳ���
	int TestConnect();
	//ɾ���ļ�
	int DeleteLocalFile();
};

