#include "DumpFileManager.h"

#include <map>
#include <vector>
#include <Psapi.h> 
#include <DbgHelp.h>
#include <Shlwapi.h>

#pragma comment(lib, "DbgHelp.lib") 
#pragma comment(lib, "psapi.lib ") 

DumpFileManager& _manager = DumpFileManager::getInstance();

void startDetectCrash(size_t type) {
	_manager.setDumpFileType(type);
	_manager.runCrashHandler();
}

DumpFileManager::DumpFileManager() : dumpFileType_(DumpFileType_Full) {

}

DumpFileManager::~DumpFileManager() {

}

void DumpFileManager::runCrashHandler() {
	if (!dumpFileName_.IsEmpty()) {// The automatic crash detection function has been activated
		return;
	}
	int pos = 0;
	CString path = _T("");
	TCHAR temp[MAX_PATH] = { 0 };
	TCHAR appPath[MAX_PATH] = { 0 };

	if (GetModuleFileName(NULL, temp, MAX_PATH) > 0) {
		path = temp;
		if ((pos = path.ReverseFind('\\')) > 0) {
			_tcscpy_s(appPath, path.Left(pos + 1));
		}
	}

	dumpFilePath_ = appPath;

	// dump file path ==> DumpFile subfolders in the path where the application is located
	dumpFilePath_ += _T("DumpFile\\");

	if (!PathFileExists(dumpFilePath_)) {
		CreateDirectory(dumpFilePath_, NULL);
	}

	// dump file name ==> The time to activate the automatic crash detection function
	SYSTEMTIME syt;
	GetLocalTime(&syt);
	dumpFileName_.Format(_T("%s%04d-%02d-%02d %02d-%02d-%02d.%03d.dmp"), dumpFilePath_, syt.wYear,
		syt.wMonth, syt.wDay, syt.wHour, syt.wMinute, syt.wSecond, syt.wMilliseconds);

	SetUnhandledExceptionFilter(unhandledExceptionFilterEx);
	disableSetUnhandledExceptionFilter();
}

void DumpFileManager::disableSetUnhandledExceptionFilter() {
	void* addr = (void*)GetProcAddress(LoadLibrary(_T("kernel32.dll")), "SetUnhandledExceptionFilter");
	if (addr) {
		unsigned char code[16];
		int size = 0;
		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
	}
}

long WINAPI DumpFileManager::unhandledExceptionFilterEx(struct _EXCEPTION_POINTERS* exception) {
	BOOL bRetVal = FALSE;

	if (!exception) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	_manager.checkDumpFileNumber(_manager.dumpFilePath_);
	_manager.createDumpFile(exception, _manager.dumpFileName_);
	TerminateProcess(GetCurrentProcess(), 0);
	return EXCEPTION_CONTINUE_SEARCH;
}

void DumpFileManager::printDumplog(const char* patch, const char* msg)
{
	std::string logFileName = cstring2String(dumpFilePath_) + patch;

	FILE* fp;
	fopen_s(&fp, logFileName.c_str(), "ab+");
	if (NULL == fp) {
		return;
	}

	SYSTEMTIME syt;
	GetLocalTime(&syt);
	char data[8000] = { 0 };
	sprintf_s(data, 8000, "%04d-%02d-%02d %02d:%02d:%02d.%3d: %s\r\n",
		syt.wYear, syt.wMonth, syt.wDay, syt.wHour, syt.wMinute, syt.wSecond, syt.wMilliseconds, msg);

	fwrite(data, sizeof(char), strlen(data), fp);
	fclose(fp);
}

void DumpFileManager::checkDumpFileNumber(CString filePath) {
	std::vector<CString> allFiles;
	std::map<CTime, CString> fileTimeMap;
	CString path = filePath;
	filePath += "*.dmp*";
	CFileFind finder;
	BOOL noEmpty = finder.FindFile(filePath);
	while (noEmpty) {
		noEmpty = finder.FindNextFile();
		CString path = finder.GetFileName();
		if (finder.IsDots()) continue;
		if (finder.IsDirectory()) continue;
		path.Insert(0, path);
		allFiles.push_back(path);
	}

	std::vector<CString>::iterator it_file = allFiles.begin();
	for (; it_file != allFiles.end(); it_file++) {
		path = *it_file;

		CFileStatus FileStatus;
		CFile::GetStatus(path, FileStatus);
		fileTimeMap[FileStatus.m_mtime] = path;
	}

	int	dumpCount = 5;// FullMemory ==> Up to 5 dump files can be stored
	if (DumpFileType_Normal == dumpFileType_) {
		dumpCount = 50;// NormalMemory ==> Up to 50 dump files can be stored
	}

	int fileCount = fileTimeMap.size();
	if (fileCount > dumpCount) {
		std::map<CTime, CString>::iterator itmap = fileTimeMap.begin();
		for (; itmap != fileTimeMap.end() && fileCount > dumpCount; itmap++, fileCount--) {
			DeleteFile(itmap->second);
		}
	}
}

bool DumpFileManager::createDumpFile(EXCEPTION_POINTERS* exception, LPCTSTR fileName) {
	DWORD handleCount;
	GetProcessHandleCount(GetCurrentProcess(), &handleCount);
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

	DWORD memoryCount = pmc.WorkingSetSize / 1024;
	DWORD pagefileUsage = pmc.PagefileUsage / 1024;

	char msg[128] = { 0 };
	sprintf_s(msg, 128, "CrashMsg: WorkingSetSize:%d(kb),PagefileUsage:%d(kb),HandleCount:(%d)", memoryCount, pagefileUsage, handleCount);
	printDumplog("DumpFile.log", msg);

	MINIDUMP_CALLBACK_INFORMATION mci;
	MINIDUMP_EXCEPTION_INFORMATION mdei;
	HANDLE file = nullptr;

	if (!exception || !fileName) {
		return false;
	}
	file = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file && file != INVALID_HANDLE_VALUE) {
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = exception;
		mdei.ClientPointers = FALSE;

		if (DumpFileType_Full == dumpFileType_) {
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpWithFullMemory, &mdei, NULL, NULL);
		}
		else {
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &mdei, NULL, NULL);
		}

		CloseHandle(file);
		return true;
	}

	return false;
}

std::string DumpFileManager::cstring2String(CString target) {
#ifdef _UNICODE		
	USES_CONVERSION;
	std::string result(W2A(target));
	return result;
#else		
	std::string result(target.GetBuffer());
	target.ReleaseBuffer();
	return result;
#endif	
}

CString DumpFileManager::string2CString(const char* target) {
#ifdef _UNICODE		
	CString result;
	int num = MultiByteToWideChar(0, 0, target, -1, NULL, 0);
	result.GetBufferSetLength(num + 1);
	MultiByteToWideChar(0, 0, target, -1, result.GetBuffer(), num);
	result.ReleaseBuffer();
#else		
	CString result;
	result.Format("%s", target);
#endif 	
	return result;
}