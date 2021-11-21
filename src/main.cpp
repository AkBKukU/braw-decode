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
}


int main(int argc, char *argv[])
{
	args = new ArgParse(argc,argv);

	Braw braw_decoder;
	braw_decoder.addArgs(args);

	args->description = "BRAW file decoder\n\r";

	args->addArg((ARG){'h',"help","Print help text"},&help);
	args->parse();
	braw_decoder.validateArgs();

	return 0;
}

