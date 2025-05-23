// For Wii information such as serial number and model number
#include <string.h>
#include <ogc/ipc.h>

extern int __CONF_GetTxt(const char *name, char *buf, int length);

// Get the Wii's serial number (ex. LU123456789)
// Maybe this should be split into two functions, one for CODE and one for SERNO? Whatever.

// Why either of these would ever fail would question me a lot. Missing setting.txt on the NAND?

void get_wii_serial_number(char* code) {
    s32 ret = __CONF_GetTxt("CODE", code, 4); // LU is 2 characters, however in every other region except the USA, 3 character CODE values are used (ex. LEH LAH LEF LJF LMM)
    if (ret < 0) {
        // Failed to get CODE
        strcpy(code, "??");
    }

    char serno[10];
    ret = __CONF_GetTxt("SERNO", serno, 10); // Serial number is always 9 characters (123456789), including the character in the square box
    if (ret < 0) {
        // Failed to get SERNO
        strcpy(serno, "?????????");
    }
    strcat(code, serno);
}

void get_wii_model(char* model) {
    s32 ret = __CONF_GetTxt("MODEL", model, 13);  // Always 12 characters
    if (ret < 0) {
        // Failed to get MODEL
        strcpy(model, "????????????");
    }
}