#include "argparse.h"
#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Test" << std::endl ;

	ArgParse args = ArgParse(&argc,&argv);
	const char text[] = "This is some text";
	ARG arg = {'f',"format","test"};
	bool farg = false;

	args.addArgFlag((ARG){'f',"format","test"},&farg);

	return 0;
}
