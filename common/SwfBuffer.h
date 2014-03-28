//
// SwfBuffer.h
// Simple, limited Flash file parser and modifier.
// Just enough capabilities to support LocalContentUpdater utility.
//

#ifndef _MFLCU_SWFBUFFER_H_
#define _MFLCU_SWFBUFFER_H_


#include <stdio.h>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

#define SUCCESS_MSG_SIZE 1000


class SwfBuffer
{
public:
	SwfBuffer();
	~SwfBuffer();

public:
	// Overall operation
	bool Read(FILE *fp, const char *path);
	bool HasChanges() { return m_bChanges; }
	void Write(FILE *fp);
	const char* GetSuccessMsg() { return m_successMsgBuf; }

public:
	// Initialization
	bool ParsePastHeaderAndDecompress();

public:
	// Main tasks
	// NOTE: Currently supports only ONE of these operations per SwfBuffer.
	bool Check();
	bool Add();
	bool Remove();

private:
	// Low-level parsing
	U16 GetWord();
	void PutWord(U16 word);
	U32 GetDWord();
	void PutDWord(U32 dword);
	bool ParseTagHeader(U16 *pCodeOut, U32 *pLenOut);

private:
	char *m_path;
	U8 *m_pBuf;
	bool m_bCompressed;
	U32 m_len;
	U32 m_pos;
	bool m_bChanges;
	char m_successMsgBuf[SUCCESS_MSG_SIZE];
};


#endif  /* _MFLCU_SWFBUFFER_H_ */

// End of file.
