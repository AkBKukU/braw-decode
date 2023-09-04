#include "argparse.h"
#include "braw.h"
#include <iostream>

ArgParse *args;

void test_func()
{
	std::cout << "Test Function" << std::endl ;
}

void help()
{
	args->printHelp();
	std::exit(0);
}


int main(int argc, char *argv[])
{
	args = new ArgParse(argc,argv);

	Braw braw_decoder;
	braw_decoder.addArgs(args);

	args->description = "BRAW file decoder\n\rUsage: braw-decode sample.braw | ffmpeg -y $(braw-decode -f sample.braw) -c:v hevc_nvenc output.mov";

	args->addArg((ARG){'h',"help","Print help text"},&help);
	args->parse();
	braw_decoder.validateArgs();
	std::vector<std::string> files = args->getArgsRemaining();
	if(files.size() == 1)
	{
		braw_decoder.openFile(files[0]);
	} else if (files.size() == 0) {
		std::cerr << "Missing input file argument" << std::endl;
		return 1;
	} else {
		std::cerr << "Too many files provided" << std::endl;
		for(int i = 0; i != files.size(); ++i)
		{
			std::cerr << "[" << i << "] " << files[i] << std::endl;
		}
		return 1;
	}

	return 0;
}

