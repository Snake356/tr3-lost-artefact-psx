#ifndef TRMOD_ERRORS_H_
#define TRMOD_ERRORS_H_

/* File Inclusions */
#include "fixedint.h"

/* Definition of error codes */
#define ERROR_NONE                     0
#define ERROR_MEMORY                   1
#define ERROR_INVALID_PARAMETERS       2
#define ERROR_FILE_OPEN_FAILED         3
#define ERROR_FILE_READ_FAILED         4
#define ERROR_FILE_WRITE_FAILED        5
#define ERROR_FILE_CLOSE_FAILED        6
#define ERROR_REMOVE_DATA_INVALID      7
#define ERROR_INVALID_TYPE             8
#define ERROR_UNIMPLEMENTED_LEVTYPE    9
#define ERROR_INVALID_COMMAND          10
#define ERROR_ROOM_DOESNT_EXIST        11
#define ERROR_ITEM_DOESNT_EXIST        12
#define ERROR_TOO_MANY_SUBPARAMS       13
#define ERROR_NOT_ENOUGH_SUBPARAMS     14
#define ERROR_INVALID_FDINDEX          15
#define ERROR_STATICMESH_DOESNT_EXIST  16
#define ERROR_LIGHT_DOESNT_EXIST       17
#define ERROR_SECTOR_DOESNT_EXIST      18
#define ERROR_INVALID_SYNTAX           19
#define ERROR_FDINDEX_OUT_OF_RANGE     20
#define ERROR_UNKNOWN_FDFUNCTION       21
#define ERROR_UNKNOWN_TRIGGERACTION    22
#define ERROR_NO_FLOORDATA_SEMICOLON   23
#define ERROR_NO_FLOORDATA_COORDINATES 24
#define ERROR_INVALID_FDFUNCTION       25
#define ERROR_INVALID_TRIGTYPE         26
#define ERROR_INVALID_TRIGACTION       27
#define ERROR_SOUNDSOURCE_DOESNT_EXIST 28
#define ERROR_VERTEX_DOESNT_EXIST      29
#define ERROR_RECTANGLE_DOESNT_EXIST   30
#define ERROR_TRIANGLE_DOESNT_EXIST    31
#define ERROR_SPRITE_DOESNT_EXIST      32
#define ERROR_SPRITESEQ_NOT_FOUND      33
#define ERROR_SPRITESEQ_NOT_FOUND_ID   34
#define ERROR_ROOM_RESIZE_TO_ZERO      35
#define ERROR_VIEWPORT_DOESNT_EXIST    36
#define ERROR_BOX_DOESNT_EXIST         37
#define ERROR_NULL_POINTER             38
#define ERROR_MOVABLE_DOESNT_EXIST     39

/* Definition of error struct */
struct error
{
	int    code;       /* Error Code (see list above) */
	char  *string[1];  /* String relating to the error */
	int    integer[1]; /* Integer relating to the error */
	INT16  s16[1];     /* Signed 16-bit integer relating to the error */
	INTU16 u16[3];     /* Unsigned 16-bit integer relating to the error */
	INT32  s32[1];     /* Signed 32-bit integer relating to the error */
	INTU32 u32[1];     /* Unigned 32-bit integer relating to the error */
};

/* Function Declarations */
void printError(struct error *error);

/* Definition of a macro that wipes the error struct */
#define error_struct_wipe(error) \
	error.code = ERROR_NONE;     \
	error.string[0] = NULL;      \
	error.integer[0] = 0;        \
	error.s16[0] = 0x0000;       \
	error.u16[0] = 0x0000;       \
	error.u16[1] = 0x0000;       \
	error.u16[2] = 0x0000;       \
	error.s32[0] = 0x00000000;   \
	error.u32[0] = 0x00000000;

#endif
