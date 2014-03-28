//
// readDir.cpp
// Cross-platform directory utilities.
//

#include "readDir.h"
#include <algorithm>

using namespace std;


#ifdef _WIN32

# include <windows.h>

# define PATH_SEP_CHAR '\\'

#else /* ! _WIN32: */

# include <dirent.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>

# define PATH_SEP_CHAR '/'

#endif /* _WIN32 */


// readDir: find all the files and directories in a directory
// Output is sorted (by char codes), "." and ".." are omitted
//
void readDir(const string &rDirPath, vector< string > &rNamesOut,
			 bool fullPaths)
{
	// Sane dir path to open
	string dirPath = (rDirPath.empty() ? "." : rDirPath);

	// Dir path with guaranteed trailing slash
	string slashPath(dirPath);
	if (slashPath[(slashPath.size() - 1)] != PATH_SEP_CHAR) {
		slashPath += PATH_SEP_CHAR;
	}

#ifdef _WIN32

	// Build search expression for FindFirstFile and FindNextFile
	string searchExp = (slashPath + "*");

	// Iterate over files with FindFirstFile and FindNextFile
	HANDLE hSearch;
	WIN32_FIND_DATA fileInfo;
	BOOL rc = TRUE;
	for (hSearch = ::FindFirstFile(searchExp.c_str(), &fileInfo);
		 hSearch != INVALID_HANDLE_VALUE && rc;
		 rc = ::FindNextFile( hSearch, &fileInfo)) {
		// Is it something other than "." or ".."?
		string filename(fileInfo.cFileName);
		if (filename.compare(".") && filename.compare("..")) {
			rNamesOut.push_back(filename);
		}
	}

	// Done
	if (hSearch != INVALID_HANDLE_VALUE) {
		::FindClose(hSearch);
	}

#else /* ! _WIN32: */

	// Open dir
	DIR *pDir = ::opendir(dirPath.c_str());
	if (pDir) {
		// Iterate over files with readdir:
		for (dirent *pEnt = ::readdir(pDir); pEnt; (pEnt = ::readdir(pDir))) {
			// Is it something other than "." or ".."?
			string filename(pEnt->d_name);
			if (filename.compare(".") && filename.compare("..")) {
				rNamesOut.push_back(filename);
			}
		}

		// Done
		::closedir(pDir);
	}

#endif /* _WIN32 */

	// Sort the files
	sort(rNamesOut.begin(), rNamesOut.end());

	// Prepend the path if asked to
	if (fullPaths) {
		for (vector< string >::iterator i = rNamesOut.begin();
			 (i != rNamesOut.end()); i++) {
			*i = (slashPath + *i);
		}
	}
}


// isDir: determine whether a path is a directory or not
// Will simply return false if path does not exist
//
bool isDir(const string &rPath)
{
#ifdef _WIN32

	DWORD attrs = ::GetFileAttributes(rPath.c_str());
	return (attrs != 0xFFFFFFFF ? ((attrs & FILE_ATTRIBUTE_DIRECTORY) != 0) : false);

#else /* ! _WIN32: */

	struct stat fileInfo;
	int rc = ::stat(rPath.c_str(), &fileInfo);
	return (rc == 0 ? S_ISDIR(fileInfo.st_mode) : false);

#endif /* _WIN32 */
}


// End of file.
