//
// SwfBuffer.cpp
// Simple, limited Flash file parser and modifier.
// Just enough capabilities to support LocalContentUpdater utility.
// See http://www.macromedia.com/software/flash/open/licensing/fileformat/
//

#include "SwfBuffer.h"

#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define FILE_ATTRS_TAG_TYPE 69
#define USE_NETWORK_FLAG_MASK 0x00000001
#define FILE_PADDING 200  // more than enough space for everything we might add


////////////////////////////////////////////////////
// CONSTRUCTION AND DESTRUCTION

SwfBuffer::SwfBuffer()
{
	m_path = NULL;
	m_pBuf = NULL;
	m_bCompressed = false;
	m_len = 0;
	m_pos = 0;
	m_bChanges = false;
	m_successMsgBuf[0] = '\0';
}

SwfBuffer::~SwfBuffer()
{
	free( m_path );
	delete[] m_pBuf;
}


////////////////////////////////////////////////////
// OVERALL OPERATION

bool SwfBuffer::Read( FILE *fp, const char *path )
{
	m_path = strdup( path );

	// How much data?
	fseek( fp, 0, SEEK_END );
	m_len = ftell( fp );
	rewind( fp );

	// Allocate buffer and read data
	m_pBuf = new U8[ m_len + FILE_PADDING ];
	U32 nRead = fread( m_pBuf, 1, m_len, fp );
	if ( nRead < m_len )
	{
		fprintf( stderr, "error: short read\n" );
		return false;
	}

	return true;
}

void SwfBuffer::Write( FILE *fp )
{
	U32 origLen = m_len;

	// Recompress if necessary
	if ( m_bCompressed )
	{
		// Allocate compression buffer
		U32 compressLen = (U32) ( ( m_len - 8 ) * 1.2 );
		U8 *pNewBuf = new U8[ compressLen ];
		memcpy( pNewBuf, m_pBuf, 8 );

		// Compress
		int rc = compress( pNewBuf + 8, &compressLen, m_pBuf + 8, m_len - 8 );

		// Replace uncompressed buffer with compressed buffer
		delete[] m_pBuf;
		m_pBuf = pNewBuf;
		m_len = compressLen + 8;

		if ( rc != Z_OK )
		{
			fprintf( stderr, "error: failed to compress\n" );
			return;
		}
	}

	// Record modified file size
	m_pos = 4;
	PutDWord( origLen );

	// Write buffer to file
	fwrite( m_pBuf, 1, m_len, fp );
}


////////////////////////////////////////////////////
// LOW-LEVEL PARSING

U16 SwfBuffer::GetWord()
{
    U16 rtn = (U16) m_pBuf[ m_pos ] | ( (U16) m_pBuf[ m_pos + 1 ] << 8 );
	m_pos += 2;
	return rtn;
}

void SwfBuffer::PutWord( U16 word )
{
	m_pBuf[ m_pos++ ] = (U8) word;
	m_pBuf[ m_pos++ ] = (U8) ( word >> 8 );
}

U32 SwfBuffer::GetDWord()
{
    U32 rtn = (U32) m_pBuf[ m_pos ] | ( (U32) m_pBuf[ m_pos + 1 ] << 8 )
		      | ( (U32) m_pBuf[ m_pos + 2 ] << 16 ) | ( (U32) m_pBuf[ m_pos + 3 ] << 24 );
	m_pos += 4;
	return rtn;
}

void SwfBuffer::PutDWord( U32 dword )
{
	m_pBuf[ m_pos++ ] = (U8) dword;
	m_pBuf[ m_pos++ ] = (U8) ( dword >> 8 );
	m_pBuf[ m_pos++ ] = (U8) ( dword >> 16 );
	m_pBuf[ m_pos++ ] = (U8) ( dword >> 24 );
}

