#include "stdafx.hpp"

#ifdef linux

#include "Platform/Platform.hpp"

IGNORE_WARNINGS_PUSH
#include <iostream>
#include <fstream>
#include <ctime>
#include <locale>

#include <sys/types.h> // For kill
#include <sys/stat.h> // For stat
#include <signal.h> // For kill
#include <unistd.h> // For getcwd, unlink, getpid
#include <errno.h> // For errno
#include <dirent.h> // For readdir
IGNORE_WARNINGS_POP

#include "FlexEngine.hpp"
#include "Helpers.hpp"

#ifndef MAX_PATH
// Match Windows behaviour
#define MAX_PATH 260
#endif

namespace flex
{
// Taken from https://stackoverflow.com/a/51846880/2317956
// These codes set the actual text to the specified color
#define RESETTEXT  "\x1B[0m" // Set all colors back to normal.
#define FOREBLK  "\x1B[30m" // Black
#define FORERED  "\x1B[31m" // Red
#define FOREGRN  "\x1B[32m" // Green
#define FOREYEL  "\x1B[33m" // Yellow
#define FOREBLU  "\x1B[34m" // Blue
#define FOREMAG  "\x1B[35m" // Magenta
#define FORECYN  "\x1B[36m" // Cyan
#define FOREWHT  "\x1B[37m" // White

/* BACKGROUND */
// These codes set the background color behind the text.
#define BACKBLK "\x1B[40m"
#define BACKRED "\x1B[41m"
#define BACKGRN "\x1B[42m"
#define BACKYEL "\x1B[43m"
#define BACKBLU "\x1B[44m"
#define BACKMAG "\x1B[45m"
#define BACKCYN "\x1B[46m"
#define BACKWHT "\x1B[47m"

#define BOLD(x) "\x1B[1m" x RESETTEXT // Embolden text then reset it.
#define BRIGHT(x) "\x1B[1m" x RESETTEXT // Brighten text then reset it. (Same as bold but is available for program clarity)
#define UNDL(x) "\x1B[4m" x RESETTEXT // Underline text then reset it.

	void Platform::GetConsoleHandle()
	{
	}

	void Platform::SetConsoleTextColor(ConsoleColour colour)
	{
#if ENABLE_CONSOLE_COLOURS
		static const char* const w_colours[] = { FOREWHT, FOREYEL, FORERED };

		std::cout << w_colours[(u32)colour];
#else
		FLEX_UNUSED(colour);
#endif
	}

	bool Platform::IsProcessRunning(u32 PID)
	{
		// Send blank signal to process
		i32 result = kill(PID, 0);
		// i32 err = errno;
		return result != 0;
	}

	u32 Platform::GetCurrentProcessID()
	{
		return (u32)getpid();
	}

	void Platform::MoveConsole(i32 width /* = 800 */, i32 height /* = 800 */)
	{
		/* TODO

		HWND hWnd = GetConsoleWindow();

		// The following four variables store the bounding rectangle of all monitors
		i32 virtualScreenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
		//i32 virtualScreenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
		i32 virtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		//i32 virtualScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		i32 monitorWidth = GetSystemMetrics(SM_CXSCREEN);
		//i32 monitorHeight = GetSystemMetrics(SM_CYSCREEN);

		// If another monitor is present, move the console to it
		if (virtualScreenWidth > monitorWidth)
		{
			i32 newX;
			i32 newY = 10;

			if (virtualScreenLeft < 0)
			{
				// The other monitor is to the left of the main one
				newX = -(width + 67);
			}
			else
			{
				// The other monitor is to the right of the main one
				newX = virtualScreenWidth - monitorWidth + 10;
			}

			MoveWindow(hWnd, newX, newY, width, height, TRUE);

			// Call again to set size correctly (based on other monitor's DPI)
			MoveWindow(hWnd, newX, newY, width, height, TRUE);
		}
		else // There's only one monitor, move the console to the top left corner
		{
			RECT rect;
			GetWindowRect(hWnd, &rect);
			if (rect.top != 0)
			{
				// A negative value is needed to line the console up to the left side of my monitor
				MoveWindow(hWnd, -7, 0, width, height, TRUE);
			}
		}
		*/
	}

