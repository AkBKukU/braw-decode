#include "braw.h"

// Returns the size in bytes required to store a single pixel of the format
unsigned int getPixelSizeInBytes(BlackmagicRawResourceFormat resourceFormat) {
	unsigned int size = 4;

	switch (resourceFormat) {
	case blackmagicRawResourceFormatRGBAU8:
	case blackmagicRawResourceFormatBGRAU8:
		// 4 color components * 1 byte
		size = 4;
		break;

	case blackmagicRawResourceFormatRGBU16:
	case blackmagicRawResourceFormatRGBU16Planar:
		// 3 color components * 2 bytes
		size = 6;
		break;

	case blackmagicRawResourceFormatRGBAU16:
	case blackmagicRawResourceFormatBGRAU16:
		// 4 color components * 2 bytes
		size = 8;
		break;

	case blackmagicRawResourceFormatRGBF32:
	case blackmagicRawResourceFormatRGBF32Planar:
		// 3 color components * 4 bytes
		size = 12;
		break;

	case blackmagicRawResourceFormatBGRAF32:
		// 4 color components * 4 bytes
		size = 16;
		break;

	default:
		break;
	}

	return size;
}

void FrameProcessor::ReadComplete(IBlackmagicRawJob* readJob, HRESULT result,
IBlackmagicRawFrame* frame)
{
	BrawInfo* info = nullptr;
	readJob->GetUserData((void**)&info);

	frame->SetResourceFormat(info->resourceFormat);
	frame->SetResolutionScale(info->resolutionScale);
	
	
	IBlackmagicRawJob* decodeAndProcessJob = nullptr;
	frame->CreateJobDecodeAndProcessFrame(nullptr, nullptr, &decodeAndProcessJob);

	decodeAndProcessJob->SetUserData(info);

	result = decodeAndProcessJob->Submit();

	if (result != S_OK)
	{
		if (decodeAndProcessJob)
			decodeAndProcessJob->Release();
	}

	readJob->Release();
}

void FrameProcessor::ProcessComplete(IBlackmagicRawJob* job, HRESULT result,
IBlackmagicRawProcessedImage* processedImage)
{
	BrawInfo* info = nullptr;
	job->GetUserData((void**)&info);

	if(info->verbose)
		std::cerr << "\rBRAW Frame/threads [" << info->frameIndex << "][" << info->threads << "] ";

	unsigned int width = 0;
	unsigned int height = 0;
	processedImage->GetWidth(&width);
	processedImage->GetHeight(&height);

	if (info->infoPass)
	{
		info->width = width;
		info->height = height;
	}

	void* imageData = nullptr;
	processedImage->GetResource(&imageData);

	// GetResource returns a buffer from the SDK that can contain some extra bytes, possibly
	// because of memory alignment to certain byte boundaries. GetResourceSizeBytes returns
	// the size of this buffer, and therefore does not reflect the actual image date bytes. So
	// we need to calculate it manually (width * height * pixelSize), and use the exact image
	// size for outputting the buffer to stdout.
	BlackmagicRawResourceFormat format;
	processedImage->GetResourceFormat(&format);
	unsigned int pixelSize = getPixelSizeInBytes(format);
	unsigned int size = width * height * pixelSize;

	// Print out image data
	if (!info->infoPass)
		std::cout.write(reinterpret_cast<char*>(imageData), size); 

	job->Release();

	if (!info->infoPass)
		info->threads--;
}

Braw::Braw()
{
	info = new BrawInfo();
}

Braw::~Braw()
{
	codec->FlushJobs();

	if (clip != nullptr)
		clip->Release();

	if (codec != nullptr)
		codec->Release();

	if (factory != nullptr)
		factory->Release();
}

void Braw::openFile(std::string filepath)
{
	// Store file
	info->filename = filepath;

	// Setup BRAW SDK
	factory = CreateBlackmagicRawFactoryInstanceFromPath(lib);
	factory->CreateCodec(&codec);
	const char *c = info->filename.c_str();
	codec->OpenClip(c, &clip);
	codec->SetCallback(&frameProcessor);

	clip->GetFrameCount(&(info->frameCount));
	clip->GetFrameRate(&info->framerate);

	if(frameOut == 0)
		frameOut = info->frameCount;

	IBlackmagicRawJob* jobRead = nullptr;
	clip->CreateJobReadFrame(info->frameIndex, &jobRead);
	jobRead->SetUserData(info);
	HRESULT result;
	result = jobRead->Submit();

        if (result != S_OK)
        {
            if (jobRead != nullptr)
                jobRead->Release();
        }
	codec->FlushJobs();

	info->infoPass = false;

	if (clipInfo)
	{
		printInfo();
		return;
	}else if (ffPrint)
	{
		printFFFormat();
		return;
	}else{
		decode();
	}
}

