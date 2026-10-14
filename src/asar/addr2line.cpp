#include "addr2line.h"
#include "asar.h"
#include "crc32.h"
#include "libstr.h"
#include <cstdint>

bool source_file_crc(const char * filename, uint32_t * out_crc);//main.cpp

//////////////////////////////////////////////////////////////////////////
// Class to store address-to-line mappings for richer symbolic information
//
// During assembly, included files and information about generated asm
// should be added to this, and then read back during symbol file
// generation

void AddressToLineMapping::reset()
{
	m_fileList.reset();
	m_filenameCrcs.reset();
	m_addrToLineInfo.reset();
}

// Adds information of what source file and line number an output rom address is at
void AddressToLineMapping::includeMapping(const char* filename, int line, int addr)
{
	AddrToLineInfo newInfo;
	newInfo.fileIdx = getFileIndex(filename);
	newInfo.line = line;
	newInfo.addr = addr;

	m_addrToLineInfo.append(newInfo);
}

// Helper to add file to list, and get the index of that file
int AddressToLineMapping::getFileIndex(const char* filename)
{
	// Quasar: consecutive lines nearly always come from the same file, so try the last one
	// before hashing the whole path for every emitted byte.
	int last = m_fileList.count - 1;
	if (last >= 0 && m_fileList[last].filename == filename) return last;

	// check if the file exists first
	uint32_t filenameCrc = crc32((const uint8_t*)filename, (unsigned int)strlen(filename));
	for (int i = 0; i < m_filenameCrcs.count; ++i)
	{
		if (m_filenameCrcs[i] == filenameCrc)
		{
			return i;
		}
	}

	// file doesn't exist, so start tracking it
	char* data = nullptr;
	int len = 0;
	uint32_t fileCrc = 0;
	// Quasar: assemblefile() already read this file and kept its CRC; reading it again here, once
	// per file per pass, was the bulk of a small patch's time.
	if (!source_file_crc(filename, &fileCrc) && readfile(filename, "", &data, &len))
	{
		fileCrc = crc32((const uint8_t*)data, (unsigned int)len);
	}
	free(data);

	m_fileList.append({ string(filename), fileCrc });
	m_filenameCrcs.append(filenameCrc);

	return m_fileList.count - 1;
}