bool SwfBuffer::ParseTagHeader( U16 *pCodeOut, U32 *pLenOut )
{
	if ( m_pos + 1 >= m_len )
	{
		fprintf( stderr, "%s: bad file: unexpected eof\n", m_path );
		return false;
	}

	// Get short tag header
	U16 header = GetWord();

	// Parse short header
	*pLenOut = header & 0x3F;
	*pCodeOut = header >> 6;

	// Get extended size if necessary
	if ( *pLenOut == 0x3F )
	{
		if ( m_pos + 3 >= m_len )
		{
			fprintf( stderr, "%s: bad file: unexpected eof\n", m_path );
			return false;
		}

		*pLenOut = GetDWord();
	}

	if ( m_pos + *pLenOut > m_len )
	{
		fprintf( stderr, "%s: bad file: unexpected eof\n", m_path );
		return false;
	}

	return true;
}


////////////////////////////////////////////////////
// INITIALIZATION

bool SwfBuffer::ParsePastHeaderAndDecompress()
{
	if ( m_len < 8 )
	{
		fprintf( stderr, "%s: bad file: too short\n", m_path );
		return false;
	}

	// First byte of file: indicates whether compressed
	switch ( m_pBuf[0] )
	{
		case 'F': m_bCompressed = false; break;
		case 'C': m_bCompressed = true; break;
		default: fprintf( stderr, "%s: bad file: bad signature\n", m_path ); return false;
	}

	// Verify second and third bytes make up valid file signature
	if ( m_pBuf[1] != 'W' || m_pBuf[2] != 'S' )
	{
		fprintf( stderr, "%s: bad file: bad signature\n", m_path );
		return false;
	}

	// Decompress if necessary
	if ( m_bCompressed )
	{
		// Read size that we can expect to uncompress to
		m_pos = 4;
		U32 len = GetDWord();

		// Allocate decompression buffer
		U8 *pNewBuf = new U8[ len + FILE_PADDING ];
		memcpy( pNewBuf, m_pBuf, 8 );

		// Decompress
		U32 decompressLen = len - 8;
		int rc = uncompress( pNewBuf + 8, &decompressLen, m_pBuf + 8, m_len - 8 );

		// Replace compressed buffer with uncompressed buffer
		delete[] m_pBuf;
		m_pBuf = pNewBuf;
		m_len = len;

		if ( rc != Z_OK || decompressLen != len - 8 )
		{
			fprintf( stderr, "%s: bad file: bad compression\n", m_path );
			return false;
		}
	}

	// File header size is variable because of RECT structure.
	// Determine RECT size, parse past it, and skip remaining 12 bytes of file header.
	U8 nRectBits = ( ( ( m_pBuf[8] >> 3 ) & 0x1F ) * 4 ) + 5;
	U8 nRectBytes = nRectBits / 8;
	if ( nRectBits % 8 != 0 ) nRectBytes++;
	m_pos = 12 + nRectBytes;
	if ( m_pos >= m_len )
	{
		fprintf( stderr, "%s: bad file: too short\n", m_path );
		return false;
	}

	return true;
}


////////////////////////////////////////////////////
// MAIN TASKS
//
// NOTE: Currently we support only ONE of these operations per SwfBuffer.
//

// Determine whether first tag in file is FileAttributes, with the UseNetwork bit set.
//
bool SwfBuffer::Check()
{
	// Read tag type
	U16 tagCode;
	U32 tagLen;
	bool ok = ParseTagHeader( &tagCode, &tagLen );
	if ( !ok ) return false;

	bool found = false;

	// Is it FileAttributes?
	if ( tagCode == FILE_ATTRS_TAG_TYPE )
	{
		// Verify tag size
		if ( tagLen < 4 )
		{
			fprintf( stderr, "%s: bad file: FileAttributes tag too short\n", m_path );
			return false;
		}

		// Is the UseNetwork bit set?
		U32 flags = GetDWord();
		if ( flags & USE_NETWORK_FLAG_MASK )
		{
			printf( "Has network privileges: %s\n", m_path );
			found = true;
		}
	}

	if ( !found )
	{
		printf( "No network privileges: %s\n", m_path );
	}

	return true;
}

