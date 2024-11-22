/* =========================Instructions=========================
* How to use the automatic crash detection function£¿
* Call the StartDetectCrash() function at program startup.
* A simple example:
* void main(){
* StartDetectCrash();
* // The following is your implementation code:
* }
*  =========================Instructions=========================
*/

#pragma once
#include <afx.h>
#include <string>

enum DumpFileType{
	DumpFileType_Full = 0,	//full memory
	DumpFileType_Normal,	//normal memory
	DumpFileType_None,		//do not save
};

extern void StartDetectCrash(size_t type = DumpFileType_Full);

class DumpFileManager{
public:
	void SetDumpFileType(size_t type) { dumpFileType_ = type; }

	void RunCrashHandler();

	static DumpFileManager& getInstance() {
		static DumpFileManager instance;
		return instance;
	}

private:
	DumpFileManager();

	~DumpFileManager();

	DumpFileManager(const DumpFileManager&);

	DumpFileManager& operator=(const DumpFileManager&);

	DumpFileManager(DumpFileManager&&);

	DumpFileManager& operator=(DumpFileManager&&);

	void DisableSetUnhandledExceptionFilter();

	static long WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS* exception);

	void PrintDumplog(const char* patch, const char* msg);

	void CheckDumpFileNumber(CString filePath);

	bool CreateDumpFile(EXCEPTION_POINTERS* exception, LPCTSTR fileName);

	static std::string CString2String(CString target);

	static CString String2CString(const char* str);

	int					dumpFileType_;

	CString				dumpFileName_;

	CString				dumpFilePath_;
};