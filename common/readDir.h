//
// readDir.h
// Cross-platform directory utilities.
//

#ifndef _MFLCU_READDIR_H_
#define _MFLCU_READDIR_H_

#ifdef _MSC_VER
// Disable MSVC warning about truncation of long symbol names from STL
# pragma warning( disable: 4786 )
#endif /* _MSC_VER */

#include <vector>
#include <string>


// readDir: find all the files and directories in a directory
// Output is sorted (by char codes), "." and ".." are omitted
//
void readDir(const std::string &rDirPath,
			 std::vector< std::string > &rNamesOut,
			 bool fullPaths);

// isDir: determine whether a path is a directory or not
// Will simply return false if path does not exist
//
bool isDir(const std::string &rPath);


#endif  /* _MFLCU_READDIR_H_ */

// End of file.
