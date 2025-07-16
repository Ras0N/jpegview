#include "StdAfx.h"
#include "DDSReader.h"
#include "JPEGImage.h"
#include "Helpers.h"
#include <string>
#include <dxgiformat.h>
#include <d3d12.h>

#include "DirectXTex.h"

//std::string getstring(LPCTSTR inputstring) {
//	if (inputstring == nullptr) {
//		return std::string();
//	}
//#ifdef UNICODE
//	uint64_t buffersize = WideCharToMultiByte(CP_ACP, 0, inputstring, -1, nullptr, 0, nullptr, nullptr);
//	if (buffersize == 0) {
//		return std::string();
//	}
//	std::string cv_string(buffersize, 0);
//	WideCharToMultiByte(CP_ACP, 0, inputstring, -1, &cv_string[0], buffersize, nullptr, nullptr);
//	return cv_string;
//#else
//	return std::string(inputstring);
//#endif
//}

CJPEGImage* CReaderDDS::ReadDdsImage(LPCTSTR strFileName, bool& bOutOfMemory) {
	if (strFileName == NULL) {
		return NULL;
	}
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		if (hr != RPC_E_CHANGED_MODE) {
		return NULL;
		}
	}
	DirectX::TexMetadata mdata;
	hr = DirectX::GetMetadataFromDDSFile(strFileName, DirectX::DDS_FLAGS_NONE, mdata);
	if (FAILED(hr)) {
		return NULL;
	}
	DirectX::ScratchImage image;
	hr = DirectX::LoadFromDDSFile(strFileName, DirectX::DDS_FLAGS_NONE, &mdata, image);
	if (FAILED(hr)) {
		return NULL;
	}
	DirectX::ScratchImage dstImage;
	bool processed = false;
	DirectX::ScratchImage decompressedImage;
	if (DirectX::IsCompressed(mdata.format)) {
		hr = DirectX::Decompress(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DXGI_FORMAT_UNKNOWN,
			decompressedImage
		);
		//const DirectX::Image* dbgpicture = decompressedImage.GetImage(0, 0, 0);
		if (FAILED(hr)) {
			return NULL;
		}
		if (decompressedImage.GetMetadata().format == DXGI_FORMAT_B8G8R8A8_UNORM) {
			;
		}
		else {
			hr = DirectX::Convert(
				decompressedImage.GetImages(),
				decompressedImage.GetImageCount(),
				decompressedImage.GetMetadata(),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				DirectX::TEX_FILTER_CUBIC,
				DirectX::TEX_THRESHOLD_DEFAULT,
				dstImage
			);
			processed = true;
		}
	}
	else {
		hr = DirectX::Convert(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DirectX::TEX_FILTER_CUBIC,
			DirectX::TEX_THRESHOLD_DEFAULT,
			dstImage
		);
		processed = true;
	}
	if (FAILED(hr)) {
		return NULL;
	}
	const DirectX::Image* picture = processed ? dstImage.GetImage(0,0,0) : decompressedImage.GetImage(0, 0, 0);
	uint64_t pixel_count = picture->width * picture->height * 4;
	uint8* pDest = new(std::nothrow) uint8[pixel_count];
	if (pDest == nullptr) {
		return NULL;
	}
	memcpy_s(pDest, pixel_count, picture->pixels, pixel_count);
	// convert picture to BGRA 8888 32bit
	// some DDS format may not have alpha channel, so just set the alpha to all 1
	/*
	* target pixel format : BGRA
	* bbbbbbbb|gggggggg|rrrrrrrr|aaaaaaaa
	*/
	CJPEGImage* pTargetImage = new CJPEGImage(picture->width, picture->height, pDest, NULL, 4, 0, IF_DDS, false, 0, 1, 0);

	return pTargetImage;
}