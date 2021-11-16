#include "argparse.h"
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

	args->description = "This is a test program for parsing args\n\r";

	bool farg = false;

	args->addArg((ARG){'h',"help","Print help text"},&help);

	args->addArg((ARG){'f',"flag","This is a flag test"},&farg);

	std::string sarg;

	args->addArg((ARG){'s',"string","This is a value test"},&sarg);
	
	std::string oarg;
	std::vector<std::string> options;
	options.push_back("RGB8");
	options.push_back("RGB16");
	std::vector<std::string> descriptions;
	descriptions.push_back("8b RGB interleaved");
	descriptions.push_back("16b RGB planar");
	args->addArg((ARG){'o',"option","This is a option test"},&oarg,options,descriptions);

	args->addArg((ARG){'a',"action","This is an action test"},&test_func);
	//std::cout << argc << " " << argv[1] << std::endl;
	args->parse();

	std::cout << "farg: " << farg << std::endl ;
	std::cout << "sarg: " << sarg << std::endl ;
	std::cout << "oarg: " << oarg << std::endl ;
	return 0;
}

