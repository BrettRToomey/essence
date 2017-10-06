#include "../api/os.h"

#if 0
volatile int variable = 10;
OSHandle mutex;

void ThreadEntry(void *argument) {
	int i = 0xF0000000; while (++i);
	OSPrint("I am another thread in the same process.\n");
	OSPrint("The variable is equal to %d.\n", variable);
	OSPrint("My thread creation argument was %d.\n", argument);
	OSAcquireMutex(mutex);
	OSPrint("This never happens :(\n");
	// _OSSyscall(30, 0, 0, 0, 0, 0);
	while (true);
	OSTerminateThread(OS_CURRENT_THREAD);
}
#endif

void ButtonCallback(OSControl *generator, void *argument) {
	OSPrint("Button callback!! %x, %X\n", generator, argument);
	generator->action.argument = (void *) ((uintptr_t) generator->action.argument + 1);
}

extern "C" void ProgramEntry() {
	OSWindow *window = OSCreateWindow(320, 200);
	OSControl *button1 = OSCreateControl();
	OSAddControl(window, button1, 16, 16);
	
	OSEventCallback *callback = &button1->action;
	callback->callback = ButtonCallback;
	callback->argument = (void *) 0;

	while (true) {
		OSMessage message;
		OSWaitMessage(OS_WAIT_NO_TIMEOUT);

		if (OSGetMessage(&message) == OS_SUCCESS) {
			if (OS_SUCCESS == OSProcessGUIMessage(&message)) {
				continue;
			}

			// The message was not handled by the GUI.
			// We should do something with it.
			OSPrint("test_program received unhandled message of type %d\n", message.type);
		}
	}
		
#if 0
	if (OSGetCreationArgument(OS_CURRENT_PROCESS)) {
		ThreadEntry(nullptr);
	} else {
		OSProcessInformation process;
		const char *executablePath = "/os/test";
		OSCreateProcess(executablePath, OSCStringLength((char *) executablePath), &process, (void *) 1);
		OSCloseHandle(process.handle); 
		OSTerminateThread(process.mainThread.handle);
		OSCloseHandle(process.mainThread.handle);
		OSTerminateThread(OS_CURRENT_THREAD);
	}
#endif

#if 0
	mutex = OSCreateMutex();
	OSAcquireMutex(mutex);
	variable = 20;
	OSThreadInformation thread;
	OSCreateThread(ThreadEntry, &thread, (void *) 1);
	int i = 0xF0000000; while (++i);
	OSPrint("Terminating the thread...\n");
	OSTerminateThread(thread.handle);
#endif

#if 0
	if (!OSGetCreationArgument(OS_CURRENT_PROCESS)) {
		OSPrint("Creating process 2...\n");
		OSProcessInformation process;
		const char *executablePath = "/os/test";
		OSCreateProcess(executablePath, OSCStringLength((char *) executablePath), &process, (void *) 1);
		OSPrint("Created.\n");
		OSCloseHandle(process.handle);
	} else if (OSGetCreationArgument(OS_CURRENT_PROCESS) == (void *) 1) {
		OSProcessInformation process;
		const char *executablePath = "/os/test";
		OSCreateProcess(executablePath, OSCStringLength((char *) executablePath), &process, (void *) 2);
		OSPrint("I am process 2!\n");
		OSCloseHandle(process.handle);
	} else {
		OSTerminateThread(OS_CURRENT_THREAD);
	}
#endif

#if 0
	float x = 50.0f;
	x += 100.0f;
	// Floats don't work :(
	OSPrint("x = %x\n", x);

	int y = 0;
	y += 1;
	OSPrint("y = %d\n", y);
#endif

#if 0
	OSHandle mutex = OSCreateMutex();
	OSCloseHandle(mutex);
#endif

#if 0
	void *m1 = OSHeapAllocate(131072);
	OSPrint("m1 = %x\n", m1);
	void *m3 = OSHeapAllocate(0);
	OSPrint("m3 = %x\n", m3);
	void *m2 = OSHeapAllocate(1);
	OSPrint("m2 = %x\n", m2);
	void *m4 = OSHeapAllocate(250);
	OSPrint("m4 = %x\n", m4);
	void *m5 = OSHeapAllocate(250);
	OSPrint("m5 = %x\n", m5);
	void *m6 = OSHeapAllocate(250);
	OSPrint("m6 = %x\n", m6);
	void *m7 = OSHeapAllocate(250);
	OSPrint("m7 = %x\n", m7);
	void *m8 = OSHeapAllocate(250);
	OSPrint("m8 = %x\n", m8);

	OSHeapFree(m6);
	OSPrint("Freed m6\n");
	OSHeapFree(m5);
	OSPrint("Freed m5\n");
	OSHeapFree(m7);
	OSPrint("Freed m7\n");

	void *m9 = OSHeapAllocate(250);
	OSPrint("m9 = %x\n", m9);
	void *m10 = OSHeapAllocate(250);
	OSPrint("m10 = %x\n", m10);
	void *m11 = OSHeapAllocate(250);
	OSPrint("m11 = %x\n", m11);
	void *m12 = OSHeapAllocate(250);
	OSPrint("m12 = %x\n", m12);

	OSHeapFree(m11);
	OSPrint("Freed m11\n");
	OSHeapFree(m8);
	OSPrint("Freed m8\n");

	void *m13 = OSHeapAllocate(1000);
	OSPrint("m13 = %x\n", m13);
	void *m14 = OSHeapAllocate(100);
	OSPrint("m14 = %x\n", m14);

	OSHeapFree(m3);
	OSPrint("Freed m3\n");
	OSHeapFree(m2);
	OSPrint("Freed m2\n");
	OSHeapFree(m4);
	OSPrint("Freed m4\n");
	OSHeapFree(m9);
	OSPrint("Freed m9\n");
	OSHeapFree(m10);
	OSPrint("Freed m10\n");
	OSHeapFree(m12);
	OSPrint("Freed m12\n");
	OSHeapFree(m13);
	OSPrint("Freed m13\n");
	OSHeapFree(m14);
	OSPrint("Freed m14\n");
#endif

#if 0
	for (int i = 0; i < 0x10000; i++) {
		a[i] = OSHeapAllocate(16);
	}

	for (int i = 0; i < 0x10000; i++) {
		OSZeroMemory(a[i], 16);
	}

	OSPrint("nice!\n");

	for (int i = 0; i < 0x10000; i++) {
		OSHeapFree(a[i]);
	}

	OSPrint("super nice!\n");
#endif

#if 0
	{
		OSWindow window;
		OSCreateWindow(&window, 260, 160);
		OSUpdateWindow(&window);
	}
#endif

#if 0
	void *creationArgument = OSGetCreationArgument(OSCurrentProcess());
	OSMessage message = {};

	if (creationArgument) {
		int n = 0;

		while (true) {
			OSWaitMessage(OS_WAIT_NO_TIMEOUT);

			if (OSGetMessage(&message) == OS_SUCCESS) {
				Print("Received message %d: \"%s\".\n", n++, 8, message.data);
			}
		}
	} else {
		OSProcessInformation process;
		char *testProcessImage = (char *) "/os/test";
		OSCreateProcess(testProcessImage, CStringLength(testProcessImage), &process, (void *) 1);
		
		CopyMemory(message.data, (void *) "Hey guys", 8);
		OSSendMessage(process.handle, &message);

		CopyMemory(message.data, (void *) "Message!", 8);
		OSSendMessage(process.handle, &message);

		for (int i = 0; i < 2000; i++) {
			OSSendMessage(process.handle, &message);
		}
	}
#endif

#if 0
	OSHandle surface = OSCreateSurface(35, 35);
	OSFillRectangle(surface, OSRectangle(0, 35, 0, 35), OSColor(255, 128, 255));
	OSClearModifiedRegion(surface);
	OSFillRectangle(surface, OSRectangle(0, 15, 5, 25), OSColor(128, 128, 255));
	OSFillRectangle(surface, OSRectangle(20, 35, 10, 30), OSColor(128, 128, 255));
	OSCopyToScreen(surface, OSPoint(10, 10), 100);
	OSForceScreenUpdate();
#endif

#if 0
	OSHandle black = OSCreateSurface(500, 500);
	OSHandle white = OSCreateSurface(500, 500);

	OSFillRectangle(black, OSRectangle(0, 500, 0, 500), OSColor(0, 0, 0));
	OSFillRectangle(white, OSRectangle(0, 500, 0, 500), OSColor(0xFF, 0xFF, 0xFF));

	for (int i = 0; i < 60; i++) {
		OSCopyToScreen(black, OSPoint((i - 1) * 5, 0), 0);
		OSCopyToScreen(white, OSPoint((i + 0) * 5, 0), 0);
		OSForceScreenUpdate();
	}
#endif

#if 0
	OSHandle surface = OSCreateSurface(256, 256);
	OSHandle surface2 = OSCreateSurface(128, 128);

	OSFillRectangle(surface, OSRectangle(0, 256, 0, 256), OSColor(0xF4, 0xF4, 0xFF));
	OSFillRectangle(surface2, OSRectangle(0, 128, 0, 128), OSColor(0xF4, 0xF4, 0xFF));

	OSFillRectangle(surface, OSRectangle(4, 252, 4, 252), OSColor(0x24, 0x48, 0xD8));
	OSCopyToScreen(surface, OSPoint(256, 256), 0xBB);

	OSFillRectangle(surface, OSRectangle(4, 252, 4, 252), OSColor(0xD8, 0x28, 0x12));
	OSCopySurface(surface, surface2, OSPoint(64, 64));
	OSCopyToScreen(surface, OSPoint(256 + 128, 256 - 128), 0x77);
	OSCopyToScreen(surface, OSPoint(1024 - 128, 768 - 128), 0x77);

	OSForceScreenUpdate();
#endif

#if 0
	void *creationArgument = OSGetCreationArgument();

	if (creationArgument == (void *) 1) {
		uint8_t *test = (uint8_t *) OSAllocate(0x100000);
		ZeroMemory(test, 0x100000);

		Print("I'm the second process! I know that because my creation argument is %x!\n", creationArgument);
	} else {
		Print("Creation argument was null, creating another process...\n");
		OSProcessInformation process;
		char *image = (char *) "/os/test";
		OSCreateProcess(image, CStringLength(image), &process, (void *) ((uintptr_t) creationArgument + 1));
		void *creationArgument2 = OSGetCreationArgument(process.handle);
		Print("Second process PID %d, handle %d\n", process.pid, process.handle);
		Print("Creation argument for second process: %x\n", creationArgument2);
	}
#endif

#if 0
	OSProcessInformation process;
	char *testProcessImage = (char *) "/os/test";

	while (true) {
		OSCreateProcess(testProcessImage, OSCStringLength(testProcessImage), &process, nullptr);
	}
#endif

#if 0
	for (int i = 0; i < 10; i++) {
		size_t size = 0x100000;
		void *memory = OSAllocate(size);
		ZeroMemory(memory, size);
		OSFree(memory);
	}
#endif

#if 0
	OSExit(OSCurrentThread());
#else
	OSPrint("Completed test program.\n");
	while (true);
#endif
}
