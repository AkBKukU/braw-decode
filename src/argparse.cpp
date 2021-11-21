#include "argparse.h"

ArgParse::ArgParse(int argc, char *argv[])
{
	// Get system provided argument information
	this->argc = argc;
	this->argv = argv;
}

void ArgParse::printHelp()
{
	// Print help info from provided arg data
	std::cout << binName << " - " << description << std::endl;

	// Print flag help
	for(auto it = flagArgs.begin(); it != flagArgs.end(); ++it)
	{
		std::cout << " -" << (*it).shorthand << ", ";	
		std::cout << "--" << (*it).longhand << std::endl;	
		std::cout << "\t" << (*it).description << std::endl;
	}	
	// Print action help
	for(auto it = actionArgs.begin(); it != actionArgs.end(); ++it)
	{
		std::cout << " -" << (*it).shorthand << ", ";	
		std::cout << "--" << (*it).longhand << std::endl;	
		std::cout << "\t" << (*it).description << std::endl;
	}
	// Print value help
	for(auto it = valueArgs.begin(); it != valueArgs.end(); ++it)
	{
		std::cout << " -" << (*it).shorthand << ", ";	
		std::cout << "--" << (*it).longhand << " [VALUE]" <<  std::endl;	
		std::cout << "\t" << (*it).description << std::endl;
		// If value has limited options print those
		if((*it).strict)
		{
			std::cout << "\t" << "Options:"  << std::endl;
			for(int i = 0; i != it->options.size(); ++i)
			{
				std::cout << "\t\t" << it->options[i];
				if(!it->descriptions.empty())
				{
					std::cout << " - " << it->descriptions[i];
				}
				std::cout << std::endl;
			}

		}
	}
			
	std::exit(0);
}

void ArgParse::parse()
{
	// Parse system arguments based on provided data
	
	// Get program name if not provided
	if(binName.empty())
	{
		binName = argv[0];
	}
	// Convert arguments to strings for manipulation
	for ( int i = 1; i != argc ; ++i)
	{
		args.push_back(argv[i]);
	}
	
	// Begin processing arguments
	bool unknown = false;
	for (int passes = 3; passes != 0; --passes)
	{
		// First pass: Set flags and get values
		// Second pass: Run functions
		// Third pass: Error if any args left, store data if exists
		for(int iarg = 0; iarg != args.size(); ++iarg)
		{
			// Build temp arg to match against
			ARG match = ARG();
			
			// Look for short and long hand arguments
			if (args[iarg][0] == flagStart && args[iarg][1] == flagStart)
			{
				// Longhand
				if (passes == 1)
				{
					std::cout << "Unknown Argument: " << args[iarg] << std::endl;
					unknown = true;
				}
				match.longhand = args[iarg].substr(2);
				// Check if arg is registered
				if(checkArg(match,iarg,passes==2))
				{
					args.erase(args.begin()+iarg);
					--iarg;
				}

			} else if (args[iarg][0] == flagStart)
			{
				// Shorthand

				// Process individual characters
				for (int i = 1; i != args[iarg].length(); ++i)
				{
					if (passes == 1)
					{
						std::cout << "Unknown Argument: -" << args[iarg][i] << std::endl;
						unknown = true;
					}
					match.shorthand = args[iarg][i];
					// Check if arg is registered
					if(checkArg(match,iarg,passes==2))
					{
						args[iarg].erase(i,1);//remove character
						--i;
					}
				}
			}else{
				// Arbitrarry data, do nothing
			}
		}
	}

	// If and unknown argument was found print help and exit
	if (unknown)
	{
		printHelp();
		std::exit(1);
	}
	parsed = true;
}

bool ArgParse::checkArg(ARG arg, int iargs, bool action)
{
	// Actions should be processed last in case they need data
	if (!action) 
	{
		// Check for flags
		for(auto it = flagArgs.begin(); it != flagArgs.end(); ++it)
		{
			if ( (*it).shorthand == arg.shorthand ||
				(*it).longhand == arg.longhand )
			{
				// Set bool to true
				*(it->value) = true;
				return true;
			}
		}

		// Check for values
		for(auto it = valueArgs.begin(); it != valueArgs.end(); ++it)
		{
			if ( (*it).shorthand == arg.shorthand ||
				(*it).longhand == arg.longhand )
			{
				// Check if arg is strict
				if(!it->strict)
				{
					// Accept anything if not
					*(it->value) = args[iargs+1];
				}else{
					// Look for match if it is
					if( std::find(it->options.begin(), it->options.end(), args[iargs+1])  != it->options.end())
					{
						*(it->value) = args[iargs+1];
					}else{
						// Error and print help if match not found
						std::cout << "Unknown option \"" << args[iargs+1] << "\" for \"" << (*it).longhand << "\"" <<  std::endl;	
						printHelp();
						std::exit(1);
					}
				}
				// Remove data arg
				args.erase(args.begin()+iargs+1);
				return true;
			}
		}
	}else{
		// Check for actions
		for(auto it = actionArgs.begin(); it != actionArgs.end(); ++it)
		{
			if ( (*it).shorthand == arg.shorthand ||
				(*it).longhand == arg.longhand )
			{
				// Run provided function
				it->value();
				return true;
			}
		}
	}
}
		
std::vector<std::string> ArgParse::getArgsRemaining()
{
	// Returns all remaining data args
	return args;
}

void ArgParse::addArg(ARG arg, bool *value)
{
	ARG_FLAG temp = ARG_FLAG();
	
	temp.shorthand = arg.shorthand;
	temp.longhand = arg.longhand;
	temp.description = arg.description;

	temp.value = value;

	flagArgs.push_back(temp);
}

void ArgParse::addArg(ARG arg, std::string *value)
{
	ARG_VALUE temp = ARG_VALUE();

	temp.shorthand = arg.shorthand;
	temp.longhand = arg.longhand;
	temp.description = arg.description;

	temp.value = value;

	valueArgs.push_back(temp);
}

void ArgParse::addArg(ARG arg, std::string *value,
			std::vector<std::string> options)
{
	ARG_VALUE temp = ARG_VALUE();

	temp.shorthand = arg.shorthand;
	temp.longhand = arg.longhand;
	temp.description = arg.description;

	temp.value = value;
	temp.strict = true;
	temp.options = options;

	valueArgs.push_back(temp);
}

void ArgParse::addArg(ARG arg, std::string *value,
				std::vector<std::string> options,
				std::vector<std::string> descriptions)
{
	ARG_VALUE temp = ARG_VALUE();

	temp.shorthand = arg.shorthand;
	temp.longhand = arg.longhand;
	temp.description = arg.description;

	temp.value = value;
	temp.strict = true;
	temp.options = options;
	temp.descriptions = descriptions;

	valueArgs.push_back(temp);
}

void ArgParse::addArg(ARG arg, std::function<void ()> value)
{
	ARG_ACTION temp = ARG_ACTION();

	temp.shorthand = arg.shorthand;
	temp.longhand = arg.longhand;
	temp.description = arg.description;

	temp.value = value;

	actionArgs.push_back(temp);
}