void Braw::decode()
{
	info->frameIndex = frameIn;

	if(info->verbose)
		std::cerr << "Begining decode of " << frameIn << "-" << frameOut << std::endl;
	
	while (info->frameIndex < frameOut)
	{
		if (info->threads >= maxThreads)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(50));
			continue;
		}

		IBlackmagicRawJob* jobRead = nullptr;
		clip->CreateJobReadFrame(info->frameIndex, &jobRead);
		jobRead->SetUserData(info);
		HRESULT result;
		result = jobRead->Submit();

		if (result != S_OK)
		{
		    if (jobRead != nullptr)
			jobRead->Release();
		}

		info->threads++;
		info->frameIndex++;
	}
}

void Braw::addArgs(ArgParse *parser)
{	
	parser->addArg(argColorFormat,&rawColorFormat,argColorFormatOptions,argColorFormatDescriptions);
	parser->addArg(argMaxThreads,&rawMaxThreads);
	parser->addArg(argFrameIn,&rawFrameIn);
	parser->addArg(argFrameOut,&rawFrameOut);
	parser->addArg(argScale,&rawScale,argScaleOptions);
	parser->addArg(argClipInfo,&clipInfo);
	parser->addArg(argVerbose,&info->verbose);
	parser->addArg(argFFFormat,&ffPrint);
}

void Braw::validateArgs()
{
	try
	{
		maxThreads = std::stoi(rawMaxThreads);
		frameIn = std::stoi(rawFrameIn);
		frameOut = std::stoi(rawFrameOut);
		info->scale = std::stoi(rawScale);

		// Get color format
		info->resourceFormat = rawColorFormat == "rgba" ? blackmagicRawResourceFormatRGBAU8 : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "bgra" ? blackmagicRawResourceFormatBGRAU8 : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "16il" ? blackmagicRawResourceFormatRGBU16 : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "16pl" ? blackmagicRawResourceFormatRGBU16Planar : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "f32s" ? blackmagicRawResourceFormatRGBF32 : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "f32p" ? blackmagicRawResourceFormatRGBF32Planar : info->resourceFormat;
		info->resourceFormat = rawColorFormat == "f32a" ? blackmagicRawResourceFormatBGRAF32 : info->resourceFormat;
		// Get scale		
		info->resolutionScale = rawScale == "1" ? blackmagicRawResolutionScaleFull : info->resolutionScale;
		info->resolutionScale = rawScale == "2" ? blackmagicRawResolutionScaleHalf : info->resolutionScale;
		info->resolutionScale = rawScale == "4" ? blackmagicRawResolutionScaleQuarter : info->resolutionScale;
		info->resolutionScale = rawScale == "8" ? blackmagicRawResolutionScaleEighth : info->resolutionScale;
	} catch (const std::exception& e) {
		std::cerr << "Invalid BRAW parameters" << std::endl;
		std::exit(1);
	}
}

void Braw::printInfo()
{
	std::cout << "Braw Decoder: Alpha" << std::endl;
	std::cout << "File: " << info->filename << std::endl;
	std::cout << "Resolution: " << info->width << "x" << info->height << std::endl;
	std::cout << "Framerate: " << info->framerate << std::endl;
	std::cout << "Frame Count: " << info->frameCount << std::endl;
	std::cout << "Scale: " << info->scale << std::endl;
}

void Braw::printFFFormat()
{
	std::string ffmpegInputFormat = "-f rawvideo -pixel_format ";
	ffmpegInputFormat += info->resourceFormat == blackmagicRawResourceFormatRGBAU8 ? "rgba" : "";
	ffmpegInputFormat += info->resourceFormat == blackmagicRawResourceFormatBGRAU8 ? "bgra" : "";
	ffmpegInputFormat += info->resourceFormat == blackmagicRawResourceFormatRGBU16 ? "rgb48le" : "";
	ffmpegInputFormat += info->resourceFormat == blackmagicRawResourceFormatRGBU16Planar ? "gbrp16le" : "";
	if (info->resourceFormat == blackmagicRawResourceFormatRGBF32 ||
		info->resourceFormat == blackmagicRawResourceFormatRGBF32Planar ||
		info->resourceFormat == blackmagicRawResourceFormatBGRAF32 )
	{
		std::cerr << "FFmpeg format unknown for: " << rawColorFormat << std::endl;
		std::exit(1);
	}

	ffmpegInputFormat += " -s ";
	ffmpegInputFormat += std::to_string(info->width);
	ffmpegInputFormat += "x";
	ffmpegInputFormat += std::to_string(info->height);
	ffmpegInputFormat += " -r ";
	ffmpegInputFormat += std::to_string(info->framerate);
	ffmpegInputFormat += " -i pipe:0 ";
	
	if (info->resourceFormat == blackmagicRawResourceFormatRGBU16Planar)
		ffmpegInputFormat += "-filter:v colorchannelmixer=0:1:0:0:0:0:1:0:1:0:0:0 ";

	std::cout << ffmpegInputFormat;
}

