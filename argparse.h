
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

struct ARG
{
	char shorthand;
	char longhand[24];
	char description[256]; 
};

class ArgParse
{
	private:
		char flagStart = '-';
		int *argc;
		char ***argv;
		std::vector<std::string> args;

		void unknownArg(std::string arg);

		std::vector<ARG> flagArgs;
		std::vector<bool *> flagValues;
		std::vector<ARG> valueArgs;
		std::vector<char **> valueValues;
		std::vector<ARG> optionArgs;
		std::vector<std::vector<std::vector<std::string>>> optionValues; // I am not proud of this.
		std::vector<ARG> actionArgs;
		std::vector<std::function<void ()>> actionValues;


	public:
		ArgParse(int *argc, char **argv[]);
		void addArgFlag(ARG arg, bool *value);
		void addArgValue(ARG arg, char *value[]);
		void addArgAction(ARG arg, std::function<void ()> action );
		void addArgOptions(ARG arg, std::vector<std::vector<std::string>> options );
		std::vector<std::string> getArgsRemaining();
		void printHelp();
		void parse();

};

