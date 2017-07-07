// AACParser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AACParser.h"
#include <iostream>
#include <vector>

static void ShowUsage(std::string name)
{
	std::cerr << "Usage: " << name.c_str() << "filename" << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc < 2) 
	{
		ShowUsage(argv[0]);
		return 1;
	}
	else
	{
		std::vector<std::string> arguments;
		for (int i = 1; i < argc; i++)
		{
			std::string filename = argv[i];
			AACParser parser;
			parser.Parse(filename);
		}
	}

	getchar();
	return 0;
}

