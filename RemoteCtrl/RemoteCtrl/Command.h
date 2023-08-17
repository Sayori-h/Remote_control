#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <corecrt_io.h>
#include "Packet.h"
#include "LockInfoDialog.h"
#include "CHuxlTool.h"
#include <list>

class CCommand
{
public:
	typedef int(CCommand::* CMDFUNC)(std::list<CPacket>& lstPacket, CPacket& inPacket);//����ָ��
	std::map<int, CMDFUNC>m_mapFunction;// ������ŵ����ܵ�ӳ��
	CLockInfoDialog dlg;
	unsigned threadid;
	//Ĭ�Ϲ��캯����ʹ��ӳ���ʽ������Զ�̲�������ӳ��
	CCommand();
	~CCommand() {};
	static void runCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket);
	int ExcuteCommand(int nCmd, std::list<CPacket>& lstPacket, CPacket& inPacket);
	//�������̷�������Ϣ�����ڲ鿴�ļ�
	int makeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//�鿴ָ��Ŀ¼�µ��ļ�
	int makeDirectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//�����ļ�
	int runFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//�����ļ�
	int downLoadFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//������
	int mouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//������ĻͼƬ
	int sendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket);
	/*******************************����***************************************/
	//�������߳��н���ת����������
	static unsigned __stdcall threadLockDlg(void* arg);
	//����ʵ��
	void threadLockDlgMain();
	//��������
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	/***************************************************************************/
	//����
	int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//���Ӳ���
	int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket);
	//ɾ���ļ�
	int DeleteLocalFile(std::list<CPacket>& lstPacket, CPacket& inPacket);
};

