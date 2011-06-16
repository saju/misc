/**
* Create sparse files on windows platforms. You need Visual Studio to compile this. 
* Specifically support for __int64 only seems to be in VS.
*
* Public domain. No Warranties explicit or implied.
* 
* saju.pillai@gmail.com
*/

#include <Windows.h>
#include <stdio.h>

void usage() {
	wprintf(L"sparse -loc <path to sparse file> -size <xxB|xxK|xxM|xxG>\n"
		L"where xxB is file size in Bytes, xxK in KBs, xxM in MBs and xxG in GB\n"
		L"xx is an integer - decimals are not accepted. Eg invocations:\n"
		L"sparse -loc C:\\foo.txt -size 20M\n"
		L"sparse -loc test.file -size 1G\n");
	exit(1);
}

void dump_error(wchar_t *msg) {
	wchar_t *msgbuff;

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgbuff, 0, NULL);
	wprintf(L"%sError: %s\n", msg == NULL ? L"" : msg, msgbuff);
}

__int64 convert_to_bytes(wchar_t *size) {
	int len;
	__int64 b = _wtoi64(size);
	if (!b) 
		return 0;
	len = wcslen(size);
	if (size[len-1] == L'G') 
		return b * 1024 * 1024 * 1024;
	else if (size[len-1] == L'M')
		return b * 1024 * 1024;
	else if (size[len-1] == L'K')
		return b * 1024;
	else if (size[len-1] == L'B')
		return b;
	else
		return 0;
}

int wmain(int argc, wchar_t **argv) {
	LARGE_INTEGER bytes;
	int i;
	wchar_t *loc=NULL, *size=NULL;
	HANDLE fd;
	DWORD volattrs = 0, discard = 0;

	for (i = 1; i < argc; i++) {
		if (!wcscmp(L"-loc", argv[i]))
			loc = argv[++i];
		else if (!wcscmp(L"-size", argv[i]))
			size = argv[++i];
		else {
			usage();
		}
	}

	if (!loc || !size)
		usage();
	
	bytes.QuadPart = convert_to_bytes(size);
	if (!bytes.QuadPart) {
		wprintf(L"Unable to figure out what size you meant by \"%s\". Please read the usage info\n\n", size);
		usage();
	}
	wprintf(L"Trying to create sparse file %s of size %s (%I64d bytes)\n", loc, size, bytes.QuadPart);
	
	fd = CreateFile(loc, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL); 
	if (fd == INVALID_HANDLE_VALUE) {
		dump_error(L"Could not create file");
		exit(-1);
	}

	/* check for filesystem support for sparse files */
	if (!GetVolumeInformationByHandleW(fd, NULL, 0, NULL, NULL, &volattrs, NULL, MAX_PATH)) {
		dump_error(L"Failed to get volume information ");
		CloseHandle(fd);
		DeleteFile(loc);
		exit(-1);
	}
	if (!(volattrs & FILE_SUPPORTS_SPARSE_FILES)) {
		wprintf(L"This filesystem does not support sparse files ");
		CloseHandle(fd);
		DeleteFile(loc);
		exit(-1);
	}

	if (!DeviceIoControl(fd, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &discard, NULL)) {
		dump_error(L"Could not make the file \"sparse\" ");
	}
	if (!SetFilePointerEx(fd, bytes, NULL, FILE_BEGIN)) {
		dump_error(L"Could not set file size ");
		CloseHandle(fd);
		DeleteFile(loc);
		exit(-1);
	}
	SetEndOfFile(fd);
	CloseHandle(fd);
	wprintf(L"Done.\n");
	return 0;
}