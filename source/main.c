#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>
#include <ogc/pad.h>
#include <wiiuse/wpad.h>
#include <di/di.h>

// Local Files
#include "globals.h"
#include "ios.h"
#include "disc_drive.h"
#include "wii.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void return_to_loader()
{
	printf("\n\nExiting...");
	exit(0);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	// we might need to have a bit of fun with IOS!
	if (!disable_ahbprot())
	{
		sleep(5);
		return -1;
	}

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	PAD_Init();
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth-20,rmode->xfbHeight-20,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	//SYS_STDIO_Report(true);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Clear the framebuffer
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

	// Make the display visible
	VIDEO_SetBlack(false);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");

	printf("Wii Disc Drive Date Checker by Aep %s\n", VERSION);
	printf("Press HOME (or START on GameCube Controller) to exit.\n\n");

	char drivedate[11] = {0};
	get_drive_date(drivedate);

	if (drivedate[0]) {
		printf("Drive Date: %s\n", drivedate);
	}
	else {
		printf("Could not get the drive date! Is the disc drive plugged into the Wii?\nReport this on the GitHub issues page.\n");
	}

	// This info was added to make sharing console drive dates with their serial numbers and model numbers easier. (Serial and model number)
	char model_number[13];
	get_wii_model(model_number);
	printf("Model Number: %s\n", model_number);

	char serial_number[13]; // 11 or 12 characters
	get_wii_serial_number(serial_number);
	printf("Serial Number: %s\n", serial_number);

	// This had to be added because people were confusing this with a production date.
	printf("\nThe date provided is not a production date for the disc drive.\nWii disc drives have different revisions, and they all have different drive\ndates in their drive firmware for each revision.\nSwapping the disc drive does affect this date if the drive you swap\nit with is not the same revision.");
	
	while(1) {

		// Call WPAD_ScanPads and PAD_ScanPads each loop, this reads the latest controller states
		PAD_ScanPads();
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed_gc = PAD_ButtonsDown(0);
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME || pressed_gc & PAD_BUTTON_START ) return_to_loader();

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}