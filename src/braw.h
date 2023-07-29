// Blackmagic RAW SDK
#ifndef BRAW_H
#define BRAW_H

#include "BlackmagicRawAPI.h"

#include "argparse.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <iterator>

struct BrawInfo
{
	// File Info
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int scale = 1;
	long unsigned int frameCount;
	unsigned int frameIndex = 0;
	float framerate = 0;
	std::string filename;
	
	// SDK Info
	BlackmagicRawResourceFormat resourceFormat = blackmagicRawResourceFormatRGBAU8;
	BlackmagicRawResolutionScale resolutionScale = blackmagicRawResolutionScaleFull;
	
	// Program Info
	bool verbose = false;
	bool infoPass = true;
	std::atomic<int> threads = {0};
};

class FrameProcessor : public IBlackmagicRawCallback
{
	public:
		explicit FrameProcessor() = default;
		virtual ~FrameProcessor() = default;

		virtual void ReadComplete(IBlackmagicRawJob* readJob, HRESULT result, IBlackmagicRawFrame* frame);
		virtual void ProcessComplete(IBlackmagicRawJob* job, HRESULT result, IBlackmagicRawProcessedImage* processedImage);

		virtual void DecodeComplete(IBlackmagicRawJob*, HRESULT) {}
		virtual void TrimProgress(IBlackmagicRawJob*, float) {}
		virtual void TrimComplete(IBlackmagicRawJob*, HRESULT) {}
		virtual void SidecarMetadataParseWarning(IBlackmagicRawClip*, const char*, uint32_t, const char*) {}
		virtual void SidecarMetadataParseError(IBlackmagicRawClip*, const char*, uint32_t, const char*) {}
		virtual void PreparePipelineComplete(void*, HRESULT) {}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*)
		{
			return E_NOTIMPL;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef(void)
		{
			return 0;
		}

		virtual ULONG STDMETHODCALLTYPE Release(void)
		{
			return 0;
		}

};


class Braw 
{
	private:
		// Arguments
		ARG argColorFormat = {'c',"color-format","Bit depth and order of color information to be output:"};
		std::vector<std::string> argColorFormatOptions = {"rgba","bgra","16il","16pl","f32s","f32p","f32a"};
		std::vector<std::string> argColorFormatDescriptions = {
			"Unsigned 8bit interleaved RGBA (FFmpeg format: rgba)",
			"Unsigned 8bit interleaved BGRA (FFmpeg format: bgra)",
			"Unsigned 16bit interleaved RGB (FFmpeg format: rgb48le)", 
			"Unsigned 16bit planar RGB      (FFmpeg format: gbrp16le)", // Use filter to realign colors -filter:v colorchannelmixer=0:1:0:0:0:0:1:0:1:0:0:0
			"Floating point interleaved RGB", // All 32b floats cause FFmpeg errors
			"Floating point planar RGB",
			"Floating point interleaved BGRA"
		};
		std::string rawColorFormat = "rgba";

		ARG argMaxThreads = {'t',"threads","Number of CPU threads to use for decoding"};
		std::string rawMaxThreads = "4";
		ARG argFrameIn = {'i',"in","Start frame index for decoding"};
		std::string rawFrameIn = "0";
		ARG argFrameOut = {'o',"out","End frame index for decoding"};
		std::string rawFrameOut = "0";
		ARG argScale = {'s',"scale", "Scale input video down by this factor."};
		std::vector<std::string> argScaleOptions = {"1","2","4","8"};
		std::string rawScale = "1";
		ARG argClipInfo = {'n',"info", "Print details of clip"};
		bool clipInfo = false;
		ARG argVerbose = {'v',"verbose", "Print more information to CERR while processing"};
		bool verbose = false;
		ARG argFFFormat = {'f',"ff-format","Print FFmpeg arguments for processing decoded video\n\r\tExample: '-f rawvideo -pixel_format rgba -s 3840x2160 -r 60 -i pipe:0`"};
		bool ffPrint = false;
		
		// File info
		BrawInfo *info;
		unsigned int frameIn;
		unsigned int frameOut;
		
		// BRAW SDK
		const char *lib = "Libraries/";
		int maxThreads = 4;
		IBlackmagicRawFactory* factory = nullptr;
		IBlackmagicRaw* codec = nullptr;
		IBlackmagicRawClip* clip = nullptr;
		IBlackmagicRawJob* readJob = nullptr;

		FrameProcessor frameProcessor;
		
		void decode();

	public:
		Braw();
		~Braw(); // Destructor needed to close up SDK
		void addArgs(ArgParse *parser);
		void validateArgs();
		void printInfo();

		void openFile(std::string filepath);
		void printFFFormat(); // Need to take scale into account
};


#endif

