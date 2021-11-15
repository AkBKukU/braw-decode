#include "argparse.h"

ArgParse::ArgParse(int argc, char *argv[])
{
	this->argc = argc;
	this->argv = argv;
}


void ArgParse::parse()
{
	
	std::cout << "Convert to std::string" << argc << std::endl;
	for ( int i = 1; i != argc ; ++i)
	{
		args.push_back(argv[i]);
	}

	for(auto it = args.begin(); it != args.end(); ++it)
	{
		if (*it[0] == flagStart && *it[1] == flagStart)
		{
			// Longhand
			for(auto itf = flagArgs.begin(); itf != flagArgs.end(); ++itf)
			{

			}
		} else if (*it[0] == flagStart)
		{
			// Shorthand

		}else{
			// Data
		}
		
	}

}

void ArgParse::addArg(ARG arg, bool *value)
{
	flagArgs.push_back(arg);
	flagValues.push_back(value);
}

void ArgParse::addArg(ARG arg, char *value[])
{
	valueArgs.push_back(arg);
	valueValues.push_back(value);
}

void ArgParse::addArg(ARG arg, std::vector<std::vector<std::string>> value)
{
	optionArgs.push_back(arg);
	optionValues.push_back(value);
}

void ArgParse::addArg(ARG arg, std::function<void ()> value)
{
	actionArgs.push_back(arg);
	actionValues.push_back(value);
}

