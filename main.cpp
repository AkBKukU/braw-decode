#include "argparse.h"
#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Test" << std::endl ;

	ArgParse args = ArgParse(argc,argv);
	
	bool farg = false;

	args.addArg((ARG){'f',"format","test"},&farg);

	//std::cout << argc << " " << argv[1] << std::endl;
	args.parse();

	return 0;
}

