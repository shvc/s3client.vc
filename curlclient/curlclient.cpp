// curlclient.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

using std::ios;
using std::fstream;

size_t libcurl_read_file(void *pBuffer, size_t size, size_t nmemb, void *hFile)
{
	DWORD dwNumberOfBytesRead = 0;
	BOOL bResult = ReadFile((HANDLE) hFile, pBuffer, size * nmemb, &dwNumberOfBytesRead, NULL);
	return dwNumberOfBytesRead;
}

void UploadFile(char *strURL, char *strFileName)
{
	CURL *curl;
	HANDLE hFile;
	CURLcode ccCurlResult = CURL_LAST;
	// curl_off_t 
	curl_off_t cotFileSize;
	
	LARGE_INTEGER liFileSize;
	// check parameters
	if( strURL == NULL || strlen(strURL) == 0 || strFileName == NULL || strlen(strFileName) == 0 )
	{
		return;
	}

	// create a handle to the file
	hFile = CreateFileA(strFileName, // file to open
			GENERIC_READ, // open for reading
			FILE_SHARE_READ, // share for reading
			NULL, // default security
			OPEN_EXISTING, // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL); // no attr. template

	if(hFile != INVALID_HANDLE_VALUE)
	{
		// global libcurl initialisation
		ccCurlResult = curl_global_init(CURL_GLOBAL_WIN32);
		if(ccCurlResult == 0)
		{
			// start libcurl easy session
			curl = curl_easy_init();
			if(curl)
			{
				// enable verbose operation
				curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
				// enable uploading
				curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
				
				// set http header
				struct curl_slist *headers = NULL;
				headers = curl_slist_append(headers, "Content-Type:application/octet-stream");
				headers = curl_slist_append(headers, "x-emc-namespace:ns1");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				
				// inform libcurl of the file's size
				GetFileSizeEx(hFile, &liFileSize);
				cotFileSize = liFileSize.QuadPart;
				curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, cotFileSize);

				// use custom read function callback read the file
				curl_easy_setopt(curl, CURLOPT_READFUNCTION, libcurl_read_file);
				// specify which file to upload
				curl_easy_setopt(curl, CURLOPT_READDATA, hFile);
				// specify destinaiton URL
				curl_easy_setopt(curl, CURLOPT_URL, strURL);
				// execute command
				ccCurlResult = curl_easy_perform(curl);

				curl_slist_free_all(headers);
				// end libcurl easy session
				curl_easy_cleanup(curl);
			}
		}
		// release file handle
		CloseHandle(hFile);
		// global libcurl cleanup
		curl_global_cleanup();
		if (ccCurlResult == CURLE_OK) {
			printf("File uploaded successfully.\n");
		} else {
			printf("File upload failed. Curl error: %d\n", ccCurlResult);
		}
	} else {
		printf("File upload failed! Could not open local file");
	}
}


size_t libcurl_read_stream(void *pBuffer, size_t size, size_t nmemb, void *stream)
{
	std::ifstream *is = (std::ifstream*)stream;
	is->read((char*)pBuffer, size * nmemb); 
	return is->gcount();
}

void UploadFileStream(char *strURL, char *strFileName)
{
	CURL *curl;
	curl_off_t cotFileSize;
	CURLcode ccCurlResult = CURL_LAST;
	// check parameters
	if( strURL == NULL || strlen(strURL) == 0 || strFileName == NULL || strlen(strFileName) == 0 )
	{
		return;
	}

	// ifstream: read file stream
	std::ifstream filestream(strFileName, std::ios::binary);

	filestream.seekg(0, std::ios::end);
	std::streampos FileLength = filestream.tellg();
	filestream.seekg(0, std::ios::beg);

	if(filestream.is_open())
	{
		// global libcurl initialisation
		ccCurlResult = curl_global_init(CURL_GLOBAL_WIN32);
		if(ccCurlResult == 0)
		{
			// start libcurl easy session
			curl = curl_easy_init();
			if(curl)
			{
				// enable verbose operation
				curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
				// enable uploading file
				curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE);
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

				// set http header
				struct curl_slist *headers = NULL;
				headers = curl_slist_append(headers, "Content-Type:application/octet-stream");
				headers = curl_slist_append(headers, "x-emc-namespace:ns1");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

				// inform libcurl of the file's size
				cotFileSize = FileLength;
				curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, cotFileSize);
				// use custom read function callback read the file
				curl_easy_setopt(curl, CURLOPT_READFUNCTION, libcurl_read_stream);
				// specify which file to upload
				curl_easy_setopt(curl, CURLOPT_READDATA, &filestream);
				// specify destinaiton URL
				curl_easy_setopt(curl, CURLOPT_URL, strURL);
				// execute command
				ccCurlResult = curl_easy_perform(curl);

				curl_slist_free_all(headers);
				// end libcurl easy session
				curl_easy_cleanup(curl);
			}
			// global libcurl cleanup
			curl_global_cleanup();
			if (ccCurlResult == CURLE_OK)
			{
				printf("File uploaded successfully.\n");
			} else {
				printf("File upload failed. Curl error: %d\n", ccCurlResult);
			}
		} else {
			//
		}

		filestream.close();
	} else {
		printf("File upload failed! Could not open local file");
	}
}


