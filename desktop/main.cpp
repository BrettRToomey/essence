#include "../api/os.h"

#define OS_MANIFEST_DEFINITIONS
#include "../bin/os/desktop.manifest.h"

char *errorMessages[] = {
	(char *) "INVALID_BUFFER",
	(char *) "UNKNOWN_SYSCALL",
	(char *) "INVALID_MEMORY_REGION",
	(char *) "MEMORY_REGION_LOCKED_BY_KERNEL",
	(char *) "PATH_LENGTH_EXCEEDS_LIMIT",
	(char *) "INVALID_HANDLE",
	(char *) "MUTEX_NOT_ACQUIRED_BY_THREAD",
	(char *) "MUTEX_ALREADY_ACQUIRED",
	(char *) "BUFFER_NOT_ACCESSIBLE",
	(char *) "SHARED_MEMORY_REGION_TOO_LARGE",
	(char *) "SHARED_MEMORY_STILL_MAPPED",
	(char *) "COULD_NOT_LOAD_FONT",
	(char *) "COULD_NOT_DRAW_FONT",
	(char *) "COULD_NOT_ALLOCATE_MEMORY",
	(char *) "INCORRECT_FILE_ACCESS",
	(char *) "TOO_MANY_WAIT_OBJECTS",
	(char *) "INCORRECT_NODE_TYPE",
	(char *) "PROCESSOR_EXCEPTION",
	(char *) "INVALID_PANE_CHILD",
	(char *) "INVALID_PANE_OBJECT",
	(char *) "UNSUPPORTED_CALLBACK",
	(char *) "MISSING_CALLBACK",
	(char *) "UNKNOWN",
	(char *) "RECURSIVE_BATCH",
	(char *) "CORRUPT_HEAP",
};

OSCallbackResponse ProcessDebuggerMessage(OSObject _object, OSMessage *message) {
	(void) _object;
	OSCallbackResponse response = OS_CALLBACK_NOT_HANDLED;

	switch (message->type) {
		case OS_MESSAGE_PROGRAM_CRASH: {
			// Terminate the crashed process.
			OSTerminateProcess(message->crash.process);
			OSCloseHandle(message->crash.process);

			char crashMessage[256];
			size_t crashMessageLength;
			OSError code = message->crash.reason.errorCode;

			if (code < OS_FATAL_ERROR_COUNT) {
				crashMessageLength = OSFormatString(crashMessage, 256, 
						"Error code: %d (%s)", code, OSCStringLength(errorMessages[code]), errorMessages[code]);
			} else {
				crashMessageLength = OSFormatString(crashMessage, 256, 
						"Error code: %d (user defined error)", code);
			}

			OSWindowSpecification specification = {};
			specification.width = 320;
			specification.height = 200;
			specification.minimumWidth = 160;
			specification.minimumHeight = 100;
			specification.flags = OS_CREATE_WINDOW_ALERT;
			specification.title = (char *) "Program Crashed";
			specification.titleBytes = OSCStringLength(specification.title);
			OSObject window = OSCreateWindow(&specification);

			OSObject content = OSCreateGrid(1, 1, 0);
			OSSetRootGrid(window, content);
			OSAddControl(content, 0, 0, OSCreateLabel(crashMessage, crashMessageLength), 0);

			response = OS_CALLBACK_HANDLED;
		} break;

		default: {
			response = OS_CALLBACK_NOT_HANDLED;
		} break;
	}

	return response;
}

bool LoadImageIntoSurface(char *cPath, OSHandle surface, bool center) {
	size_t fileSize;
	uint8_t *loadedFile = (uint8_t *) OSReadEntireFile(cPath, OSCStringLength(cPath), &fileSize);

	if (!loadedFile) {
		return false;
	} else {
		int imageX, imageY, imageChannels;
		uint8_t *image = stbi_load_from_memory(loadedFile, fileSize, &imageX, &imageY, &imageChannels, 4);

		if (!image) {
			OSHeapFree(loadedFile);
			return false;
		} else {
			OSLinearBuffer buffer; 
			OSGetLinearBuffer(surface, &buffer);

			void *bitmap = OSMapObject(buffer.handle, 0, buffer.height * buffer.stride, OS_MAP_OBJECT_READ_WRITE);
			int xOffset = 0, yOffset = 0;

			if (center) {
				if (imageX > (int) buffer.width) {
					xOffset = imageX / 2 - buffer.width / 2;
				}

				if (imageY > (int) buffer.height) {
					yOffset = imageY / 2 - buffer.height / 2;
				}
			}

			for (uintptr_t y = 0; y < buffer.height; y++) {
				for (uintptr_t x = 0; x < buffer.width; x++) {
					uint8_t *destination = (uint8_t *) bitmap + y * buffer.stride + x * 4;
					uint8_t *source = image + (y + yOffset) * imageX * 4 + (x + xOffset) * 4;

					destination[2] = source[0];
					destination[1] = source[1];
					destination[0] = source[2];
					destination[3] = source[3];
				}
			}

			OSInvalidateRectangle(surface, OS_MAKE_RECTANGLE(0, imageX, 0, imageY));
			OSHeapFree(image);
			OSFree(bitmap);
			OSCloseHandle(buffer.handle);
		}

		OSHeapFree(loadedFile);
	}

	return true;
}

extern "C" void ProgramEntry() {
	LoadImageIntoSurface((char *) "/os/UISheet.png", OS_SURFACE_UI_SHEET, false);

#if 0
	LoadImageIntoSurface((char *) "/os/sample_images/Nebula.jpg", OS_SURFACE_WALLPAPER, true);
#else
	OSHandle surface = OS_SURFACE_WALLPAPER;
	OSLinearBuffer buffer; OSGetLinearBuffer(surface, &buffer);
	OSFillRectangle(surface, OS_MAKE_RECTANGLE(0, buffer.width, 0, buffer.height), OSColor(32, 64, 128));
#endif

	OSRedrawAll();

	{
		for (int i = 0; i < 1; i++) {
			const char *path = "/os/calculator";
			// const char *path = "/os/test";
			OSProcessInformation process;
			OSCreateProcess(path, OSCStringLength((char *) path), &process, nullptr);
			OSCloseHandle(process.mainThread.handle);
			OSCloseHandle(process.handle);
		}
	}

	OSSetCallback(OS_CALLBACK_DEBUGGER_MESSAGES, OS_MAKE_CALLBACK(ProcessDebuggerMessage, nullptr));
	OSProcessMessages();
}
