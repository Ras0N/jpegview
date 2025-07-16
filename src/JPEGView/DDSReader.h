#pragma once

class CJPEGImage;

// Simple reader for .dds files
class CReaderDDS
{
public:
	// Returns NULL in case of errors. backgroundColor is used for blending transparent parts of the image.
	static CJPEGImage* ReadDdsImage(LPCTSTR strFileName, bool& bOutOfMemory);
private:
	CReaderDDS(void);
};