	void Platform::PrintStringToDebuggerConsole(const char* str)
	{
		// Linux doesn't have an equivalent to Windows' OutputDebugString
		FLEX_UNUSED(str);
	}

	void Platform::RetrieveCurrentWorkingDirectory()
	{
		char cwdBuffer[MAX_PATH];
		char* str = getcwd(cwdBuffer, sizeof(cwdBuffer));
		if (str)
		{
			FlexEngine::s_CurrentWorkingDirectory = ReplaceBackSlashesWithForward(str);
		}
	}

	bool Platform::CreateDirectoryRecursive(const std::string& absoluteDirectoryPath)
	{
		if (absoluteDirectoryPath.find("..") != std::string::npos)
		{
			PrintError("Attempted to create directory using relative path! Must specify absolute path!\n");
			return false;
		}

		if (Platform::DirectoryExists(absoluteDirectoryPath))
		{
			// Directory already exists
			return true;
		}

		i32 status = 0;
		mode_t mode = S_IRUSR; // R for owner (see https://jameshfisher.com/2017/02/24/what-is-mode_t/)
		struct stat st;
		if (stat(absoluteDirectoryPath.c_str(), &st) != 0)
		{
			if (mkdir(absoluteDirectoryPath.c_str(), mode) != 0 && errno != EEXIST)
			{
				status = -1;
			}
		}
		else if (!S_ISDIR(st.st_mode))
		{
			errno = ENOTDIR;
			status = -1;
		}

		return status != -1;
	}

	bool Platform::DeleteFile(const std::string& filePath, bool bPrintErrorOnFailure /* = true */)
	{
		i32 result = unlink(filePath.c_str());
		if (!result)
		{
			return true;
		}
		else
		{
			if (bPrintErrorOnFailure)
			{
				PrintError("Failed to delete file %s\n", filePath.c_str());
			}
			return false;
		}
	}

	bool Platform::CopyFile(const std::string& filePathFrom, const std::string& filePathTo)
	{
		// TODO: Move into common file
		std::ifstream src(filePathFrom);
		std::ofstream dst(filePathTo);

		if (src.is_open() && dst.is_open())
		{
			dst << src.rdbuf();
			src.close();
			dst.close();
			return true;
		}
		else
		{
			PrintError("Failed to copy file from \"%s\" to \"%s\"\n", filePathFrom.c_str(), filePathTo.c_str());
			return false;
		}
	}

	bool Platform::DirectoryExists(const std::string& absoluteDirectoryPath)
	{
		if (absoluteDirectoryPath.find("..") != std::string::npos)
		{
			PrintError("Attempted to create directory using relative path! Must specify absolute path!\n");
			return false;
		}

		struct stat st;
		if (stat(absoluteDirectoryPath.c_str(), &st) != 0)
		{
			return S_ISDIR(st.st_mode);
		}

		return false;
	}

	void Platform::OpenExplorer(const std::string& absoluteDirectory)
	{
		// Linux doesn't quite have the same guaranteed idea of a "file explorer" as Windows
		// TODO: Look into alternatives
		FLEX_UNUSED(absoluteDirectory);
	}

