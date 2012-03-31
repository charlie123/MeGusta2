#include "stdafx.h"

#define PSAPI_VERSION 1
#include <psapi.h>

using namespace System;
using namespace System::Diagnostics;

HANDLE hFileMapping = NULL;
HANDLE hFile = NULL;

VOID UpdateRemoteFAL() {
#if fAIl
    PVOID  pBaseAddress = NULL;
    DWORD  dwRawData = 0;
    PVOID  pEntryPoint = NULL;
    PVOID  pCodeStart = NULL;
    PVOID  pCodeEnd = NULL;    
    SIZE_T dwCodeSize = 0;

	cli::array<Process^>^ processes = Process::GetProcesses();

	for each (Process^ process in processes) {
		String^ module = process->MainModule->FileName;
		IntPtr ipFileName = Runtime::InteropServices::Marshal::StringToHGlobalUni(module);
		wchar_t* szFileName = (wchar_t*)ipFileName.ToPointer();

		//ProcessFAL(szFileName);

		Runtime::InteropServices::Marshal::FreeHGlobal(ipFileName);
	}

	DWORD dwProcessIds[32768];
	DWORD numProcesses;
	DWORD numBytesReturned;
    EnumProcesses(dwProcessIds, 32768, &numBytesReturned);

	numProcesses = numBytesReturned / sizeof(DWORD);

	for (int i = 0; i < numProcesses; i++) {       
		DWORD dwPid = dwProcessIds[i];
		TCHAR szFileName[MAX_PATH * 2];

		if (dwPid <= 4) {
			continue;
		}

		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

		if (hProc) {
			memset(szFileName, 0, sizeof(szFileName));

			if (GetModuleFileNameEx(hProc, NULL, szFileName, sizeof(szFileName))) {
				/*FILE *pFile = NULL;
				_wfopen_s( &pFile, szFileName, L"rb" );
				fseek( pFile, 0, SEEK_END );
				int size = ftell( pFile );
				fclose( pFile );*/

				OutputDebugString(szFileName);

				//GatherDiskImageInformation( &szFileName, pBaseAddress, dwRawData,
				//	pEntryPoint, pCodeStart, dwCodeSize, pCodeEnd );

				

				if( NULL != pBaseAddress ) { UnmapViewOfFile( pBaseAddress ); }
				if( NULL != hFileMapping ) { CloseHandle( hFileMapping ); }
			} else {
				DWORD error = GetLastError();
				char szError[128];
				_snprintf(szError, 128, "%x", error);

				OutputDebugStringA(szError);
			}
		} else {
				DWORD error = GetLastError();
				char szError[128];
				_snprintf(szError, 128, "%x", error);

				OutputDebugStringA(szError);
			}

		CloseHandle(hProc);
	}
#endif
}

#pragma unmanaged
VOID GatherDiskImageInformation( PTCHAR szFilename, PVOID& pBaseAddress, DWORD& dwRawData,
        PVOID& pEntryPoint, PVOID& pCodeStart, SIZE_T& dwCodeSize, PVOID& pCodeEnd )
{
    //const UINT PATH_SIZE = 2 * MAX_PATH;
    //TCHAR szFilename[ PATH_SIZE ] = { 0 };

    /////////////////////////////////////////////////
    /////////////////////////////////////////////////
    /*if( 0 == GetModuleFileName( NULL, szFilename, PATH_SIZE ) )
    {
        return;
    }*/

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    hFile = CreateFile( szFilename, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    hFileMapping = CreateFileMapping( hFile, NULL,
        PAGE_READONLY, 0, 0, NULL );
    if ( NULL == hFileMapping )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    pBaseAddress = MapViewOfFile( hFileMapping,
        FILE_MAP_READ, 0, 0, 0 );
    if ( NULL == pBaseAddress )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PIMAGE_DOS_HEADER pDOSHeader = NULL;
    pDOSHeader = static_cast<PIMAGE_DOS_HEADER>( pBaseAddress );
    if( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PIMAGE_NT_HEADERS pNTHeader = NULL;
    pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
        (PBYTE)pBaseAddress + pDOSHeader->e_lfanew );
    if( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PIMAGE_FILE_HEADER pFileHeader = NULL;
    pFileHeader = reinterpret_cast<PIMAGE_FILE_HEADER>(
        (PBYTE)&pNTHeader->FileHeader );

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PIMAGE_OPTIONAL_HEADER pOptionalHeader = NULL;
    pOptionalHeader = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(
        (PBYTE)&pNTHeader->OptionalHeader );

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    if( IMAGE_NT_OPTIONAL_HDR32_MAGIC != pNTHeader->OptionalHeader.Magic )
    {
        return;
    }

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    PIMAGE_SECTION_HEADER pSectionHeader = NULL;
    pSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(
        (PBYTE)&pNTHeader->OptionalHeader +
        pNTHeader->FileHeader.SizeOfOptionalHeader );

    /////////////////////////////////////////////////////////////
    DWORD dwEntryPoint = pNTHeader->OptionalHeader.AddressOfEntryPoint;
    UINT nSectionCount = pNTHeader->FileHeader.NumberOfSections;

    for( UINT i = 0; i < nSectionCount; i++ )
    {
        // When we find a Section such that
        //   Section Start <= Entry Point <= Section End,
        //   we have found the .TEXT Section
        if( pSectionHeader->VirtualAddress <= dwEntryPoint &&
            dwEntryPoint < pSectionHeader->VirtualAddress +
                           pSectionHeader->Misc.VirtualSize )
        { break; }
        
        pSectionHeader++;
    }
    
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    // Use this when probing On Disk. It is where things
    //   are on disk - not where they will be in memory            
    dwRawData = pSectionHeader->PointerToRawData;
       
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    // Use this when probing On Disk. It is where things
    //   are on disk - not where they will be in memory
    pCodeStart = (PVOID)((PBYTE)pBaseAddress +
        pSectionHeader->PointerToRawData );

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    pEntryPoint = (PVOID)(((PBYTE)pBaseAddress) + dwEntryPoint);

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    dwCodeSize = pSectionHeader->Misc.VirtualSize;

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    pCodeEnd = (PVOID)((PBYTE)pCodeStart +
        pSectionHeader->Misc.VirtualSize );
}