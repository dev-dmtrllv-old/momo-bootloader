#include "core/shell.hpp"

#include "core/macros.hpp"
#include "core/vesa.hpp"
#include "core/keyboard.hpp"
#include "core/string.hpp"
#include "core/ascii.hpp"
#include "core/mm.hpp"
#include "core/fs.hpp"
#include "core/path.hpp"
#include "core/elf.hpp"
#include "core/module.hpp"

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

		Command* findCommand(const char* cmd)
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

				int exitCode = cmd->func(cwd_, argv, argc);
				if (exitCode != 0)
				{
					INT_STR_BUFFER_ARR;
					itoa(exitCode, buf, 10);
					char exitStr[50] = {};
					memcpy(exitStr, "Command exited with error code ", 31);
					memcpy(&exitStr[31], buf, strlen(buf));
					ERROR(exitStr);
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
			char absolutePath[128];
			memset(absolutePath, 0, 128);
			Path::resolve(128, absolutePath, cwd_, dir);

			if (strlen(absolutePath) == 2)
			{
				INFO("cd root");
				memcpy(cwd_, absolutePath, strlen(absolutePath) + 1);
				cwd_[2] = '/';
				cwd_[3] = '\0';
				return 0;
			}
			else
			{
				FS::PathInfo pi;

				if (FS::getPathInfo(&pi, absolutePath))
				{
					if (pi.isDirectory)
					{
						memcpy(cwd_, absolutePath, strlen(absolutePath) + 1);
						return 0;
					}
					return 2;
				}
				return 1;
			}
		}

		void ls(char* dir)
		{
			FS::readDir(dir, [](const char* name, const FS::PathInfo& pi) {
				Vesa::Color fg;
				Vesa::Color bg;

				if (pi.isDirectory)
				{
					fg = Vesa::Color::YELLOW;
					bg = Vesa::Color::BLACK;
				}
				else
				{
					fg = Vesa::Color::LIGHT_BLUE;
					bg = Vesa::Color::BLACK;
				}

				const size_t l = strlen(const_cast<char*>(name));
				const size_t off = 80 - (Vesa::getCursorOffset() % 80);
				if (off < l)
					Vesa::writeLine("");
				Vesa::write(name, fg, bg);
				Vesa::write(" ");
			});
			Vesa::writeLine("");
		}
	};

	bool registerCommand(const char* cmd, CommandFunction func)
	{
		Command* foundCmd = findCommand(cmd);
		if (foundCmd == nullptr)
		{
			for (size_t i = 0; i < maxCommands; i++)
			{
				if (commandList_[i].func == nullptr)
				{
					Vesa::writeLine("registered command ", cmd);
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

			registerCommand("clear", [](const char* cwd, char** argv, size_t argc) {
				Vesa::clear();
				return 0;
			});

			registerCommand("echo", [](const char* cwd, char** argv, size_t argc) {
				if ((argv[0][0] == '>' || argv[0][0] == '<') && argc >= 2)
				{
					char* path;
					char absolutePath[128];

					if (Path::isAbsolute(argv[1]))
					{
						path = argv[1];
					}
					else
					{
						memset(absolutePath, 0, 128);
						Path::resolve(128, reinterpret_cast<char*>(absolutePath), cwd, argv[1]);
						path = absolutePath;
					}

					FS::PathInfo pi;
					if (FS::getPathInfo(&pi, path))
					{
						if (pi.isDirectory)
						{
							ls(path);
							return 0;
						}

						size_t numberOfPages = 0;
						void* fileBuf = MM::getPages(pi.size, &numberOfPages);

						if (!FS::readFile(path, fileBuf))
						{
							MM::freePages(fileBuf, numberOfPages);
							return 1;
						}

						Vesa::writeLine(reinterpret_cast<char*>(fileBuf));
						MM::freePages(fileBuf, numberOfPages);
						return 0;
					}
					else
					{
						return 1;
					}
				}
				else
				{
					for (size_t i = 0; i < argc - 1; i++)
					{
						Vesa::write(argv[i]);
						Vesa::write(" ");
					}
					Vesa::writeLine(argv[argc - 1]);
				}
				return 0;
			});

			registerCommand("cd", [](const char* cwd, char** argv, size_t argc) {
				return changeDir(argv[0]);
			});

			registerCommand("ls", [](const char* cwd, char** argv, size_t argc) {
				ls(cwd_);
				return 0;
			});

			registerCommand("elf", [](const char* cwd, char** argv, size_t argc) {
				char* path;
				char absolutePath[128];

				if (Path::isAbsolute(argv[0]))
				{
					path = argv[0];
				}
				else
				{
					memset(absolutePath, 0, 128);
					Path::resolve(128, reinterpret_cast<char*>(absolutePath), cwd, argv[0]);
					path = absolutePath;
				}

				FS::PathInfo pi;
				if (FS::getPathInfo(&pi, path))
				{
					if (pi.isDirectory)
						return 1;

					size_t numberOfPages = 0;
					void* fileBuf = MM::getPages(pi.size, &numberOfPages);

					if (!FS::readFile(path, fileBuf))
					{
						MM::freePages(fileBuf, numberOfPages);
						return 2;
					}

					Elf::info(fileBuf);

					MM::freePages(fileBuf, numberOfPages);
					return 0;
				}
				else
				{
					return 3;
				}
			});

			registerCommand("loadmod", [](const char* cwd, char** argv, size_t argc) {
				Module::Context* ctx = Module::loadModule(argv[0]);
				
				if(ctx == nullptr)
					return 1;

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
				Vesa::write(cwd_, Vesa::Color::LIGHT_GREEN, Vesa::Color::BLACK);
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
