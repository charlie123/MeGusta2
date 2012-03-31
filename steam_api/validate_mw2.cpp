#include "stdafx.h"

// code mainly taken from http://www.codeproject.com/KB/cpp/VerifyIntegrity.aspx
// released under the CPOL: http://www.codeproject.com/info/cpol10.aspx
#pragma unmanaged 

#define USE_HARDCODED_HASH 0
#define SHOW_DEBUG_HASH 0
#define PERFORM_MEMORY_DUMP1 0

#if USE_HARDCODED_HASH
BYTE hardcodedHash[64] = { 0x1D, 0x8E, 0x19, 0x01, 0x1D, 0x81, 0x0C, 0x89, 0x91, 0x02, 0xED, 0xAC, 0x6D, 0x5B, 0x59, 0x03, 0x51, 0xC0, 0x91, 0x17, 0x9F, 0x33, 0x6F, 0x79, 0x03, 0x92, 0xC8, 0xDE, 0x98, 0xFC, 0xE5, 0x19, 0x53, 0xFD, 0x08, 0x1C, 0x57, 0xD3, 0x7F, 0xB1, 0xA2, 0x99, 0x7A, 0xF4, 0x1F, 0xC4, 0xF7, 0x0D, 0x54, 0x06, 0xD1, 0x13, 0xC1, 0x07, 0x4E, 0x6E, 0x29, 0xA2, 0x44, 0x24, 0x14, 0x0A, 0x64, 0xDD };
#else
BYTE hardcodedHash[64] = { 0x00, 0x00, 0x00, 0x00, 0x28, 0x64, 0x21, 0x9D, 0x2C, 0x6E, 0x39, 0x2E, 0x41, 0xC2, 0x42, 0xFB, 0xAF, 0xE8, 0x09, 0xAB, 0x2E, 0x99, 0xBA, 0xD4, 0xFC, 0x1E, 0x52, 0x55, 0x08, 0xB9, 0xCE, 0xCC, 0xE3, 0x51, 0xEC, 0x16, 0xBE, 0x83, 0x1E, 0x43, 0xAD, 0xB2, 0xE5, 0xA7, 0xE1, 0x79, 0x4E, 0x47, 0xD2, 0xD2, 0xF9, 0xED, 0xC9, 0xC1, 0x33, 0xD1, 0xE4, 0x7B, 0xA9, 0x52, 0x07, 0xDD, 0x42, 0x33 };
#endif

const char* whyDidFalFail = "This string is a tribute to the classic code that used to be housed in validate_mw2.cpp. Originally 'released' with the 1.3.37a release, it got enabled less than a week after the massive server overload from the release ended. It has been through tough times, with some triggers being misplaced, and many code even having been commented out for over a month. However, the people on the primary 'enemy' forum apparently were too stupid to know assembly to actually properly patch this code out. Now, when finally people started to patch the code that used to sit below this comment, this code has been defined useless. More changes to it will be patched very quicky, and as such that would make no sense. In fact, I'm cancelling the 'active cheating measures' initiative -- and I hope MaxDamage doesn't read this message as he won't like it. Anyway, you've done good, validation_mw2.cpp (aka FALRunner), but it's time to go rest now. Let the knowledge done during your development might be able to do some good later on. Amen.";
const char* falMessage = "VALIDATE_MW2.CPP - 2010-07-11 TO 2010-09-15. NEVER HAS A FAL BEEN MORE USEFUL THAN YOU. More history could be written in these two strings, but I don't think it matters now -- check future releases for more hidden strings though.";

using namespace std;

#pragma managed
using namespace System;

void PrintFALTributes()
{
	cli::array<String^>^ fals = (gcnew System::String(whyDidFalFail))->Split(gcnew cli::array<String^>(1) { ". " }, StringSplitOptions::None);

	for each (String^ fal in fals)
	{
		IntPtr data = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fal + ".");
		char* cha = (char*)data.ToPointer();
		OutputDebugStringA(cha);
		Runtime::InteropServices::Marshal::FreeHGlobal(data);
	}

	OutputDebugStringA(falMessage);
}
#pragma unmanaged

/////////////////////////////////////////////////
// Hash Specific Variables
BYTE cbInitiaHash[ 64 ];
BYTE cbMemoryHash[ 64 ];

