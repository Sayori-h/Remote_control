#pragma once
class CHuxlTool
{
public:
	static void dump(BYTE* pData, size_t nSize);
	static bool IsAdmin();
	static bool RunAsAdmin();
	static void ShowError();
	static BOOL WriteStartupDir(const CString& strPath);//ͨ���޸Ŀ��������ļ���ʵ�ֿ�������
	static bool WriteRegisterTable(const CString& strPath);//ͨ���޸�ע�����ʵ�ֿ�������
	static bool Init();//���ڴ�MFC��������Ŀ��ʼ����ͨ�ã�
};

