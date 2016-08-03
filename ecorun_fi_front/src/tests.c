#include "ff/ff.h"
#include "json/jsmn.h"
#include <stdint.h>

static uint32_t BYTEs_to_uint32(BYTE* BYTEs)
{
	return (BYTEs[0] & 0xFF) | ((BYTEs[1] & 0xFF) << 8)
			| ((BYTEs[2] & 0xFF) << 16) | ((BYTEs[3] & 0xFF) << 24);
}

typedef struct
{
	BYTE checkRoutionOnx86[446];
	struct
	{
		BYTE bootDescriptor; /* 0x80: bootable device, 0x00: non-bootable */
		BYTE firstPartitionSector[3]; /* 1st sector number */
		BYTE fileSystemDescriptor; /* 1:FAT12, 4:FAT16(less than 32MB), 5:Šg’£ DOS ƒp[ƒeƒBƒVƒ‡ƒ“,
		 6:FAT16(more 32MB), 0xb:FAT32(more 2GB),
		 0xc:FAT32 Int32h Šg’£, 0xe:FAT16 Int32h Šg’£,
		 0xf:Šg’£ DOS ƒp[ƒeƒBƒVƒ‡ƒ“‚Ì Int32h Šg’£ */
		BYTE lastPartitionSector[3];
		BYTE firstSectorNumbers[4]; /* first sector number (link to BPB sector) */
		BYTE numberOfSectors[4];
	} partitionTable[4];
	BYTE sig[2]; /* 0x55, 0xaa */
} MasterBootRecode;

typedef union
{
	/* FAT16 or FAT12 BPB */
	struct FAT16BIOSParameterBlock
	{
		BYTE jmpOpeCode[3]; /* 0xeb ?? 0x90 */
		BYTE OEMName[8];
		/* FAT16 */
		BYTE BYTEsPerSector[2]; /* BYTEs/sector */
		BYTE sectorsPerCluster; /* sectors/cluster */
		BYTE reservedSectors[2]; /* reserved sector, beginning with sector 0 */
		BYTE numberOfFATs; /* file allocation table */
		BYTE rootEntries[2]; /* root entry (512) */
		BYTE totalSectors[2]; /* partion total secter */
		BYTE mediaDescriptor; /* 0xf8: Hard Disk */
		BYTE sectorsPerFAT[2]; /* sector/FAT (FAT32 always zero: see bigSectorsPerFAT) */
		BYTE sectorsPerTrack[2]; /* sector/track (not use) */
		BYTE heads[2]; /* heads number (not use) */
		BYTE hiddenSectors[4]; /* hidden sector number */
		BYTE bigTotalSectors[4]; /* total sector number */
		/* info */
		BYTE driveNumber;
		BYTE unused;
		BYTE extBootSignature;
		BYTE serialNumber[4];
		BYTE volumeLabel[11];
		BYTE fileSystemType[8]; /* "FAT16   " */
		BYTE loadProgramCode[448];
		BYTE sig[2]; /* 0x55, 0xaa */
	} fat16;
	/* FAT32 BPB */
	struct FAT32BIOSParameterBlock
	{
		BYTE jmpOpeCode[3]; /* 0xeb ?? 0x90 */
		BYTE OEMName[8];
		/* FAT32 */
		BYTE BYTEsPerSector[2];
		BYTE sectorsPerCluster;
		BYTE reservedSectors[2];
		BYTE numberOfFATs;
		BYTE rootEntries[2];
		BYTE totalSectors[2];
		BYTE mediaDescriptor;
		BYTE sectorsPerFAT[2];
		BYTE sectorsPerTrack[2];
		BYTE heads[2];
		BYTE hiddenSectors[4];
		BYTE bigTotalSectors[4];
		BYTE bigSectorsPerFAT[4]; /* sector/FAT for FAT32 */
		BYTE extFlags[2]; /* use index zero (follows) */
		/* bit 7      0: enable FAT mirroring, 1: disable mirroring */
		/* bit 0-3    active FAT number (bit 7 is 1) */
		BYTE FS_Version[2];
		BYTE rootDirStrtClus[4]; /* root directory cluster */
		BYTE FSInfoSec[2]; /* 0xffff: no FSINFO, other: FSINFO sector */
		BYTE bkUpBootSec[2]; /* 0xffff: no back-up, other: back up boot sector number */
		BYTE reserved[12];
		/* info */
		BYTE driveNumber;
		BYTE unused;
		BYTE extBootSignature;
		BYTE serialNumber[4];
		BYTE volumeLabel[11];
		BYTE fileSystemType[8]; /* "FAT32   " */
		BYTE loadProgramCode[420];
		BYTE sig[2]; /* 0x55, 0xaa */
	} fat32;
} BIOSParameterBlock;

void test_sdcard(void)
{
	ssp_init(1); // for sdcard
	init_io_ssp1();
	ssp_clock_slow(1);

	delay_ms(20);

	disk_initialize(0);

	FATFS fs;
	FRESULT res;
	if (FR_OK == (res = f_mount(&fs, "0:/", 1)))
	{

	}
	else
	{
		usart_write_string("mount err : ");
		usart_write_uint32(res);
		usart_writeln_string("\r\n");
	}

	FIL file;
	if (FR_OK
			== (res = f_open(&file, "0:/diskio.c", FA_OPEN_EXISTING | FA_READ)))
	{
		usart_write_string("file size : ");
		usart_write_uint32(f_size(&file));
		usart_writeln_string("\r\n");

		char str[256 + 1];

		while (!f_eof(&file))
		{
			usart_write_string(f_gets(str, 256, &file));
		}
		usart_writeln_string("");

		f_close(&file);
	}
	else
	{
		usart_write_string("open err : ");
		usart_write_uint32(res);
		usart_writeln_string("\r\n");
	}

	if (FR_OK
			== (res = f_open(&file, "0:/hello.txt", FA_OPEN_ALWAYS | FA_WRITE)))
	{
		f_puts("Hello, World!", &file);
		f_puts("\r\n", &file);
		f_puts("I am ECU", &file);
		f_close(&file);
	}
	else
	{
		usart_write_string("open err : ");
		usart_write_uint32(res);
		usart_writeln_string("\r\n");
	}
}

void test_json(void)
{
	const char* json_str = "{ \"name\" : \"Jack\", \"age\" : 27 }";

	const int num_tokens = 256;
	jsmntok_t tokens[num_tokens];
	jsmn_parser parser;
	jsmn_init(&parser);

	jsmnerr_t err = jsmn_parse(&parser, json_str, (jsmntok_t*) tokens,
			num_tokens);
}
