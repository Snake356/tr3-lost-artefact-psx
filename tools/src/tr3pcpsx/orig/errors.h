#ifndef TR3PCPSX_ERROR_H_
#define TR3PCPSX_ERROR_H_

/* Definition of Error Codes */
#define ERROR_NONE                   0
#define ERROR_INVALID_PARAMETERS     1
#define ERROR_PC_FILE_INVALID        2
#define ERROR_PC_OPEN_FAILED         3
#define ERROR_PC_READ_FAILED         4
#define ERROR_PC_CLOSE_FAILED        5
#define ERROR_PSX_OPEN_FAILED        6
#define ERROR_PSX_READ_FAILED        7
#define ERROR_PSX_WRITE_FAILED       8
#define ERROR_PSX_CLOSE_FAILED       9
#define ERROR_MEMORY                 10
#define ERROR_REMOVE_DATA_INVALID    11
#define ERROR_INVALID_WATER_COLOUR   12
#define ERROR_ARRAY_TOO_LONG         13
#define ERROR_COLOURS_INVALID        14
#define ERROR_TOO_MANY_TEXTILES      15
#define ERROR_TEXTURE_CORNERS        16
#define ERROR_TOO_MANY_TEXTURES      17
#define ERROR_SFX_READ_FAILED        18
#define ERROR_SAMPLE_BITRATE         19
#define ERROR_WAVE_NON_PCM           20
#define ERROR_WAVE_TOO_MANY_CHANNELS 21
#define ERROR_SFX_OPEN_FAILED        22
#define ERROR_FIXED_INTEGERS         23
#define ERROR_TOO_MANY_CHUNKS        24
#define ERROR_SFX_TOO_MANY_SAMPLES   25
#define ERROR_MODULE_CLASH           26
#define ERROR_ROOM_TABLE_CLASH       28
#define ERROR_MESH_TOO_MANY_VERTICES 29
#define ERROR_TOO_MANY_PALETTES      30

/* Function Declarations */
void printError(int errorNumber, char *argv[],
                int pcparam, int psxparam, char *sfxPath);

#endif
