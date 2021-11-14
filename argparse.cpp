#include "argparse.h"

ArgParse::ArgParse(int *argc, char **argv[])
{
	this->argc = argc;
	this->argv = argv;
}


void ArgParse::parse()
{
	std::cout << *argv[0] << std::endl;
}

void ArgParse::addArgFlag(ARG arg, bool *value)
{
	flagArgs.push_back(arg);
	flagValues.push_back(value);
}

