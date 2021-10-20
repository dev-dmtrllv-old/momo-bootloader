#pragma once

#include "core/macros.hpp"

namespace Shell
{
	typedef int(*CommandFunction)(const char* cwd, char** args, size_t argc);

	struct Command
	{
		char* cmd;
		CommandFunction func;
	} PACKED;

	void init();
	void start();
	void stop();

	bool registerCommand(char* cmd, CommandFunction func);
};
