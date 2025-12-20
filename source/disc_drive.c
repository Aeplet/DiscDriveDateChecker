// Disc drive stuff, such as the drive date, and the revisions of it

#include <di/di.h>
#include <gccore.h>
#include <stdio.h>

#include "globals.h"

// The rest of the dates are here now too
static const u32 DMS_D2A_DATE = 0x20060526;
static const u32 D2B_DATE     = 0x20060907;
static const u32 D2C_D2E_DATE = 0x20070213;
static const u32 D3_DATE      = 0x20080714; // The date we have to consider "maybe" in some form for DVD compatibility. And some way to detect the difference between D3 and D3-2. (which as of December 20th, 2025, we still do not have)
static const u32 D4v1_DATE    = 0x20081218;
// d4v2 seems to have 2 dates? still investigating why, been wondering since March(?) 2025, maybe even February...
static const u32 D4v2_DATE_1  = 0x20091121;
static const u32 D4v2_DATE_2  = 0x20101207;

static const u32 MINI_DATE    = 0x20120629;

static const u32 WIIU_DATE_1  = 0x20110628;
static const u32 WIIU_DATE_2  = 0x20120712;

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

// now we get the revision (v1.1.0+)
void get_drive_revision(char *buffer) {
    switch (id.rel_date) {
        case DMS_D2A_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "DMS or D2A");
            break;
        case D2B_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "D2B");
            break;
        case D2C_D2E_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "D2C or D2E");
            break;
        case D3_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "D3 or D3-2");
            break;
        case D4v1_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "D4v1");
            break;
        // one day we must figure out why there are 2 d4v2 dates that seem to behave basically identically...
        case D4v2_DATE_1:
        case D4v2_DATE_2:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "D4v2");
            break;
        case MINI_DATE:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "Wii Mini Disc Drive?");
            break;
        case WIIU_DATE_1:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "Wii U Drive 1"); // better name for this plz
            break;
        case WIIU_DATE_2:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "Wii U Drive 2"); // this too!
            break;
        default:
            snprintf(buffer, DRIVE_REVISION_MAX_LENGTH, "Unknown");
            break;
    }
}