// upload data to ECS(default namespace: ns1)
void Upload(char *strURL, char *content, int contentLen)
{
	CURL *curl;
	CURLcode ccCurlResult = CURL_LAST;
	// check parameters
	if( strURL == NULL || strlen(strURL) == 0 || content == NULL || contentLen <= 0)
	{
		return;
	}

	// global libcurl initialisation
	ccCurlResult = curl_global_init(CURL_GLOBAL_WIN32);
	if(ccCurlResult == CURLE_OK)
	{
		// start libcurl easy session
		curl = curl_easy_init();
		if(curl)
		{
			// enable PUT operation
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
			// enable verbose operation
			curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);

			// set http header
			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Content-Type:application/octet-stream");
			headers = curl_slist_append(headers, "x-emc-namespace:ns1");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			// inform libcurl of the upload content 
			curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, content);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, contentLen);

			// specify upload destination full URL
			curl_easy_setopt(curl, CURLOPT_URL, strURL);

			// execute command
			ccCurlResult = curl_easy_perform(curl);

			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
		} else {
			// init curl error
		}
	}
	// global libcurl cleanup
	curl_global_cleanup();
	if (ccCurlResult == CURLE_OK)
	{
		printf("string uploaded successfully.\n");
	} else {
		printf("string upload failed. Curl error: %d\n", ccCurlResult);
	}

}


// 处理从服务器下载的数据, 并将数据写入流文件
size_t libcurl_write_stream(void *pBuffer, size_t size, size_t nmemb, void *stream)
{
    std::ofstream *is = (std::ofstream*)stream;
    is->write((char*)pBuffer, size*nmemb);
    return size*nmemb;
}

void DownloadFileStream(char *strURL, char *strFileName)
{
	CURL *curl;
	CURLcode ccCurlResult = CURL_LAST;

	// check parameters
	if(strURL == NULL || strlen(strURL) == 0) {
		return;
	}
	if(strFileName == NULL || strlen(strFileName) == 0) {
		return;
	}

	//std::ifstream filestream;
	std::fstream filestream(strFileName, fstream::out | ios::binary); 

	// global libcurl initialisation
	ccCurlResult = curl_global_init(CURL_GLOBAL_ALL);

	// start libcurl easy session
	curl = curl_easy_init();
	if(curl)
	{
		// enable verbose operation
		curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type:application/octet-stream");
		headers = curl_slist_append(headers, "x-emc-namespace: ns1");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_write_stream);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &filestream);

		curl_easy_setopt(curl, CURLOPT_URL, strURL);
		ccCurlResult = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		// end libcurl easy session
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return;
}


struct MemoryStruct {
  char *memory;
  size_t size;
};
// 处理从服务器下载的数据, 并将数据写入 buff
size_t libcurl_write(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) {
		/* out of memory! */ 
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
 
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
 
	return realsize;
}

// Download file to memory
size_t Download(char *strURL, char *buff)
{
	CURL *curl;
	CURLcode ccCurlResult = CURL_LAST;

	// check parameters
	if(strURL == NULL || strlen(strURL) == 0) {
		return;
	}

	struct MemoryStruct chunk;
	chunk.memory = buff;  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;            /* no data at this point */
	buff = malloc(1);


	// global libcurl initialisation
	ccCurlResult = curl_global_init(CURL_GLOBAL_ALL);

	// start libcurl easy session
	curl = curl_easy_init();
	if(curl)
	{
		// enable verbose operation
		curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		// set http header
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type:application/octet-stream");
		headers = curl_slist_append(headers, "x-emc-namespace: ns1");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// send all data to this function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, libcurl_write);
		// we pass our 'chunk' struct to the callback function
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
 		curl_easy_setopt(curl, CURLOPT_URL, strURL);
		ccCurlResult = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		// end libcurl easy session
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return chunk.size;
}

int _tmain(int argc, _TCHAR *argv[])
{
	// normal file upload
	// URL(http://ip:port/bucket/key), filename(local filename)
	//UploadFile("http://192.168.1.6:9000/open/test.txt", "C:\\test.txt");


	// stream file upload
	// URL(http://ip:port/bucket/key), filename(local filename)
	//UploadFileStream("http://192.168.1.6:9000/open/test2.txt", "open", "C:\\test2.txt");


	// upload data(string or binary)
	// URL(http://ip:port/bucket/key), contents, contents lenght
	Upload("http://192.168.55.2:9020/open/str01", "string\0contents 00002", 22);

	
	// stream file download
	// URL(http://ip:port/bucket/key), filename(local filename)
	DownloadFileStream("http://192.168.55.2:9020/open/str01", "test.txt");


	char *buff;
	// download file to memory buff
	size_t buff_len = Download("http://192.168.55.2:9020/open/str01", buff)
	
	printf("Press any key to continue...");
	_getch();
	return 0;
}
