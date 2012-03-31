#include "stdafx.h"
#include <windows.h>
#include "AppTools.h"

#pragma unmanaged
CAppTools g_Logging;

string CAppTools::GetFileExtension( string file )
{
    return GetAfterLast( file, "." );
}

string CAppTools::GetAfterLast( string haystack, string needle )
{
    return haystack.substr( haystack.find_last_of( needle ) + needle.length() );
}

string CAppTools::GetToLast( string haystack, string needle )
{
    return haystack.substr( 0, haystack.find_last_of( needle ) );
}

vector< string > CAppTools::Split_Chr( string &S, CHAR D )
{
    CHAR                SZ[2] = { D, 0 };
    string                SS = S;
    vector< string >    ReturnStringPool;

    int Found = static_cast< int >( SS.find_first_of( SZ ) );

    while( Found != string::npos )
    {
        if( Found > 0 )
        {
            ReturnStringPool.push_back( SS.substr( 0, Found ) );
        }

        SS = SS.substr( Found + 1 );

        Found = static_cast< int >( SS.find_first_of( SZ ) );
    }

    if( SS.length() > 0 )
    {
        ReturnStringPool.push_back( SS );
    }

    return ReturnStringPool;
}

char* CAppTools::GetDirectoryFileA( char *szFile )
{
    static char path[ 320 ];
    strcpy( path, m_LocalDirectory );
    strcat( path, szFile );

    return path;
}

wchar_t* CAppTools::GetDirectoryFileW( wchar_t *szFile )
{
    char szFilename[ MAX_PATH ] = { 0 };

    wcstombs( szFilename, szFile, MAX_PATH );

    char *szDirectoryFile = GetDirectoryFileA( szFilename );

    static wchar_t szDirectoryFileW[ MAX_PATH ] = { 0 };

    mbstowcs( szDirectoryFileW, szDirectoryFile, MAX_PATH );

    return szDirectoryFileW;
}

vector< string > CAppTools::GetFilesInPath( string Path )
{
    vector< string > ReturnVector;

    WIN32_FIND_DATAA WF;

    HANDLE hHandle = FindFirstFileA( Path.c_str(), &WF );

    if( hHandle != INVALID_HANDLE_VALUE )
    {
        if( _stricmp( WF.cFileName, ".." ) && _stricmp( WF.cFileName, "." ) )
        {
            ReturnVector.push_back( WF.cFileName );
        }

        while( FindNextFileA( hHandle, &WF ) == TRUE )
        {
            if( _stricmp( WF.cFileName, ".." ) && _stricmp( WF.cFileName, "." ) )
            {
                ReturnVector.push_back( WF.cFileName );
            }
        }

        FindClose( hHandle );
    }

    return ReturnVector;
}

void CAppTools::AddToLogFileA( char *szFile, char *szLog, ... )
{
	return;
    va_list va_alist;
    
    char logbuf[ 1024 ] = { 0 };
    
    FILE * fp;

    va_start( va_alist, szLog );
    
    _vsnprintf( logbuf + strlen( logbuf ), 
        sizeof( logbuf ) - strlen( logbuf ), 
        szLog, va_alist);

    va_end( va_alist );

    if ( ( fp = fopen ( GetDirectoryFileA( szFile ), "a" ) ) != NULL )
    {
        fprintf( fp, "%s\n", logbuf );
        fclose( fp );
    }
}

void CAppTools::AddToLogFileW( wchar_t *szFile, wchar_t *szLog, ... )
{
	return;
    va_list va_alist;

    wchar_t logbuf[ 1024 ] = { 0 };
    
    FILE * fp;

    va_start( va_alist, szLog );

    _vsnwprintf( logbuf + wcslen( logbuf ), 
        sizeof( logbuf ) - wcslen( logbuf ), 
        szLog, va_alist);
    
    va_end( va_alist );

    if ( ( fp = _wfopen( GetDirectoryFileW( szFile ), L"a" ) ) != NULL )
    {
        fwprintf( fp, L"%s\n", logbuf );
        fclose( fp );
    }
}

void CAppTools::BaseUponModule( HMODULE hModule )
{
    m_LocalModule = hModule;

    memset( m_LocalDirectory, 0, MAX_PATH );

    MEMORY_BASIC_INFORMATION mbi;

    if( VirtualQuery( ( LPCVOID )hModule, &mbi, sizeof( mbi ) ) > 0 )
    {
        if( mbi.Type == MEM_IMAGE )
        {
            //Normal mapped library

            if( GetModuleFileNameA( hModule, m_LocalDirectory, MAX_PATH ) != 0 )
            {
                for( int i = ( int )strlen( m_LocalDirectory ); i > 0; i-- )
                {
                    if( m_LocalDirectory[ i ] == '\\' )
                    {
                        m_LocalDirectory[ i + 1 ] = 0;

                        break;
                    }
                }
            }
        }
        else
        {
            //Manually mapped

            BaseUponModule( GetModuleHandleW( NULL ) );
        }
    }
    else
    {
        //BORKED

        BaseUponModule( GetModuleHandleW( NULL ) );
    }
}