// If there is no FileAttributes tag, insert it as first tag in file, with UseNetwork bit set.
// If there is a FileAttributes tag without the UseNetwork bit set, set the UseNetwork bit.
// If there is a FileAttributes tag with the UseNetwork bit set, do nothing.
//
bool SwfBuffer::Add()
{
	// Remember parse position at first tag in file
	U32 savePos = m_pos;

	// Read tag type
	U16 tagCode;
	U32 tagLen;
	bool ok = ParseTagHeader( &tagCode, &tagLen );
	if ( !ok ) return false;

	// Is it already FileAttributes?
	if ( tagCode == FILE_ATTRS_TAG_TYPE )
	{
		// Remember parse position at start of tag body
		savePos = m_pos;

		// Read flags from tag body
		U32 flags = GetDWord();

		// Is the UseNetwork bit already set?
		if ( flags & USE_NETWORK_FLAG_MASK )
		{
			printf( "%s already has network privileges. Not modifying.\n", m_path );
			return false;
		}
		else
		{
			// Modify flags and overwrite them in tag
			flags |= USE_NETWORK_FLAG_MASK;
			m_pos = savePos;
			PutDWord( flags );

			m_bChanges = true;
			snprintf( m_successMsgBuf, SUCCESS_MSG_SIZE, "Network privileges added to %s\n", m_path );
			return true;
		}
	}
	else
	{
		// Need to insert FileAttributes.

		// Return to saved position
		m_pos = savePos;

		// Shift all tags rightward by 6 bytes to make room for new tag
		U8 *pSrc = m_pBuf + m_len - 1;
		U8 *pDst = pSrc + 6;
		for ( U32 i = m_len - 1; i >= m_pos; i-- )
		{
			*pDst-- = *pSrc--;
		}

		// Write new tag header
		U16 newHeader = (U16) FILE_ATTRS_TAG_TYPE << 6;  // tag type
		newHeader |= (U16) 4;  // tag length (excluding header)
		PutWord( newHeader );

		// Write new tag body
		U32 flags = USE_NETWORK_FLAG_MASK;
		PutDWord( flags );

		// Adjust file length
		m_len += 6;
		m_bChanges = true;

		snprintf( m_successMsgBuf, SUCCESS_MSG_SIZE, "Network privileges added to %s\n", m_path );
		return true;
	}
}

// Clear UseNetwork flag if present.
//
bool SwfBuffer::Remove()
{
	// Read tag type
	U16 tagCode;
	U32 tagLen;
	bool ok = ParseTagHeader( &tagCode, &tagLen );
	if ( !ok ) return false;

	// Is it FileAttributes?
	if ( tagCode == FILE_ATTRS_TAG_TYPE )
	{
		// Verify tag size
		if ( tagLen < 4 )
		{
			fprintf( stderr, "%s: bad file: FileAttributes tag too short\n", m_path );
			return false;
		}

		// Remember parse position at start of tag body
		U32 savePos = m_pos;

		// Read flags from tag body
		U32 flags = GetDWord();

		// Is the UseNetwork bit set?
		if ( flags & USE_NETWORK_FLAG_MASK )
		{
			// Clear the bit and write the modified flags back into the tag body
			flags &= ~USE_NETWORK_FLAG_MASK;
			m_pos = savePos;
			PutDWord( flags );

			m_bChanges = true;
			snprintf( m_successMsgBuf, SUCCESS_MSG_SIZE, "Network privileges removed from %s\n", m_path );
			return true;
		}
	}

	// If we get here, there is no FileAttributes tag, or the UseNetwork bit is not set
	printf( "%s does not have network privileges. Not modifying.\n", m_path );
	return false;
}


// End of file.
