#include "core/shell.hpp"

#include "core/macros.hpp"
#include "core/vesa.hpp"
#include "core/keyboard.hpp"
#include "core/string.hpp"
#include "core/ascii.hpp"
#include "core/mm.hpp"
#include "core/fs.hpp"

namespace Shell
{
	namespace
	{
		constexpr uint32_t maxCommands = MM::pageSize / sizeof(Command);
		constexpr uint32_t maxCwdCharacters = 256;

		bool isInitialized_;
		bool isRunning_;
		void* buffer_;
		Command* commandList_;
		char cwd_[maxCwdCharacters];

		Command* findCommand(char* cmd)
		{
			for (size_t i = 0; i < maxCommands; i++)
			{
				if (commandList_[i].func != nullptr)
				{
					if (strcmp(commandList_[i].cmd, cmd) == 0)
						return &commandList_[i];
				}
			}
			return nullptr;
		}

		size_t getCmdLength(char* buf)
		{
			size_t i = 0;
			while (buf[i] != '\0' && buf[i] != ' ')
				i++;
			return i;
		}

		void fillArgvPtrs(char* buf, char** argvPtr)
		{
			size_t i = 0;

			size_t argCharCounter = 0;
			size_t argCounter = 0;

			while (buf[i] != '\0')
			{
				if (buf[i] == ' ')
				{
					if (argCharCounter > 0)
						argCharCounter = 0;
				}
				else
				{
					if (argCharCounter == 0)
						argvPtr[argCounter++] = &buf[i];
					argCharCounter++;
				}
				i++;
			}
		}

		size_t getArgCount(char* buf)
		{
			size_t argCharCounter = 0;

			size_t argc = 0;

			size_t i = 0;

			while (buf[i] != '\0')
			{
				if (buf[i] == ' ')
				{
					if (argCharCounter > 0)
						argCharCounter = 0;
				}
				else
				{
					if (argCharCounter == 0)
						argc++;
					argCharCounter++;
				}
				i++;
			}

			return argc;
		}

		void terminateArgs(char* buf)
		{
			size_t i = 0;
			while (buf[i] != '\0')
			{
				if (buf[i] == ' ')
					buf[i] = '\0';
				i++;
			}
		}

		void processCommand(char* buf)
		{
			while (*buf == ' ')
				buf++;

			const size_t cmdLength = getCmdLength(buf) + 1;

			buf[cmdLength - 1] = '\0';

			Command* cmd = findCommand(buf);

			if (cmd != nullptr)
			{
				const size_t argLength = strlen(buf) - cmdLength;

				char* argBuf = &buf[cmdLength];

				const size_t argc = getArgCount(argBuf);

				char* argv[argc];

				fillArgvPtrs(argBuf, argv);

				terminateArgs(argBuf);

				int exitCode = cmd->func(argv, argc);
				if (exitCode != 0)
				{
					char* exitBuf = "Command exited with error code "INT_STR_BUFFER;
					itoa(exitCode, &exitBuf[31], 10);
					ERROR(exitBuf);
				}
			}
			else
			{
				Vesa::write("command ");
				Vesa::write(buf);
				Vesa::writeLine(" not found!");
			}
		}

		int changeDir(char* dir)
		{
			if (strcmp(dir, ".") == 0)
				return 0;

			FS::PathInfo pi;

			const size_t l = strlen(cwd_);
			char cpy[l];
			memcpy(cpy, cwd_, l + 1);

			char* cwdEnd = &cwd_[l];

			if (strcmp(dir, "..") == 0)
			{
				if (cwd_[l - 1] == '/') // we are already in the root dir
				{
					return 0;
				}

				size_t i = l;

				while (i > 2)
					if (cwd_[i--] == '/')
						break;

				if (i != 2)
					cwd_[i] = '\0';
				else
					cwd_[i + 1] = '\0';
				return 0;
			}
			else
			{
				if (dir[0] != '/' && cwd_[l - 1] != '/')
					*cwdEnd++ = '/';

				memcpy(cwdEnd, dir, strlen(dir) + 1);

				if (!FS::getPathInfo(&pi, cwd_))
				{
					Vesa::write("could not find ");
					Vesa::write(cwd_);
					Vesa::writeLine("!");

					memcpy(cwd_, cpy, l + 1);

					return 1;
				}
				else
				{
					if (pi.isDirectory)
						return 0;

					Vesa::write(cwd_);
					Vesa::writeLine(" is not a directory!");

					memcpy(cwd_, cpy, l + 1);

					return 2;
				}
			}

			Vesa::write(cwd_);
			Vesa::writeLine(" unknown error!");

			memcpy(cwd_, cpy, l + 1);

			return 3;
		}
	};

