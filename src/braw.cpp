#include "braw.h"


void FrameProcessor::ReadComplete(IBlackmagicRawJob* readJob, HRESULT result,
IBlackmagicRawFrame* frame)
{
	BrawInfo* info = nullptr;
	readJob->GetUserData((void**)&info);

	IBlackmagicRawJob* decodeAndProcessJob = nullptr;
	frame->CreateJobDecodeAndProcessFrame(nullptr, nullptr, &decodeAndProcessJob);

	decodeAndProcessJob->SetUserData(info);

	frame->SetResourceFormat(info->resourceFormat);

	decodeAndProcessJob->Submit();

	if (decodeAndProcessJob)
		decodeAndProcessJob->Release();

	delete info;

	readJob->Release();
}

void FrameProcessor::ProcessComplete(IBlackmagicRawJob* job, HRESULT result,
IBlackmagicRawProcessedImage* processedImage)
{
	BrawInfo* info = nullptr;
	job->GetUserData((void**)&info);

	if(info->verbose)
		std::cerr << "BRAW Frame [" << info->frameIndex << "] \r";

	unsigned int size = 0;
	void* imageData = nullptr;

	processedImage->GetWidth(&info->width);
	processedImage->GetHeight(&info->height);
	processedImage->GetResource(&imageData);
	processedImage->GetResourceSizeBytes(&size);

	// Print out image data
	std::cout.write(reinterpret_cast<char*>(imageData), size); 

	delete info;

	job->Release();
	info->threads--;
}

Braw::Braw()
{
	info = new BrawInfo();
}

Braw::~Braw()
{
}

void Braw::openFile(std::string filepath)
{
	factory = CreateBlackmagicRawFactoryInstanceFromPath("../Libraries/");
	factory->CreateCodec(&codec);
	const char *c = info->filename.c_str();
	codec->OpenClip(c, &clip);
	codec->SetCallback(&frameProcessor);

	clip->GetFrameCount(&info->frameCount);
	clip->GetFrameRate(&info->framerate);

	
}

void Braw::addArgs(ArgParse *parser)
{	
	parser->addArg(argColorFormat,&rawColorFormat,argColorFormatOptions,argColorFormatDescriptions);
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
		frameIn = std::stoi(rawFrameIn);
		frameOut = std::stoi(rawFrameOut);
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


