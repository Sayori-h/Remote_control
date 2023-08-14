#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "StatusDlg.h"
#include "resource.h"
#include "RemoteClientDlg.h"
#include "CHuxlTool.h"
#include <map>
					    
//#define WM_SEND_DATA    (WM_USER + 2)//��������
#define WM_SHOW_STATUS  (WM_USER + 3)//չʾ״̬
#define WM_SHOW_WATCH   (WM_USER + 4)//Զ�̼���
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����

//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();

	//��ʼ������,�����߳�
	int InitController();

	//���� ������ʾMFC���棬������ʾԶ�ؿ���̨m_remoteDlg�������洰��
	int Invoke(CWnd*&pMainWnd);

	//������Ϣ
	//LRESULT SendMessage(MSG msg);
	
	//��������������ĵ�ַ
	//֪ͨClientSocket��������UpdateAddress()�����ڸ��·����IP�Ͷ˿�
	void UpdateAddress(int nIP, int nPort);
	
	//���׽���ת��Ϊcontroller��һ���ţ�ն�Ͻ�������ݵ����
	//֪ͨClientSocket��������DealCommand()��������õ�����  ����ֵ������
	int DealCommand();
	
	//֪ͨClientSocket�����ر��׽���
	void CloseSocket();
	//���س�Աm_packet
	CPacket& GetPacket();
	
	/*----------�������ݰ�-------------------
	1���鿴���̷���     ||   2���鿴ָ��Ŀ¼�µ��ļ�
	3�����ļ�	      ||   4�������ļ�
	5��������	      ||   6��������Ļ������
	7������			  ||   8������
	9��ɾ���ļ�		  ||   2001����������
	����˵����֪ͨClientSocket��������SendPacket()�������ݰ�����
	����1�����������Ĵ��ڽ��棬���ݰ��յ�����ҪӦ��Ĵ���
	����2������
	����3���Ƿ��Զ��رգ�Ĭ��Ϊtrue
	����4����Ҫ���͵����ݰ�
	����5�����ݳ���
	����6���ò������ڴ����ļ���ָ�룬�����ļ����أ��������Ϊ0
	����ֵ���ɹ�����true,ʧ�ܷ���fasle*/
	bool SendCommandPacket(HWND hWnd, int nCmd,bool bAutoClose = true,
		 BYTE* pData = NULL,size_t nLength = 0,WPARAM wParam = 0
		/*std::list<CPacket>* plstPacks=NULL������Ӧ��Ӧ��ͨ������Ϣ��������Ҫ����*/);
		
	
	//��ȡͼƬ��ͼ������
	//��ȡ���ݣ�����ʹ�ù��߿�CTool���ֽ�ת��ΪͼƬ��ʽ����
	//����1��ͼƬCImage����
	int GetImage(CImage& image);
	
	//������ɣ��������ش��ڣ��ָ����
	void DownloadEnd();

	//�����ļ�
	int DownFile(CString& strPath);

	//����Զ��ͼ���̣߳�Ȼ����ʾͼ������
	void StartWatchScreen();
protected:
	//Ĭ�Ϲ��캯������ʼ����Ա������ָ��ͼ�������ش��ڵĸ�����Ϊ�����洰��
	CClientController();

	//�����������ȴ��߳̽���������
	~CClientController();

	/*ͼ���̺߳��������Ʒ���Ƶ�ʣ�ѭ������SendCommandPack()����ͼ������
	�ڲ���Ҫʹ�õ�ϵͳ����: GetTickCount64()*/
	void threadWatchScreen();

	/*�ò�����Ҫ�����߳���������ز��������������̵߳�����Ҫ�����ȣ��������̲߳��ܴ���thisָ�룬
	Ҳ��˵���̺߳���Ϊ��̬�������������̺߳�������Ҫ�õ�thisָ������������Ա��������Ա������)��
	�����Ҫ��ת������ɣ���thisָ�뵱�ɲ��������̣߳���ͨ��thisָ����ó�Ա����*/
	static void threadWatchScreenEntry(void* arg);

	void threadFunc();

	//unsignedΪ�˻�ȡ�߳�ID
	static unsigned __stdcall threadEntry(void* arg);

	static void releaseInstance();
	//LRESULT OnSendPack(UINT nMSG, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMSG, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	//�ص�����
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMSG,WPARAM wParam, LPARAM lParam);
	//��Ϣ����ӳ��
	static std::map<UINT, MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;//Զ������Ի���
	CRemoteClientDlg m_remoteDlg;//��������ƶԻ���
	CStatusDlg m_statusDlg;//�������ضԻ���
	HANDLE m_hThread;//�߳�
	HANDLE m_hThreadWatch;//Զ�������߳�
	bool m_isClosed;//�����Ƿ�ر�
	CString m_strRemote;//�����ļ���Զ��·��
	CString m_strLocal;//�����ļ��ı��ر���·��
	unsigned m_nThreadID;//�߳�ID
	static CClientController* m_instance;//ȫ��Ψһ����
	class CNewAndDel;
	static CNewAndDel m_newdel;
};

