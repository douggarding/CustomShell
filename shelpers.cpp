#include "shelpers.hpp"
#include "assert.h"

/*
  text handling functions
 */

bool splitOnSymbol(std::vector<std::string> &words, int i, char c)
{
	if (words[i].size() < 2)
	{
		return false;
	}
	int pos;
	if ((pos = words[i].find(c)) != std::string::npos)
	{
		if (pos == 0)
		{
			//starts with symbol
			words.insert(words.begin() + i + 1, words[i].substr(1, words[i].size() - 1));
			words[i] = words[i].substr(0, 1);
		}
		else
		{
			//symbol in middle or end
			words.insert(words.begin() + i + 1, std::string{c});
			std::string after = words[i].substr(pos + 1, words[i].size() - pos - 1);
			if (!after.empty())
			{
				words.insert(words.begin() + i + 2, after);
			}
			words[i] = words[i].substr(0, pos);
		}
		return true;
	}
	else
	{
		return false;
	}
}

std::vector<std::string> tokenize(const std::string &s)
{

	std::vector<std::string> ret;
	int pos = 0;
	int space;
	//split on spaces
	while ((space = s.find(' ', pos)) != std::string::npos)
	{
		std::string word = s.substr(pos, space - pos);
		if (!word.empty())
		{
			ret.push_back(word);
		}
		pos = space + 1;
	}

	std::string lastWord = s.substr(pos, s.size() - pos);
	if (!lastWord.empty())
	{
		ret.push_back(lastWord);
	}

	for (int i = 0; i < ret.size(); ++i)
	{
		for (auto c : {'&', '<', '>', '|'})
		{
			if (splitOnSymbol(ret, i, c))
			{
				--i;
				break;
			}
		}
	}

	return ret;
}

std::ostream &operator<<(std::ostream &outs, const Command &c)
{
	outs << c.exec << " argv: ";
	for (const auto &arg : c.argv)
	{
		if (arg)
		{
			outs << arg << ' ';
		}
	}
	outs << "fds: " << c.fdStdin << ' ' << c.fdStdout << ' ' << (c.background ? "background" : "");
	return outs;
}

//returns an empty vector on error
/*

  You'll need to fill in a few gaps in this function and add appropriate error handling
  at the end.

 */
std::vector<Command> getCommands(const std::vector<std::string> &tokens)
{

	// Create vector of commands to be returned. (1 + num |'s commands)
	std::vector<Command> ret(std::count(tokens.begin(), tokens.end(), "|") + 1);

	int first = 0;
	// Searches in the range defined by its first two arguments. It returns an iterator pointing
	// to the first element that matches. If no element matches, it returns its 2nd parameter.
	// Get the position of the token right before the first '|'?
	int last = (int)(std::find(tokens.begin(), tokens.end(), "|") - tokens.begin());
	bool error = false;

	// Create and process each command. ('i' represents which command is being worked).
	for (int i = 0; i < ret.size(); ++i)
	{
		if ((tokens[first] == "&") || (tokens[first] == "<") || (tokens[first] == ">") || (tokens[first] == "|"))
		{
			error = true;
			break;
		}

		ret[i].exec = tokens[first]; // get the name of the program to execute
		ret[i].argv.push_back(tokens[first].c_str()); // contains command and its flags
		ret[i].fdStdin = 0; // set file descriptor "in" to standard input
		ret[i].fdStdout = 1; // set file descriptor "out" to standard output
		ret[i].background = false; // set this command to not run in the background

		// Check all the tokens associated with this command
		for (int j = first + 1; j < last; ++j)
		{
			// If there is a redirection token, set the file descriptor to redirect as specified
			if (tokens[j] == ">" || tokens[j] == "<")
			{
				if (tokens[j] == "<")
				{
					// gets the file descriptor value
					ret[i].fdStdin = open(tokens[++j].c_str(), O_RDONLY);
				}
				else if (tokens[j] == ">")
				{
					// gets the file descriptor value
					ret[i].fdStdout = open(tokens[++j].c_str(), (O_CREAT | O_RDWR | O_TRUNC), 0666);
				}
			}

			// If there is an '&' symbol in this command, set it to run in the background
			else if (tokens[j] == "&")
			{
				ret[i].background = true;
			}
			// If here, this is a normal command line argument
			else
			{
				ret[i].argv.push_back(tokens[j].c_str());
			}
		} // end for loop


		// If we're here, then there was a command before this one, and we need
		// to pipe the previous command to the current one.
		if (i > 0)
		{
			// Create pipe
			int pipeFileDesc[2];
			int pipeRetValue = pipe(pipeFileDesc);
			if (pipeRetValue == -1)
			{
				perror("pipe");
				exit(EXIT_FAILURE);
			}

			// Hook up the ends of the pipe to the processes
			ret[i].fdStdin = pipeFileDesc[0];
			ret[i - 1].fdStdout = pipeFileDesc[1];

		}

		// exec wants argv to have a nullptr at the end!
		ret[i].argv.push_back(nullptr);

		// find the next pipe character
		first = last + 1;
		if (first < tokens.size())
		{
			last = (int)(std::find(tokens.begin() + first, tokens.end(), "|") - tokens.begin());
		}
	}

	return ret;
}
