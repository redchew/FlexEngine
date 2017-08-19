#include "stdafx.h"

#include <windows.h>

#include "FlexEngine.h"

// Memory leak checking includes
#if defined(DEBUG) | defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC  
	#include <stdlib.h>  
	#include <crtdbg.h>  
#endif

int main(int argc, char *argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	// Notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(881);
#endif

	{
		FlexEngine* engineInstance = new FlexEngine();
		engineInstance->Initialize();
		engineInstance->UpdateAndRender();
		SafeDelete(engineInstance);
	}

	system("PAUSE");
	exit(EXIT_SUCCESS);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	return main(0, {});
}