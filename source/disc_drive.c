// Disc drive stuff, such as the drive date, and the revisions of it

#include <di/di.h>
#include <stdio.h>

// thanks blackb0x
void get_drive_date(char *drivedate) {
    DI_Init(); // Requires AHBPROT disabled
    DI_DriveID id;
    if (DI_Identify(&id) == 0) {
        char temp[9] = {0};
        sprintf(temp, "%08x", id.rel_date);
        sprintf(drivedate, "%.4s/%.2s/%.2s", temp, temp + 4, temp + 6); // ISO 8601 format
    }
    DI_Close();
}