VOID  GatherMemoryImageInformation( HMODULE& hModule,
            PVOID& pVirtualAddress, PVOID& pEntryPoint,
            PVOID& pCodeStart, SIZE_T& dwCodeSize, PVOID& pCodeEnd );

BOOL CalculateHash( 
			PVOID pMessage, SIZE_T nMessageSize,
            PBYTE pcbHashBuffer, SIZE_T nHashBufferSize );

VOID UpdateValidation(BOOL isInitial);

#if SHOW_DEBUG_HASH
VOID OutputHash(BYTE* hash, size_t size);
//VOID DumpHashInformation( HashTransformation& hash, PBYTE pcbHash, SIZE_T nHashSize );
#endif

VOID InitializeValidation() {
	UpdateValidation(TRUE);
}

DWORD* projection;
DWORD* initProjection;
DWORD newLoc = 0;
int i = 0;

int mmemcmp(size_t count, const void* buf1, const void* buf2) {
	return memcmp(buf1, buf2, count);
}

void DumpToFile(void* start, size_t size) {
	char name[256];
	sprintf(name, "0x%x.bin", (DWORD)start);
	FILE* file = fopen(name, "wb");
	fwrite(start, 1, size, file);
	fclose(file);
}

BOOL didInitialize = FALSE;
BOOL didTribute = FALSE;

VOID UpdateValidation(BOOL isInitial) {
#if DEDICATED
	return;
#endif
	if (didInitialize && !didTribute)
	{
		PrintFALTributes();

		didTribute = TRUE;
	}

	didInitialize = TRUE;

	return;

#if USE_VALIDATION
	isInitial = false;

	if (!didInitialize) {
		isInitial = true;
		didInitialize = true;
	}

	/////////////////////////////////////////////////
    // In-Memory Variables
    HMODULE hModule = NULL;
    PVOID   pVirtualAddress = NULL;  
    PVOID   pMemoryEntryPoint = NULL;
    PVOID   pMemoryCodeStart = NULL;
    PVOID   pMemoryCodeEnd = NULL;     
    SIZE_T  dwMemoryCodeSize = 0;

	GatherMemoryImageInformation( hModule, pVirtualAddress,
    pMemoryEntryPoint, pMemoryCodeStart,
	dwMemoryCodeSize, pMemoryCodeEnd );

	if (isInitial) {
		memcpy( cbInitiaHash, hardcodedHash, sizeof( cbInitiaHash ) );

#if PERFORM_MEMORY_DUMP1
		DumpToFile(pMemoryCodeStart, dwMemoryCodeSize);
#endif

		if (hardcodedHash[0] == 0x00) {
			CalculateHash( pMemoryCodeStart, dwMemoryCodeSize,
				cbInitiaHash, sizeof( cbInitiaHash ) );
		}

#if SHOW_DEBUG_HASH
		OutputHash(cbInitiaHash, sizeof(cbInitiaHash));
#endif
	} else {
		CalculateHash( pMemoryCodeStart, dwMemoryCodeSize,
			cbMemoryHash, sizeof( cbMemoryHash ) );

#if SHOW_DEBUG_HASH
		OutputHash(cbMemoryHash, sizeof(cbInitiaHash));
#endif

		if (mmemcmp(sizeof(cbInitiaHash), cbMemoryHash, cbInitiaHash)) {
		//if (true) {
#if SHOW_DEBUG_HASH
			OutputDebugString(_T("... hack detected"));
#endif
			// code is different, terminate the process
			//TerminateProcess(GetCurrentProcess(), 0);

			__asm {
				mov projection, esp
			}

			//DWORD oldProtect;
			//VirtualProtect(projection, 256 * 4, PAGE_EXECUTE_READWRITE, &oldProtect);

			// changed to 64, 256 caused obvious crash in some code
			for (i = 0; i < 64; i++) {
				//__asm mov [projection], 0xCDCDCDCD
				// oddfuscated code to kill damacc signatures. fuck him
				*projection = 0x00000001;

				projection += 1;
				projection -= 3;
				projection += 1;
				projection += 2;

				*(projection - 1) = 0x00000000;
			}

			// jump to a null pointer to complicate malicious debugging
			__asm {
				jmp newLoc
			}
		}
	}
#endif
}

