#ifndef TRMOD_ERRORS_H_
#define TRMOD_ERRORS_H_

/* File Inclusions */
#include "fixedint.h"

/* Definition of error codes */
enum error_type
{
	ERROR_NONE,                     /* No error (success) */
	ERROR_MEMORY,                   /* Memory could not be allocated */
	ERROR_INVALID_PARAMETERS,       /* Invalid input parameters */
	ERROR_FILE_OPEN_FAILED,         /* Failed to open file */
	ERROR_FILE_READ_FAILED,         /* Failed to read from file */
	ERROR_FILE_WRITE_FAILED,        /* Failed to write to file */
	ERROR_FILE_CLOSE_FAILED,        /* Failed to close file */
	ERROR_REMOVE_DATA_INVALID,      /* Data to be removed doesn't exist */
	ERROR_INVALID_TYPE,             /* Invalid level type identifier */
	ERROR_UNIMPLEMENTED_LEVTYPE,    /* Level type hasn't been implemented */
	ERROR_UNIMPLEMENTED_FUNCTION,   /* Function hasn't been implemented */
	ERROR_INVALID_COMMAND,          /* Invalid command */
	ERROR_ROOM_DOESNT_EXIST,        /* Room does not exist */
	ERROR_ITEM_DOESNT_EXIST,        /* Item does not exist */
	ERROR_TOO_MANY_SUBPARAMS,       /* Too many parameters in command */
	ERROR_NOT_ENOUGH_SUBPARAMS,     /* Not enough parameters in command */
	ERROR_INVALID_FDINDEX,          /* Invalid FDIndex */
	ERROR_STATICMESH_DOESNT_EXIST,  /* Static Mesh does not exist */
	ERROR_LIGHT_DOESNT_EXIST,       /* Light does not exist */
	ERROR_SECTOR_DOESNT_EXIST,      /* Sector does not exist */
	ERROR_INVALID_SYNTAX,           /* Syntax error */
	ERROR_FDINDEX_OUT_OF_RANGE,     /* FDindex out of range */
	ERROR_UNKNOWN_FDFUNCTION,       /* FDFunction is unknown */
	ERROR_UNKNOWN_TRIGGERACTION,    /* TrigAction is unknown */
	ERROR_NO_FLOORDATA_SEMICOLON,   /* No separating semicolon in floordata */
	ERROR_NO_FLOORDATA_COORDINATES, /* No room, column and row specification */
	ERROR_INVALID_FDFUNCTION,       /* FDFunction is invalid */
	ERROR_INVALID_TRIGTYPE,         /* TrigType is invalid */
	ERROR_INVALID_TRIGACTION,       /* TrigAction is invalid */
	ERROR_SOUNDSOURCE_DOESNT_EXIST, /* Sound source does not exist */
	ERROR_VERTEX_DOESNT_EXIST,      /* Vertex does not exist */
	ERROR_RECTANGLE_DOESNT_EXIST,   /* Rectangle does not exist */
	ERROR_TRIANGLE_DOESNT_EXIST,    /* Triangle does not exist */
	ERROR_SPRITE_DOESNT_EXIST,      /* Sprite does not exist */
	ERROR_SPRITESEQ_NOT_FOUND,      /* No Sprite Sequence with that offset */
	ERROR_SPRITESEQ_NOT_FOUND_ID,   /* No Sprite Sequence with that ID */
	ERROR_ROOM_RESIZE_TO_ZERO,      /* Cannot resize room to zero */
	ERROR_VIEWPORT_DOESNT_EXIST,    /* Viewport does not exist */
	ERROR_BOX_DOESNT_EXIST,         /* Box does not exist */
	ERROR_NULL_POINTER,             /* NULL-pointer error (shouldn't happen) */
	ERROR_SILENT,                   /* Some error that doesn't print anything */
	ERROR_INVALID_NAVARRAY          /* Invalid navigation array */
};

/* Definition of error struct */
struct error
{
	int    code;       /* Error Code (see list above) */
	char  *string[1];  /* String relating to the error */
	int    integer[1]; /* Integer relating to the error */
	int16  s16[1];     /* Signed 16-bit integer relating to the error */
	uint16 u16[3];     /* Unsigned 16-bit integer relating to the error */
	int32  s32[1];     /* Signed 32-bit integer relating to the error */
	uint32 u32[1];     /* Unigned 32-bit integer relating to the error */
};

/* Function Declarations */
void printError(struct error *error);

#endif
