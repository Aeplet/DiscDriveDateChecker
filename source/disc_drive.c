// Disc drive stuff, such as the drive date, and the revisions of it

#include <di/di.h>
#include <gccore.h>

#include <stdio.h>

// The date we have to consider "maybe" in some form. And some way to detect the difference between D3 and D3-2.
static const u32 D3_DATE = 0x20080714;

DI_DriveID id;

// thanks blackb0x
void get_drive_date(char *drivedate) {
    DI_Init(); // Requires AHBPROT disabled
    if (DI_Identify(&id) == 0) {
        char temp[9] = {0};
        sprintf(temp, "%08x", id.rel_date);
        sprintf(drivedate, "%.4s/%.2s/%.2s", temp, temp + 4, temp + 6); // ISO 8601 format
    }
    DI_Close();
}

// 2008/07/14... my nemesis.
// opting out of a bool here.
// 0 - true
// 1 -- false
// 2 -- 2008/07/14 (currently say maybe)
u8 is_dvd_compatible(void)
{
    return (id.rel_date == D3_DATE) ? 2 : (id.rel_date > D3_DATE) ? 1 : 0;
}