VOID LogDebugMessage(LPTSTR lpMessage) {
	// currently use OutputDebugString
	OutputDebugString(lpMessage);
}

#if SHOW_DEBUG_HASH
VOID OutputHash(BYTE* hash, size_t size) {
	std::string string;
	char buffer[3];

	for (int i = 0; i < size; i++) {
		sprintf(buffer, "%02X", hash[i]);
		string.append(buffer);
	}

	OutputDebugStringA(string.c_str());
}
#endif

VOID GatherMemoryImageInformation( HMODULE& hModule, PVOID& pVirtualAddress,
        PVOID& pEntryPoint, PVOID& pCodeStart, SIZE_T& dwCodeSize, PVOID& pCodeEnd )
{
    const UINT PATH_SIZE = 2 * MAX_PATH;
    TCHAR szFilename[ PATH_SIZE ] = { 0 };    

    __try {

        /////////////////////////////////////////////////
        /////////////////////////////////////////////////
        if( 0 == GetModuleFileName( NULL, szFilename, PATH_SIZE ) )
        {
            __leave;
        }

        hModule = GetModuleHandle( szFilename );

        if( NULL == hModule )
        {
            __leave;
        }

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_DOS_HEADER pDOSHeader = NULL;
        pDOSHeader = static_cast<PIMAGE_DOS_HEADER>( (PVOID)hModule );
        if( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )
        {
            __leave;
        }

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_NT_HEADERS pNTHeader = NULL;
        pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
            (PBYTE)hModule + pDOSHeader->e_lfanew );
        if( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        {
            __leave;
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
        if( IMAGE_NT_OPTIONAL_HDR32_MAGIC !=
            pNTHeader->OptionalHeader.Magic )
        {
            __leave;
        }        

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        PIMAGE_SECTION_HEADER pSectionHeader = NULL;
        pSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(
            (PBYTE)&pNTHeader->OptionalHeader +
            pNTHeader->FileHeader.SizeOfOptionalHeader );

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        DWORD dwEntryPoint = pNTHeader->OptionalHeader.AddressOfEntryPoint;
        UINT nSectionCount = pNTHeader->FileHeader.NumberOfSections;

        for( UINT i = 0; i < nSectionCount; i++ )
        {
            // When we find a Section such that
            //  Section Start <= Entry Point <= Section End,
            //  we have found the .TEXT Section
            if( pSectionHeader->VirtualAddress <= dwEntryPoint &&
                dwEntryPoint < pSectionHeader->VirtualAddress +
                               pSectionHeader->Misc.VirtualSize )
            { break; }

            pSectionHeader++;
        }

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        pVirtualAddress = (PVOID)(pSectionHeader->VirtualAddress);

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////
        pEntryPoint = (PVOID)(((PBYTE)hModule) + dwEntryPoint);

        dwCodeSize = pSectionHeader->Misc.VirtualSize;

        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////        
        pCodeStart = (PVOID)(((PBYTE)hModule) +
                     (SIZE_T)((PBYTE)pVirtualAddress) );
                     
        /////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////                      
        pCodeEnd = (PVOID)((PBYTE)pCodeStart + dwCodeSize );                      
}

    __except( EXCEPTION_EXECUTE_HANDLER ) {
         //OutputDebugString(_T("Caught Exception"));
    }
}

BOOL CalculateHash( 
            PVOID pMessage, SIZE_T nMessageSize,
            PBYTE pcbHashBuffer, SIZE_T nHashBufferSize )
{
	ZeroMemory( pcbHashBuffer, nHashBufferSize );

    if( nHashBufferSize < 64 )
    {
        ZeroMemory( pcbHashBuffer, nHashBufferSize );
        return FALSE;
    }
#if 0
	hash_state hashState;
	sha512_init(&hashState);
	sha512_process(&hashState, (PBYTE)pMessage, nMessageSize);
	sha512_done(&hashState, pcbHashBuffer);
#endif
    //hash.Update( (const PBYTE)pMessage, nMessageSize );
    //hash.Final( pcbHashBuffer );

    return TRUE;
}