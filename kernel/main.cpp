// Timing problems:
// 	- Qemu's timer (sometimes) runs too slow on SMP.

// TODO VirtualBox gets a lot of bugs - most of which I think are to do with the AHCI implementation?

#include "kernel.h"
#define IMPLEMENTATION
#include "kernel.h"

void KernelInitialisation() {
	pmm.Initialise2();
	InitialiseObjectManager();
	vfs.Initialise();
	deviceManager.Initialise();
	graphics.Initialise(); 
	windowManager.Initialise();

	char *desktop = (char *) "/os/desktop";
	desktopProcess = scheduler.SpawnProcess(desktop, CStringLength(desktop));

	scheduler.TerminateThread(GetCurrentThread());
}

extern "C" void KernelAPMain() {
	while (!scheduler.started);
	scheduler.InitialiseAP();

	NextTimer(20);

	// The InitialiseAP call makes this an idle thread.
	// Therefore there are always enough idle threads for all processors.
	// TODO Enter low power state?
	ProcessorIdle();
}

extern "C" void KernelMain() {
	Print("---------------------------\n");
	kernelVMM.Initialise();
	memoryManagerVMM.Initialise();
	pmm.Initialise();
	scheduler.Initialise();
	acpi.Initialise(); // Initialises CPULocalStorage.
	scheduler.SpawnThread((uintptr_t) KernelInitialisation, 0, kernelProcess, false);
	KernelLog(LOG_VERBOSE, "Starting preemption...\n");
	scheduler.Start();

	KernelAPMain();
}
