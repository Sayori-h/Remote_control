#pragma once
class CHuxlTool
{
public:
	static void dump(BYTE* pData, size_t nSize);
	static bool IsAdmin();
	static bool RunAsAdmin();
	static void ShowError();
	static BOOL WriteStartupDir(const CString& strPath);//通过修改开机启动文件夹实现开机启动
	static bool WriteRegisterTable(const CString& strPath);//通过修改注册表来实现开机启动
	static bool Init();//用于带MFC命令行项目初始化（通用）
};

