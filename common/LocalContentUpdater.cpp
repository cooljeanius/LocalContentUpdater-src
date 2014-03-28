//
// LocalContentUpdater.cpp
// Utility for managing the UseNetwork flag in SWF files.
//
// On Windows: be sure to link with setargv.obj (part of MSVC) for path wildcard expansion
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>
#include <functional>

#include "version.h"
#include "readDir.h"
#include "SwfBuffer.h"

using namespace std;

enum AppMode { modeUnknown = 0, modeVersion, modeCheck, modeAdd, modeRemove };

void usage()
{
	fprintf( stderr, "usage: LocalContentUpdater [-r [-s]] { -v | -c | -a | -x } [--] swf-file [...]\n" );
	fprintf( stderr, "  -r: recursive operation; specify files and/or directories\n" );
	fprintf( stderr, "  -s: ignore all except .swf files\n" );
	fprintf( stderr, "  -v: display version and exit\n" );
	fprintf( stderr, "  -c: check for network privileges\n" );
	fprintf( stderr, "  -a: add network privileges if not present\n" );
	fprintf( stderr, "  -x: remove network privileges if present\n" );
	fprintf( stderr, "  --: end of flags (allows filenames that start with dashes)\n" );
}


// NotSwfPred: STL predicate helper that returns true if its argument doesn't end in ".swf".
//
struct NotSwfPred : public unary_function< string, bool >
{
	bool operator()( string path )
	{
		return ( path.size() < 5 || path.substr( path.size() - 4 ).compare( ".swf" ) != 0 );
	}
};

// getFiles: expands rPaths, which may contain file and directory paths, into rFilesOut,
//   which will contain only file paths, taken recursively from all the files and
//   directories of rPaths.
// Uses readDir and isDir helpers from readDir.h.
// Calls itself recursively for directory paths.
//
void getFiles( const vector< string > &rPaths, vector< string > &rFilesOut )
{
	for ( vector< string >::const_iterator i = rPaths.begin(); i != rPaths.end(); i++ )
	{
		if ( isDir( *i ) )
		{
			vector< string > dirFiles;
			readDir( *i, dirFiles, true );
			getFiles( dirFiles, rFilesOut );
		}
		else
		{
			rFilesOut.push_back( *i );
		}
	}
}


////////////////////////////////////////////////////
// MAIN ROUTINE

int main( int argc, char *argv[] )
{
	// ***
	// *** Extract arguments
	// ***

	int curArg = 1;
	AppMode mode = modeUnknown;
	bool modeRecursive = false;
	bool modeSwf = false;

	// Extract flags - all arguments beginning with dashes
	while ( curArg < argc
		    && argv[ curArg ][ 0 ] == '-'
			&& strcmp( argv[ curArg ], "--" ) != 0 )
	{
		// Mode flags - must have exactly one
		AppMode newMode = modeUnknown;
		if ( !strcmp( argv[ curArg ], "-v" ) ) newMode = modeVersion;
		else if ( !strcmp( argv[ curArg ], "-c" ) ) newMode = modeCheck;
		else if ( !strcmp( argv[ curArg ], "-a" ) ) newMode = modeAdd;
		else if ( !strcmp( argv[ curArg ], "-x" ) ) newMode = modeRemove;

		if ( newMode != modeUnknown )
		{
			if ( mode != modeUnknown )
			{
				// More than one mode flag
				usage();
				return 1;
			}
			mode = newMode;
		}

		// Non-exclusive flags
		else
		{
			if ( !strcmp( argv[ curArg ], "-r" ) ) modeRecursive = true;
			else if ( !strcmp( argv[ curArg ], "-s" ) ) modeSwf = true;
			else
			{
				// Unrecognized flag
				usage();
				return 1;
			}
		}

		curArg++;
	}

	// Version mode: print version and quit.
	if ( mode == modeVersion )
	{
		printf( "LocalContentUpdater version %s\n", APPVERSION );
		return 0;
	}

	if ( mode == modeUnknown || curArg >= argc )
	{
		// No mode specified, or no files specified
		usage();
		return 1;
	}

	if ( modeSwf && !modeRecursive )
	{
		// -s without -r
		usage();
		return 1;
	}

	// Treat all remaining arguments as paths to operate on
	vector< string > paths;
	while ( curArg < argc )
	{
		paths.push_back( argv[ curArg++ ] );
	}

	// ***
	// *** Expand path arguments
	// ***

	// If -r was specified, expand any directory paths into file paths
	vector< string > *pFiles;
	vector< string > foundFiles;
	if ( modeRecursive )
	{
		getFiles( paths, foundFiles );
		pFiles = &foundFiles;
	}
	else
	{
		pFiles = &paths;
	}

	// If -s was specified, delete all paths that don't end in .swf
	if ( modeSwf )
	{
		// If this code looks weird, thank STL.
		// See http://www.sgi.com/tech/stl/remove_if.html
		pFiles->erase( remove_if( pFiles->begin(), pFiles->end(), NotSwfPred() ), pFiles->end() );
	}

	// Display number of paths
	printf( "Operating on %d %s.\n", pFiles->size(), ( pFiles->size() == 1 ? "file" : "files" ) );

	// ***
	// *** Operate on paths
	// ***

	for ( vector< string >::const_iterator i = pFiles->begin(); i != pFiles->end(); i++ )
	{
		string swfPath = *i;

		// Open the file
		FILE *fp = fopen( swfPath.c_str(), "rb" );
		if ( !fp )
		{
			fprintf( stderr, "can't open '%s': %s\n", swfPath.c_str(), strerror( errno ) );
			continue;
		}

		// Provide the file as input to a SwfBuffer
		SwfBuffer swfBuf;
		bool ok = swfBuf.Read( fp, swfPath.c_str() );
		fclose( fp );
		if ( !ok ) continue;

		// Initialize the SwfBuffer parser
		ok = swfBuf.ParsePastHeaderAndDecompress();
		if ( !ok ) continue;

		// Ask the SwfBuffer to perform the requested task
		// This will produce messages on stdout and/or stderr
		switch ( mode )
		{
			case modeCheck: ok = swfBuf.Check(); break;
			case modeAdd: ok = swfBuf.Add(); break;
			case modeRemove: ok = swfBuf.Remove(); break;
		}
		if ( !ok ) continue;

		// If the task caused the file's contents to change, overwrite the original file
		if ( swfBuf.HasChanges() )
		{
			fp = fopen( swfPath.c_str(), "wb" );
			if ( !fp )
			{
				fprintf( stderr, "can't open '%s': %s\n", swfPath.c_str(), strerror( errno ) );
				continue;
			}

			swfBuf.Write( fp );
			fclose( fp );

			printf( "%s", swfBuf.GetSuccessMsg() );
		}
	}

	return 0;
}


// End of file.
