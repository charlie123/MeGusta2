// by s0beit

#ifndef __APPTOOLS_HEADER__
#define __APPTOOLS_HEADER__

#include <windows.h>
#include <string>
#include <vector>

using namespace std;

class CAppTools
{
public:
    string                GetFileExtension( string file );
    string                GetAfterLast( string haystack, string needle );
    string                GetToLast( string haystack, string needle );
    vector< string >    Split_Chr( string &S, CHAR D );

    char*                GetDirectoryFileA( char *szFile );
    wchar_t*            GetDirectoryFileW( wchar_t *szFile );
    vector< string >    GetFilesInPath( string Path );
    void                AddToLogFileA( char *szFile, char *szLog, ... );
    void                AddToLogFileW( wchar_t *szFile, wchar_t *szLog, ... );
    void                BaseUponModule( HMODULE hModule );

    HMODULE                GetLocalModule(){ return m_LocalModule; }

    HMODULE                m_LocalModule;
    char                m_LocalDirectory[ 320 ];
};

extern CAppTools g_Logging;

#endif  