#ifndef IMPLEMENTATION

#define KERNEL

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define OS_FOLDER "/os"

#define MAX_PROCESSORS (256)
#define MAX_PATH (4096)

#define DRIVE_ACCESS_READ (0)
#define DRIVE_ACCESS_WRITE (1)

#ifdef ARCH_X86_64
#define TIMER_INTERRUPT (0x40)
#define TLB_SHOOTDOWN_IPI (0xF0)
#define KERNEL_PANIC_IPI (0x0)
#define LOW_MEMORY_MAP_START (0xFFFFFF0000000000)
#endif

// Scoped defer: http://www.gingerbill.org/article/defer-in-cpp.html
template <typename F> struct privDefer { F f; privDefer(F f) : f(f) {} ~privDefer() { f(); } };
template <typename F> privDefer<F> defer_func(F f) { return privDefer<F>(f); }
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define _Defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
#define Defer(code)   _Defer(code)

enum LogLevel {
	LOG_NONE,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
};

void KernelLog(LogLevel level, const char *format, ...);
void KernelPanic(const char *format, ...);
void Print(const char *format, ...);

#include "../api/os.h"
#define CF(x) x
#include "../api/common.cpp"

#include "linked_list.cpp"

extern "C" void ProcessorDisableInterrupts();
extern "C" void ProcessorEnableInterrupts();
extern "C" bool ProcessorAreInterruptsEnabled();
extern "C" void ProcessorHalt();
extern "C" void ProcessorIdle();
extern "C" void ProcessorOut8(uint16_t port, uint8_t value);
extern "C" uint8_t ProcessorIn8(uint16_t port);
extern "C" void ProcessorOut16(uint16_t port, uint16_t value);
extern "C" uint16_t ProcessorIn16(uint16_t port);
extern "C" void ProcessorOut32(uint16_t port, uint32_t value);
extern "C" uint32_t ProcessorIn32(uint16_t port);
extern "C" void ProcessorInvalidatePage(uintptr_t virtualAddress);
extern "C" void ProcessorAPStartup();
extern "C" void ProcessorMagicBreakpoint(...);
extern "C" void ProcessorBreakpointHelper(...);
extern "C" struct CPULocalStorage *ProcessorGetLocalStorage();
extern "C" void ProcessorSetLocalStorage(struct CPULocalStorage *cls);
extern "C" void ProcessorSendIPI(uintptr_t interrupt, bool nmi = false);
extern "C" void ProcessorDebugOutputByte(uint8_t byte);
extern "C" void ProcessorFakeTimerInterrupt();
extern "C" uint64_t ProcessorReadTimeStamp();
extern "C" void DoContextSwitch(struct InterruptContext *context, uintptr_t virtualAddressSpace, uintptr_t threadKernelStack);
extern "C" void ProcessorSetAddressSpace(uintptr_t virtualAddressSpaceIdentifier);

volatile uintptr_t ipiVector;
extern struct Spinlock ipiLock;

#ifdef ARCH_X86_64
extern "C" uint64_t ProcessorReadCR3();
extern "C" void gdt_data();

volatile uintptr_t tlbShootdownVirtualAddress;
volatile size_t tlbShootdownPageCount;
volatile uintptr_t tlbShootdownRemainingProcessors;

extern "C" CPULocalStorage *cpu_local_storage;

extern "C" bool simdSSE3Support;
extern "C" bool simdSSSE3Support;

extern "C" void SSSE3Framebuffer32To24Copy(volatile uint8_t *destination, volatile uint8_t *source, size_t pixelGroups);
#endif

// Returns the start address of the executable.
uintptr_t LoadELF(char *imageName, size_t imageNameLength);

bool HandlePageFault(uintptr_t page);

void NextTimer(size_t ms); // Receive a TIMER_INTERRUPT in ms ms.
void Delay1MS(); // Spin for 1ms. Use only during initialisation. Not thread-safe.

typedef bool (*IRQHandler)(uintptr_t interruptIndex);
bool RegisterIRQHandler(uintptr_t interruptIndex, IRQHandler handler);

struct Mutex {
	void Acquire();
	void Release();
	void AssertLocked();

	struct Thread *volatile owner;
	uintptr_t acquireAddress, releaseAddress;

	size_t handles;

	LinkedList blockedThreads;
};

struct Spinlock {
	void Acquire();
	void Release(bool force = false);
	void AssertLocked();

	volatile uint8_t state;
	volatile bool interruptsEnabled;
	struct Thread *volatile owner;
	volatile uintptr_t acquireAddress, releaseAddress;
};

typedef void (*AsyncTaskCallback)(void *argument);

struct AsyncTask {
	AsyncTaskCallback callback;
	void *argument;
	struct VirtualAddressSpace *addressSpace;
};

struct CPULocalStorage {
	struct Thread *currentThread, 
		      *idleThread, 
		      *asyncTaskThread;

	bool irqSwitchThread;
	unsigned processorID;
	bool schedulerReady;
	size_t spinlockCount;

	struct ACPIProcessor *acpiProcessor;

#define MAX_ASYNC_TASKS 1024
	volatile AsyncTask asyncTasks[MAX_ASYNC_TASKS];
	volatile size_t asyncTasksCount;

	size_t interruptRecurseCount;
	struct InterruptContext *interruptContexts[16];
};

enum KernelObjectType {
	KERNEL_OBJECT_PROCESS 	= 0x00000001,
	KERNEL_OBJECT_THREAD	= 0x00000002,
	KERNEL_OBJECT_SURFACE	= 0x00000004,
	KERNEL_OBJECT_WINDOW	= 0x00000008,
	KERNEL_OBJECT_MUTEX	= 0x00000010,
};

struct Handle {
	KernelObjectType type;
	void *object;
	volatile unsigned lock; // Must be 0 to close the handle.
			        // Incremented when the handle is used in a system call.
	volatile unsigned closing;
};

void CloseHandleToObject(void *object, KernelObjectType type);

#endif

#include "memory.cpp"
#include "scheduler.cpp"
#include "terminal.cpp"
#include "graphics.cpp"
#include "acpi.cpp"

#ifdef ARCH_X86_64
#include "x86_64.cpp"
#include "pci.cpp"
#include "ata.cpp"
#endif

#include "vfs.cpp"
#include "ext2.cpp"
#include "ps2.cpp"
#include "devices.cpp"
#include "elf.cpp"

#include "window_manager.cpp"

#include "syscall.cpp"