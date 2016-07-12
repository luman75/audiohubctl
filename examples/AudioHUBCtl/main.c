/*		
	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o bitmode main.c -L. -lftd2xx -Wl,-rpath /usr/local/lib
*/

#include <stdio.h>
#include "../ftd2xx.h"

#define BIT_BANG 1

int main(int argc, char *argv[])
{

	DWORD dwBytesInQueue = 0;

	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle;
	unsigned char originalMode;
	unsigned char ucMode = 0x01;
	int iport;
	char mask[8];
	int i;
	char mask_value = 0x00;
	if(argc == 3 && strlen(argv[2])==8) {
		sscanf(argv[1], "%d", &iport);
		strncpy(mask, argv[2], 8);
		
		for(i=0;i<8;i++) {
			if(mask[i] == '0') 
				mask_value = mask_value << 1;
			else
				mask_value = (mask_value << 1) | 0x01;
		}
		
	}
	else {
		printf("Usage: audiohub_ctl port_id bit_mask\n");
		iport = 0;
		exit(1);
	}
	ftStatus = FT_Open(iport, &ftHandle);
	if(ftStatus != FT_OK) {
		/* 
			This can fail if the ftdi_sio driver is loaded
		 	use lsmod to check this and rmmod ftdi_sio to remove
			also rmmod usbserial
		 */
		printf("FT_Open(%d) failed, with error %d.\n", iport, (int)ftStatus);
		printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
		printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
		return 1;
	}

	ftStatus = FT_GetBitMode(ftHandle, &originalMode);
	if(ftStatus != FT_OK) {
		printf("Failed to get bit mode (error %d)\n", (int)ftStatus);
		goto exit;
	}

	printf("Current bit mode is 0x%X\n", originalMode);
	printf("Setting bit mode to %d (asynchronous bit bang)\n", BIT_BANG);

	ftStatus = FT_SetBitMode(ftHandle, 0xFF, BIT_BANG);
	if(ftStatus != FT_OK) {
		printf("Failed to set bit mode\n");
		goto exit;
	}

	ftStatus = FT_SetBaudRate(ftHandle, 9600);
	if(ftStatus != FT_OK) {
		printf("Failed to set baud rate (error %d)\n", (int)ftStatus);
		goto exit;
	}
	
	ftStatus = FT_Write(ftHandle, &ucMode, 1, &dwBytesInQueue);
	if(ftStatus != FT_OK) {
		printf("Failed to write 1 byte (error %d)\n", (int)ftStatus);
		goto exit;
	}

	ftStatus = FT_GetBitMode(ftHandle, &ucMode);
	if(ftStatus != FT_OK) {
		printf("Failed to get bit mode\n");
		goto exit;
	}

	if(ucMode != BIT_BANG) {
		printf("Failure: new bit mode is 0x%X, but expected 0x%X\n", 
		       ucMode, BIT_BANG);
		goto exit;
	}

	printf("Success: new bit mode is 0x%X.\n", ucMode);


	printf("Got bitmask: 0x%X\n", mask_value);

	ftStatus = FT_Write(ftHandle, &mask_value, 1, &dwBytesInQueue);
	if(ftStatus != FT_OK) {
		printf("Failed to write 1 byte (error %d)\n", (int)ftStatus);
		goto exit;
	}


exit:
	FT_Close(ftHandle);
	
	return 0;
}

