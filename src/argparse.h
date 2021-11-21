
#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <functional>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <algorithm>

/* arg types needed

run and quit
-f , --ff-format , "Print the FFmpeg input format parameters"
-n , --info , "Print clip info"

No values available, must be input as other variable flags. 
Runs void function after parsing all other arguments.


alter program state
-v , --verbose , "Print more info to CERR"

takes pointer to bool that is set false if not found and true if is.


input array
-c , --color , "Decoded BRAW will be output in this format" , rgbau8|rgbau16|f32 , 
-s , --scale , "Scale output by this factor" , 1|2|4|8
-d , --decoder , "Decoder method to use" , cuda|cpu

takes a multidim array that has the values to match against and desctriptions of them to print for help
ArgParse will error and print help if argument provided is not in array


input variable
-i , --in , "Starting frame" , frame number
-o , --out , "Ending frame" , frame number

takes c-string pointer, parsing is on the user

arbitrary input
* , parse flagless inputs as files

function of class to return array of std::strings not associated with any flags

*/

struct ARG
{
	char shorthand;
	std::string longhand;
	std::string description; 
};

struct ARG_FLAG : ARG
{
	bool *value;
};

struct ARG_VALUE : ARG
{
	std::string *value;
	bool strict = false;
	std::vector<std::string> options;
	std::vector<std::string> descriptions;
};

struct ARG_ACTION : ARG
{
	std::function<void ()> value;
};

class ArgParse
{
	private:
		char flagStart = '-';
		int argc;
		char **argv;
		std::vector<std::string> args;
		bool parsed = false;

		void unknownArg(std::string arg);
		bool checkArg(ARG arg, int iargs, bool action);

		std::vector<ARG_FLAG> flagArgs;
		std::vector<ARG_VALUE> valueArgs;
		std::vector<ARG_ACTION> actionArgs;

	public:
		std::string description;
		std::string binName;

		ArgParse(int argc, char *argv[]);
		void addArg(ARG arg, bool *value);
		void addArg(ARG arg, std::string *value);
		void addArg(ARG arg, std::string *value,
				std::vector<std::string> options);
		void addArg(ARG arg, std::string *value,
				std::vector<std::string> options,
				std::vector<std::string> descriptions);
		void addArg(ARG arg, std::function<void ()> action );
		std::vector<std::string> getArgsRemaining();
		void printHelp();
		void parse();

};

#endif

