// Leif Larson's Brainf*ck interpreter
// runs .b or .bf files

#define GOOD_BUFFER 0
#define PATH_TOO_LONG 1
#define BAD_PATH_END 2

#include <fstream>
#include <iostream>

static bool debug = false;
const static size_t MAX_BUFFER_SIZE = 256;

#define EXTRANEOUS_OUTPUT 0   /* for debugging Brainf*ck programs */

#if EXTRANEOUS_OUTPUT
int interpretProgram(char* path, std::ostream& printErrorsWhere, std::ostream& printDebugWhere) {
#else
static int interpretProgram(char* path) {
#endif

	// set up interpreter environment

	// Setup a dynamic data array
	const static float EXPANSION_MULT = 1.25f; // How much to extend the array by when it expands. Must be enough to increase the size of the array.
	size_t len = 16;				// length of the data array
	size_t lm1 = len - 1;			// length minus 1
	char* data = new char[len];		// data array
	for (size_t i = 0; i < len; i++) {
		data[i] = '\0';
	}
	char* dp = data + lm1/3;		// data pointer
	/* Note: the dp starts out biased towards the left side of the array, 
	 * since brainfuck programs usually don't use that side.
	 */
	
	// Setup our file stream
	std::ifstream f(path);	// the filestream with our brainfuck program
	char* ip;				// instruction pointer

	// load all instructions into char* prog
	f.seekg(0, std::ios_base::end);
	size_t progLen = 0;
	char* prog = nullptr;

	// code in this branch taken from https://cplusplus.com/reference/istream/istream/seekg/
	if (f) {
		// get length of file:
		f.seekg(0, f.end);
		progLen = f.tellg();
		f.seekg(0, f.beg);

		// allocate memory:
		prog = new char[progLen];

		// read data as a block:
		f.read(prog, progLen);

		f.close();
	}

	if (prog == nullptr) {
		#if EXTRANEOUS_OUTPUT
		printErrorsWhere << path << ": Failed to open file." << std::endl;
		#endif
		return 2;
	}
	
	// execute the program
	ip = prog;
	while (ip - prog < progLen) { 	
		switch (*ip) {
		// MOVE DATA POINTER
		case '<':
			// expand array to the left if needed
			if (dp == data) {
				char* newData = new char[len * EXPANSION_MULT];
				int newi = len * (EXPANSION_MULT - 1);
				// position the data pointer
				dp = dp - data - 1 + newData + newi;
				// copy data to newData
				for (int i = 0; i < len; i++) {
					newData[newi] = data[i];
				}
				delete[] data;
				data = newData;			
				len *= EXPANSION_MULT;	// update length
				for (int i = 0; i < len; i++) {
					data[i] = '\0';
				}
				break;
			}
			// else just decrement data pointer
			--dp;
			break;
		case '>':
			// expand array to the right if needed
			if (dp - lm1 == data) {
				char* newData = new char[len * EXPANSION_MULT];
				// position the data pointer
				dp = dp - data + 1 + newData;
				// copy data to newData
				for (size_t i = 0; i < len; i++) {
					newData[i] = data[i];
				}
				delete[] data;
				data = newData;			
				len *= EXPANSION_MULT;	// update length
				for (size_t i = 0; i < len; i++) {
					data[i] = '\0';
				}
				break;
			}
			// else just increment data pointer
			++dp;
			break;

		// ALTER DATA
		case '+':
			++*dp;
			break;
		case '-':
			--*dp;
			break;

		// INPUT/OUTPUT
		case '.':
			putchar(*dp);
			break;
		case ',':
			*dp = getchar();
			break;

		// CONTROL FLOW
		case '[':
			if (*dp == 0) {
				// TODO jump table would be more efficient than seeking every time!
				int brackets_passed = 0;
				do {
					++ip;
					switch (*ip) {
					case '[':
						++brackets_passed;
						break;
					case ']':
						--brackets_passed;
						break;
					default:
						break;
					}
				} while (brackets_passed >= 0);
			}
			// Else, execute inside the brackets.
			break;
		case ']':
			if (*dp != 0) {
				// TODO jump table would be more efficient than seeking every time!
				int brackets_passed = 0;
				do {
					--ip;
					switch (*ip) {
					case '[':
						++brackets_passed;
						break;
					case ']':
						--brackets_passed;
						break;
					default:
						break;
					}
				} while (brackets_passed <= 0);
			}
			// Else, do nothing and move on.
			break;
		default:
			// ignore any non-standard characters, such as white space, etc.
			break;
		}
		++ip; // advance to the next instruction
	}

	#if EXTRANEOUS_OUTPUT
	if (debug) {
		printDebugWhere << '\n' << path << ": Final state =\n"
			<< "[";
		for (size_t i = 0; i < len; i++) {
			printDebugWhere << (char)(data[i]); // NOTE we can't print null characters this way.
		}
		printDebugWhere << "]\n"
			<< " ";
		for (size_t i = 0; i < (dp - data) - 1; i++) {
			printDebugWhere << " ";
		}
		printDebugWhere << "^\n";
	}
	#endif

	return 0;	
}

#if EXTRANEOUS_OUTPUT
static int goodBuffer(const char* filepath, bool doPrint, std::ostream& printWhere) {
#else
static int goodBuffer(const char* filepath) {
#endif
	bool good = false;
	const char* last = filepath;
	while (*last != '\0' && last - filepath < MAX_BUFFER_SIZE)
	{
		last++;
	}
	if (*last != '\0') {
		#if EXTRANEOUS_OUTPUT	
		if (doPrint) { printWhere << "Path must be less than " << MAX_BUFFER_SIZE << " long.\n"; }
		#endif
		return PATH_TOO_LONG;
	}
	else if (last - filepath >= 3 && (
			(
				filepath[last - filepath - 2] == '.' &&
				filepath[last - filepath - 1] == 'b'
			) || (
				filepath[last - filepath - 3] == '.' &&
				filepath[last - filepath - 2] == 'b' &&
				filepath[last - filepath - 1] == 'f'
			)
		)
	) {
		return GOOD_BUFFER;
	}
	else {
		#if EXTRANEOUS_OUTPUT
		if (doPrint) { printWhere << "Path must end with either \".b\" or \".bf\"\n"; }
		#endif
		return BAD_PATH_END;
	}
	return good;
}

int main(int argc, char* argv[]) {
	char* filepath = new char[MAX_BUFFER_SIZE];
	// if the file was specified in the command line arguments
	if (argc > 0) {
		// check each argument to see if it has the filepath.
		int goodPath = -1;
		for (int i = 0; i < argc; ++i) {
			filepath = argv[i];
			#if EXTRANEOUS_OUTPUT
			if (goodBuffer(filepath, false, std::cerr)) {
			#else
			if (goodBuffer(filepath)) {
			#endif
				// check for debug mode argument "-db"
				++i;
				if (i < argc) {
					if (argv[i] != nullptr) {
						// NOTE args null terminated
						if (argv[i][0] == '-' &&
							argv[i][1] == 'd' &&
							argv[i][2] == 'b') {
							debug = true;
						}
					}
				}
				goodPath = true;
			}
		}
		if (goodPath != 0) {
			return goodPath; // bad filepath or no filepath!
		}
		#if EXTRANEOUS_OUTPUT
		return interpretProgram(filepath, std::cerr, std::cout);
		#else
		return interpretProgram(filepath);
		#endif
	}
	// without command line arguments... assume debug mode.
	else {
		while (true) {
			std::cout << "path = ";
			std::cin >> filepath;
			#if EXTRANEOUS_OUTPUT
			if (goodBuffer(filepath, true, std::cout)) {
			#else
			if (goodBuffer(filepath)) {
			#endif
				break;
			}
		}
		debug = true;
		#if EXTRANEOUS_OUTPUT
		return interpretProgram(filepath, std::cout, std::cout);
		#else
		return interpretProgram(filepath);
		#endif
	}
	
	// this would be a bug!
	return -1;
}