	bool registerCommand(char* cmd, CommandFunction func)
	{
		Command* foundCmd = findCommand(cmd);
		if (foundCmd == nullptr)
		{
			for (size_t i = 0; i < maxCommands; i++)
			{
				if (commandList_[i].func == nullptr)
				{
					INFO("REGISTER CMD");
					commandList_[i].cmd = cmd;
					commandList_[i].func = func;
					return true;
				}
			}
		}
		return false;
	}

	void init()
	{
		if (isInitialized_)
		{
			ERROR("Shell is already initialized!");
		}
		else
		{
			commandList_ = reinterpret_cast<Command*>(MM::getPage());
			memset(commandList_, 0, MM::pageSize);
			memset(cwd_, 0, maxCwdCharacters);
			memcpy(cwd_, "0:/", 3);

			registerCommand("clear", [](char** argv, size_t argc) {
				Vesa::clear();
				return 0;
			});

			registerCommand("echo", [](char** argv, size_t argc) {
				for (size_t i = 0; i < argc - 1; i++)
				{
					Vesa::write(argv[i]);
					Vesa::write(" ");
				}
				Vesa::writeLine(argv[argc - 1]);
				return 0;
			});

			registerCommand("cd", [](char** argv, size_t argc) {
				changeDir(argv[0]);
				return 0;
			});

			isInitialized_ = true;
		}
	}

	void start()
	{
		if (isRunning_)
		{
			ERROR("Shell is already running!");
		}
		else if (!isInitialized_)
		{
			ERROR("Shell is not initialized yet!");
		}
		else
		{
			isRunning_ = true;
			buffer_ = MM::getPage();

			while (isRunning_)
			{
				Vesa::write(cwd_);
				Vesa::write(" ");

				char* inputBuffer = reinterpret_cast<char*>(buffer_);
				memset(inputBuffer, 0, MM::pageSize);

				const size_t maxChars = MM::pageSize / sizeof(char);

				size_t inputLength = 0;
				size_t i = 0;

				uint16_t startOffset = Vesa::getCursorOffset();

				char writeBuf[2] = { '\0','\0' };

				Keyboard::KeyInfo keyInfo;

				while (i < maxChars)
				{
					Keyboard::getChar(&keyInfo);

					if (keyInfo.keyCode == 28) // enter
					{
						if (inputBuffer == 0)
							continue;
						break;
					}
					else if (keyInfo.keyCode == 14) // backspace
					{
						if (inputLength == 0)
							continue;

						if (i < inputLength)
						{
							const size_t cpySize = inputLength - i;
							char cpyBuf[cpySize];
							memcpy(cpyBuf, &inputBuffer[i], cpySize);
							memcpy(&inputBuffer[i - 1], cpyBuf, cpySize);
						}

						inputBuffer[--inputLength] = ' ';
						i--;
						Vesa::writeAt(inputBuffer, startOffset);
						Vesa::setCursorOffset(Vesa::getCursorOffset() - 1);

					}
					else if (keyInfo.keyCode == 75) // left arrow
					{
						if (i > 0)
						{
							i--;
							Vesa::setCursorOffset(Vesa::getCursorOffset() - 1);
						}
					}
					else if (keyInfo.keyCode == 77) // right arrow
					{
						if (i < inputLength)
						{
							i++;
							Vesa::setCursorOffset(Vesa::getCursorOffset() + 1);
						}
					}
					else if (Ascii::isChar(keyInfo.charCode))
					{
						if (i == inputLength)
						{
							inputBuffer[i] = static_cast<char>(keyInfo.charCode);
							writeBuf[0] = inputBuffer[i];

							Vesa::write(writeBuf);

							i++;
							inputLength++;
						}
						else
						{
							const size_t cpySize = inputLength - i;
							char cpyBuf[cpySize];
							memcpy(cpyBuf, &inputBuffer[i], cpySize);
							inputBuffer[i++] = keyInfo.charCode;
							memcpy(&inputBuffer[i], cpyBuf, cpySize);
							Vesa::writeAt(inputBuffer, 0);
							Vesa::writeAt(inputBuffer, startOffset);
							inputLength++;
							Vesa::setCursorOffset(Vesa::getCursorOffset() + 1);
						}
					}
				}

				Vesa::writeLine("");

				if (inputLength >= maxChars)
				{
					Vesa::writeLine("Max shell buffer reached!");
				}
				else if (inputLength > 0)
				{
					processCommand(reinterpret_cast<char*>(buffer_));
				}
				else
				{
					Vesa::writeLine("nothing provided!");
				}
			}

			MM::freePage(buffer_);
		}
	}

	void stop()
	{
		if (!isRunning_)
		{
			ERROR("Shell is not running!");
		}
		else if (!isInitialized_)
		{
			ERROR("Shell is not initialized yet!");
		}
		else
		{
			isRunning_ = false;
		}
	}
};
