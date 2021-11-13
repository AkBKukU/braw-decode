
#include <functional>
#include <string>
#include <iostream>
#include <vector>

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


class ArgParse
{
	private:
		char flagStart = "-";
		int argc;
		char *argv[];
		std::vector<std::string> args;

		void unknownArg(std::string arg);


	public:
		ArgParse(int argc, char *argv[]);
		void addArgFlag(const char shorthand, const char longhand[], const char description[], bool *value);
		void addArgValue(const char shorthand, const char longhand[], const char description[], char *value[]);
		void addArgAction(const char shorthand, const char longhand[], const char description[], std::function<void ()> action );
		void addArgOptions(const char shorthand, const char longhand[], const char description[], std::vector<std::vector<std::string>> options );
		std::vector<std::string> getArgsRemaining();
		void printHelp();
		void parse();

}

