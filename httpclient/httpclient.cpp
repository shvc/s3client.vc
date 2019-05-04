// test_winhttp.cpp :Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <winhttp.h>
#include <conio.h>
#pragma comment(lib, "Winhttp.lib")

typedef void(*DownLoadCallback)(int ContentSize, int CUR_LEN);


typedef struct _URL_INFO
{
    WCHAR szScheme[512];
    WCHAR szHostName[512];
    WCHAR szUserName[512];
    WCHAR szPassword[512];
    WCHAR szUrlPath[512];
    WCHAR szExtraInfo[512];
}URL_INFO;


void dcallback(int ContentSize, int file_size)
{
    printf("count:%d,size:%d\n", ContentSize, file_size);
}

void WinhttpDownload(const wchar_t *Url, const wchar_t *FileName, DownLoadCallback Func)
{
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    BOOL  bResults = TRUE;
    HINTERNET  hSession = NULL, 
               hConnect = NULL,
               hRequest = NULL;

    URL_INFO url_info = { 0 };
    URL_COMPONENTS UrlComponents = { 0 };

    UrlComponents.dwStructSize = sizeof(UrlComponents);
    UrlComponents.lpszExtraInfo = url_info.szExtraInfo;
    UrlComponents.lpszHostName = url_info.szHostName;
    UrlComponents.lpszPassword = url_info.szPassword;
    UrlComponents.lpszScheme = url_info.szScheme;
    UrlComponents.lpszUrlPath = url_info.szUrlPath;
    UrlComponents.lpszUserName = url_info.szUserName;

    UrlComponents.dwExtraInfoLength = 
        UrlComponents.dwHostNameLength = 
        UrlComponents.dwPasswordLength = 
        UrlComponents.dwSchemeLength = 
        UrlComponents.dwUrlPathLength = 
        UrlComponents.dwUserNameLength = 512;

    WinHttpCrackUrl(Url, 0, ICU_ESCAPE, &UrlComponents);
	
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    DWORD ContentLength = sizeof(ContentLength);
    DWORD FileSize, FileIndex = 0;

    // Specify an HTTP server.
    if (hSession)
		hConnect = WinHttpConnect( hSession,UrlComponents.lpszHostName, UrlComponents.nPort, 0);

    // Create an HTTP request handle. first query the size of the file
    if (hConnect){
        hRequest = WinHttpOpenRequest( hConnect, L"HEAD", UrlComponents.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &FileSize, &ContentLength, &FileIndex);
	WinHttpCloseHandle(hRequest);
    }

	// Create an HTTP request handle. download file
    hRequest = WinHttpOpenRequest(hConnect, L"GET", UrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
    bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    bResults = WinHttpReceiveResponse(hRequest, 0);

    // download file
    DWORD BUF_LEN = 1024, ReadedLen = 0;
    BYTE *pBuffer = NULL;
    pBuffer = new BYTE[BUF_LEN];

    HANDLE hFile = CreateFileW(FileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Keep checking for data until there is nothing left.
    do 
    {
        // Check for available data.
        dwSize = 0;
        if (!WinHttpQueryDataAvailable( hRequest, &dwSize)) 
        {
            printf( "Error %u in WinHttpQueryDataAvailable.\n",
                    GetLastError());
            break;
        }
        
         //No more available data.
        if (!dwSize)
            break;

        // Allocate space for the buffer.
        ZeroMemory(pBuffer, BUF_LEN);

		// 写入文件
        if (!WinHttpReadData( hRequest, (LPVOID)pBuffer, 
                              BUF_LEN, &dwDownloaded))
        {                                  
            printf( "Error %u in WinHttpReadData.\n", GetLastError());
        }
        else
        {
	    ReadedLen += dwDownloaded;

	    // write into file
	    WriteFile(hFile, pBuffer, dwDownloaded, &dwDownloaded, NULL);

	    // callback of the process
	    Func(FileSize, ReadedLen);
        }

        // This condition should never be reached since WinHttpQueryDataAvailable
        // reported that there are bits to read.
        if (!dwDownloaded)
            break;
            
    } while (FileSize > ReadedLen);

	// Free the memory allocated to the buffer.
	delete [] pBuffer;

    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}

void WinhttpUpload()
{
	return; 
}


//ensure the features of HTTP upload and download
int _tmain(int argc, _TCHAR* argv[])
{
    //need the minio IP Address and the path of file to write
    WinhttpDownload(L"http://127.0.0.1:9000/alexbucket/winhttp.md", L"./winhttp.md", &dcallback);
    printf("Press any key to continue...");
    _getch();
    return 0;
}
