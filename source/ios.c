/*
MIT License

Copyright (c) 2025 Aep

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <string.h>
#include <unistd.h>

// libogc file
#include <ogc/ipc.h>

#define AHBPROT_DISABLED			(*(vu32*)0xcd800064 == 0xFFFFFFFF)

static const u32 stage0[] = {
    0x4903468D,	/* ldr r1, =0x10100000; mov sp, r1; */
    0x49034788,	/* ldr r1, =entrypoint; blx r1; */
    /* Overwrite reserved handler to loop infinitely */
    0x49036209, /* ldr r1, =0xFFFF0014; str r1, [r1, #0x20]; */
    0x47080000,	/* bx r1 */
    0x10100000,	/* temporary stack */
    0x00000000, /* entrypoint */
    0xFFFF0014,	/* reserved handler */
};

static const u32 stage1[] = {
    0xE3A01536, // mov r1, #0x0D800000
    0xE5910064, // ldr r0, [r1, #0x64]
    0xE380013A, // orr r0, #0x8000000E
    0xE3800EDF, // orr r0, #0x00000DF0
    0xE5810064, // str r0, [r1, #0x64]
    0xE12FFF1E, // bx  lr
};

// Run the /dev/sha (Starlet ACE) exploit here to disable AHBPROT. This must be done this way because only Starlet can access the AHBPROT register.
bool disable_ahbprot()
{
    if (AHBPROT_DISABLED) {
        return true;
    }

    u32 *const mem1 = (u32 *)0x80000000;

    __attribute__((__aligned__(32)))
    ioctlv vectors[3] = {
        [1] = {
            .data = (void *)0xFFFE0028,
            .len  = 0,
        },

        [2] = {
            .data = mem1,
            .len  = 0x20,
        }
    };

    memcpy(mem1, stage0, sizeof(stage0));
    mem1[5] = (((u32)stage1) & ~0xC0000000);

    int ret = IOS_Ioctlv(0x10001, 0, 1, 2, vectors);
    if (ret < 0)
        return false;

    int tries = 1000;
    while (!AHBPROT_DISABLED) {
        usleep(1000);
        if (!tries--)
            return false;
    }

    return true;
}