#include "universal_semaphore.h"

#include <stdint.h>
#include <meshLibraryProcesses.h>
#include <systemDependentHelpFunctions.h>

void meshClockInc(mesh_clock_handle * mch){
	binary_semaphore_wait(mch->sem);
	++(*(mch->value));
	binary_semaphore_post(mch->sem);
}

uint32_t getCurrentMeshTimestamp(mesh_clock_handle * mch){
	binary_semaphore_wait(mch->sem);
	uint32_t val = *mch->value;
	binary_semaphore_post(mch->sem);
	return val;
}

uint32_t meshTimeDiff(uint32_t startVal, uint32_t endVal) {
    const uint32_t maxVal = UINT32_MAX;
	
    if (endVal >= startVal) {
        return endVal - startVal;
    } else {
        return maxVal - startVal + endVal + 1;
    }
}


