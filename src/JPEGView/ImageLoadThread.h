
#pragma once

#include "ProcessParams.h"
#include "WorkThread.h"
#include <gdiplus.h>

class CJPEGImage;

// returned image data by CImageLoadThread.GetLoadedImage() method
class CImageData
{
public:
	// Gets the image
	CJPEGImage* Image;
	// True if the request failed due to memory
	bool IsRequestFailedOutOfMemory;
	// True if the request failed due to an unhandled exception
	bool IsRequestFailedException;

	CImageData(CJPEGImage* pImage, bool isRequestFailedOutOfMemory, bool isRequestFailedException) {
		Image = pImage;
		IsRequestFailedOutOfMemory = isRequestFailedOutOfMemory;
		IsRequestFailedException = isRequestFailedException;
	}
};

// Read ahead and processing thread
class CImageLoadThread : public CWorkThread
{
public:
	CImageLoadThread(void);
	~CImageLoadThread(void);

	// Asynchronous loading  of an image. The message WM_IMAGE_LOAD_COMPLETED is
	// posted to the given window's message queue when loading is finished and the given event is signaled (if not NULL).
	// Returns identifier to query for resulting image. The query can be done as soon as the message is
	// received or the event has been signaled.
	// The file to load is given by its filename (with path) and the frame index (for multiframe images). The
	// frame index needs to be zero when the image only has one frame.
	int AsyncLoad(LPCTSTR strFileName, int nFrameIndex, const CProcessParams & processParams, HWND targetWnd, HANDLE eventFinished);

	// Get loaded image, CImageData::Image is null if not (yet) available - use handle returned by AsyncLoad().
	// Call after having received the WM_IMAGE_LOAD_COMPLETED message to retrieve the loaded image.
	// Marks the request for deletion - only call once with the same handle
	CImageData GetLoadedImage(int nHandle);

	// Releases the cached image file if an image of the specified name is cached
	void ReleaseFile(LPCTSTR strFileName);

	// Gets the request handle value used for the last request
	static int GetCurHandleValue() { return m_curHandle; }

private:

	// Request for loading an image
	class CRequest : public CRequestBase {
	public:
		CRequest(LPCTSTR strFileName, int nFrameIndex, HWND wndTarget, const CProcessParams& processParams, HANDLE eventFinished) 
			: CRequestBase(eventFinished), ProcessParams(processParams) {
			FileName = strFileName;
			FrameIndex = nFrameIndex;
			TargetWnd = wndTarget;
			RequestHandle = ::InterlockedIncrement((LONG*)&m_curHandle);
			Image = NULL;
			OutOfMemory = false;
			ExceptionError = false;
		}

		CString FileName;
		int FrameIndex;
		HWND TargetWnd;
		int RequestHandle;
		CJPEGImage* Image;
		CProcessParams ProcessParams;
		bool OutOfMemory;  // load caused an out of memory condition
		bool ExceptionError;  // an unhandled exception caused the load to fail
	};

	// Request to release image file
	class CReleaseFileRequest : public CRequestBase {
	public:
		CReleaseFileRequest(LPCTSTR strFileName) : CRequestBase() {
			FileName = strFileName;
			Type = ReleaseFileRequest;
		}

		enum { ReleaseFileRequest = 1 };

		CString FileName;
	};

	static volatile int m_curHandle; // Request handle returned by AsyncLoad()

	Gdiplus::Bitmap* m_pLastBitmap; // Last read GDI+ bitmap, cached to speed up GIF animations
	CString m_sLastFileName; // Only for GDI+ files
	CString m_sLastWebpFileName; // Only for animated WebP files
	CString m_sLastPngFileName; // Only for animated PNG files
	CString m_sLastJxlFileName; // Only for animated JPEG XL files
	CString m_sLastAvifFileName; // Only for animated AVIF files

	virtual void ProcessRequest(CRequestBase& request);
	virtual void AfterFinishProcess(CRequestBase& request);
	void DeleteCachedGDIBitmap();
	void DeleteCachedWebpDecoder();
	void DeleteCachedPngDecoder();
	void DeleteCachedJxlDecoder();
	void DeleteCachedAvifDecoder();

	void ProcessReadJPEGRequest(CRequest * request);
	void ProcessReadPNGRequest(CRequest * request);
	void ProcessReadBMPRequest(CRequest * request);
	void ProcessReadTGARequest(CRequest * request);
	void ProcessReadWEBPRequest(CRequest * request);
	void ProcessReadJXLRequest(CRequest* request);
	void ProcessReadAVIFRequest(CRequest* request);
	void ProcessReadHEIFRequest(CRequest * request);
	void ProcessReadQOIRequest(CRequest * request);
	void ProcessReadPSDRequest(CRequest * request);
	void ProcessReadRAWRequest(CRequest * request);
	void ProcessReadGDIPlusRequest(CRequest * request);
	void ProcessReadWICRequest(CRequest* request);
	void ProcessReadDDSRequest(CRequest* request);

	static void SetFileDependentProcessParams(CRequest * request);
	static bool ProcessImageAfterLoad(CRequest * request);
};
