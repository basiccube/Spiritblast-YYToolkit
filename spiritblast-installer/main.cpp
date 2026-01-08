#include <string>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <Windows.h>
#include <atlstr.h>
#include <nfd.hpp>

using namespace std;

#define AURIECORE_URL "https://github.com/AurieFramework/Aurie/releases/download/v2.0.0b/AurieCore.dll"
#define AURIEPATCHER_URL "https://github.com/AurieFramework/Aurie/releases/download/v2.0.0b/AuriePatcher.exe"
#define YYTOOLKIT_URL "https://github.com/AurieFramework/YYToolkit/releases/download/v5.0.0a/YYToolkit.dll"

// This doesn't work because there are only pre-release releases and no releases with Spiritblast as the name right now
//#define SPIRITBLAST_URL "https://github.com/basiccube/Spiritblast/releases/latest/download/Spiritblast.dll"
#define SPIRITBLAST_URL "https://github.com/basiccube/Spiritblast/releases/download/0.3/AB-Mod.dll"

LPWSTR convert_string(string str)
{
	CA2WEX wstr(str.c_str());
	LPWSTR lpstr = wstr;
	return lpstr;
}

bool exec_and_wait(string cmdLine)
{
	printf("Running : %s\n", cmdLine.c_str());

	// I fucking hate the Win32 API
	STARTUPINFO startInfo;
	PROCESS_INFORMATION procInfo;

	ZeroMemory(&startInfo, sizeof(startInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));
	startInfo.cb = sizeof(startInfo);

	LPWSTR cmdStr = convert_string(cmdLine);
	if (!CreateProcess(NULL, cmdStr, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo))
	{
		printf("Failed to create process : %s\n", cmdLine.c_str());
		return false;
	}
	WaitForSingleObject(procInfo.hProcess, INFINITE);

	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);

	return true;
}

#define MAX_DOWNLOAD_ATTEMPTS 3
bool download_file(string url, string outPath, int attempt = 1)
{
	if (attempt > MAX_DOWNLOAD_ATTEMPTS)
	{
		printf("Attempted to download a total of %i times but none have succeeded.\n", attempt - 1);
		return false;
	}

	printf("URL : %s\n", url.c_str());
	printf("Output destination : %s\n", outPath.c_str());
	printf("Attempt : %i\n", attempt);

	string curlCmd = "curl --retry 3 --progress-bar --create-dirs -L -o \"" + outPath + "\" \"" + url + "\"";
	if (!exec_and_wait(curlCmd))
	{
		printf("The download failed, retrying...\n");
		return download_file(url, outPath, ++attempt);
	}

	// Check if the file at the very least was created
	if (!filesystem::exists(outPath))
	{
		printf("File %s doesn't exist, the download might have failed.\nRetrying download...\n", outPath.c_str());
		return download_file(url, outPath, ++attempt);
	}

	// Check if it at least downloaded something that isn't "Not Found"
	printf("File downloaded, size : %ji\n", filesystem::file_size(outPath));
	if (filesystem::file_size(outPath) < 2048)
	{
		printf("File %s didn't download properly, retrying...\n", outPath.c_str());
		return download_file(url, outPath, ++attempt);
	}
	
	return true;
}

int wait_for_input()
{
	int c = getchar();
	return c;
}

int quit(int result)
{
	printf("\nPress any key to exit.\n");
	wait_for_input();

	NFD::Quit();
	exit(result);
	return result;
}

int main(int argc, char *argv[])
{
	NFD::Init();
	NFD::UniquePathU8 outputPath;

	printf("Spiritblast installer\n");

	printf("Select the folder where ANTONBLAST is installed.\n");
	nfdresult_t folderResult = NFD::PickFolder(outputPath);

	if (folderResult == NFD_ERROR)
	{
		printf("Folder selection dialog failed, quitting...\n");
		return quit(1);
	}
	else if (folderResult == NFD_CANCEL)
	{
		printf("Folder path was not specified, quitting...\n");
		return quit(1);
	}

	string outputStr = outputPath.get();
	printf("Path : %s\n", outputStr.c_str());

	string exePath = outputStr + "\\ANTONBLAST.exe";
	if (!filesystem::exists(exePath))
	{
		printf("ANTONBLAST executable not found!\nMake sure you selected the correct directory.\n");
		return quit(1);
	}
	else
		printf("ANTONBLAST executable found\n");

	printf("\nThe installer will now download all files required for the mod to work.\nPress any key to continue.\n");
	wait_for_input();

	string aurieCorePath = outputStr + "\\mods\\Native\\AurieCore.dll";
	if (!filesystem::exists(aurieCorePath))
	{
		printf("Downloading AurieCore.dll...\n");
		if (!download_file(AURIECORE_URL, aurieCorePath))
		{
			printf("Failed to download AurieCore!\n");
			return quit(1);
		}
	}
	else
		printf("AurieCore.dll found\n");

	string auriePatcherPath = outputStr + "\\AuriePatcher.exe";
	if (!filesystem::exists(auriePatcherPath))
	{
		printf("Downloading AuriePatcher.exe...\n");
		if (!download_file(AURIEPATCHER_URL, auriePatcherPath))
		{
			printf("Failed to download AuriePatcher!\n");
			return quit(1);
		}
	}
	else
		printf("AuriePatcher.exe found\n");

	string yyToolkitPath = outputStr + "\\mods\\Aurie\\YYToolkit.dll";
	if (!filesystem::exists(yyToolkitPath))
	{
		printf("Downloading YYToolkit.dll...\n");
		if (!download_file(YYTOOLKIT_URL, yyToolkitPath))
		{
			printf("Failed to download YYToolkit!\n");
			return quit(1);
		}
	}
	else
		printf("YYToolkit.dll found\n");

	string spiritblastPath = outputStr + "\\mods\\Aurie\\Spiritblast.dll";
	printf("Downloading Spiritblast.dll...\n");
	if (!download_file(SPIRITBLAST_URL, spiritblastPath))
	{
		printf("Failed to download Spiritblast!\n");
		return quit(1);
	}

	printf("Patching ANTONBLAST.exe...\n");
	if (!exec_and_wait(auriePatcherPath + " \"" + exePath + "\" \"" + aurieCorePath + "\" install"))
	{
		printf("Failed to execute the patcher!\n");
		return quit(1);
	}

	printf("\nInstallation done.\nIf everything went well then launching the game should now have a command prompt window open.\n");
	printf("If this doesn't happen, then something might've went wrong. Maybe try the manual installation method next.\n");
	
	return quit(0);
}