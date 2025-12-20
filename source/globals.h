// "Config"
#define VERSION "v1.1.0"

// everything but a "config"
#define AHBPROT_DISABLED			(*(vu32*)0xcd800064 == 0xFFFFFFFF)

#define DRIVE_REVISION_MAX_LENGTH   25