	bool Platform::OpenFileDialog(const std::string& windowTitle, const std::string& absoluteDirectory, std::string& outSelectedAbsFilePath, char filter[] /* = nullptr */)
	{
		FLEX_UNUSED(windowTitle);
		FLEX_UNUSED(absoluteDirectory);
		FLEX_UNUSED(outSelectedAbsFilePath);
		FLEX_UNUSED(filter);

		// TODO: https://github.com/mlabbe/nativefiledialog ?
		// OPENFILENAME openFileName = {};
		// openFileName.lStructSize = sizeof(OPENFILENAME);
		// openFileName.lpstrInitialDir = absoluteDirectory.c_str();
		// openFileName.nMaxFile = (filter == nullptr ? 0 : strlen(filter));
		// if (openFileName.nMaxFile && filter)
		// {
			// openFileName.lpstrFilter = filter;
		// }
		// openFileName.nFilterIndex = 0;
		// const i32 MAX_FILE_PATH_LEN = 512;
		// char fileBuf[MAX_FILE_PATH_LEN];
		// memset(fileBuf, '\0', MAX_FILE_PATH_LEN - 1);
		// openFileName.lpstrFile = fileBuf;
		// openFileName.nMaxFile = MAX_FILE_PATH_LEN;
		// openFileName.lpstrTitle = windowTitle.c_str();
		// openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		// bool bSuccess = GetOpenFileName(&openFileName) == 1;

		// if (openFileName.lpstrFile)
		// {
		// 	outSelectedAbsFilePath = ReplaceBackSlashesWithForward(openFileName.lpstrFile);
		// }

		// return bSuccess;
		return false;
	}

	bool Platform::FindFilesInDirectory(const std::string& directoryPath, std::vector<std::string>& filePaths, const std::string& fileType)
	{
		std::string cleanedFileType = fileType;
		{
			size_t dotPos = cleanedFileType.find('.');
			if (dotPos != std::string::npos)
			{
				cleanedFileType.erase(dotPos, 1);
			}
		}

		std::string cleanedDirPath = directoryPath;
		if (cleanedDirPath[cleanedDirPath.size() - 1] != '/')
		{
			cleanedDirPath += '/';
		}

		std::string cleanedDirPathWithWildCard = cleanedDirPath + '*';

		struct dirent *ent;
		DIR *dir = opendir(cleanedDirPath.c_str());
		if (dir != NULL)
		{
			/* print all the files and directories within directory */
			while ((ent = readdir(dir)) != NULL)
			{
				std::string fileNameStr(ent->d_name);
				bool foundFileTypeMatches = false;
				if (cleanedFileType == "*")
				{
					foundFileTypeMatches = true;
				}
				else
				{
					size_t dotPos = fileNameStr.find('.');

					if (dotPos != std::string::npos && fileNameStr != "..")
					{
						std::vector<std::string> parts = Split(fileNameStr, '.');
						if (parts.size() > 0)
						{
							std::string foundFileType = parts[1];
							if (foundFileType == cleanedFileType)
							{
								foundFileTypeMatches = true;
							}
						}
					}
				}

				if (foundFileTypeMatches)
				{
					filePaths.push_back(cleanedDirPath + fileNameStr);
				}
			}
			closedir(dir);
		}
		else
		{
			/* could not open directory */
			PrintError("Error encountered while finding files in directory %s\n", cleanedDirPath.c_str());
			return false;
		}

		return !filePaths.empty();
	}

	std::string Platform::GetDateString_YMD()
	{
		std::stringstream result;

		char timeStrBuf[256];
		std::time_t t = std::time(nullptr);
		if (std::strftime(timeStrBuf, 256, "%Y %m %d", std::localtime(&t))) // In format "YYYY MM DD"
		{
			std::string timeStr(timeStrBuf);
			result << timeStr.substr(0, 4) << '-' <<
				timeStr.substr(5, 2) << '-' <<
				timeStr.substr(8, 2);
		}

		return result.str();
	}

	std::string Platform::GetDateString_YMDHMS()
	{
		std::stringstream result;

		char timeStrBuf[256];
		std::time_t t = std::time(nullptr);
		if (std::strftime(timeStrBuf, 256, "%Y %m %d %I %M %S", std::localtime(&t))) // In format "YYYY MM DD HH MM SS"
		{
			std::string timeStr(timeStrBuf);
			result << timeStr.substr(0, 4) << '-' <<
				timeStr.substr(5, 2) << '-' <<
				timeStr.substr(8, 2) << '-' <<
				timeStr.substr(11, 2) << '-' <<
				timeStr.substr(14, 2) << '-' <<
				timeStr.substr(17, 2);
		}

		return result.str();
	}

} // namespace flex
#endif // linux
