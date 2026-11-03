#ifndef TRMOD_STRUCTS_H_
#define TRMOD_STRUCTS_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O (used for file pointer) */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Global variables */
extern const char *partnames[];

/* Definition of level type identifiers */
enum levtype
{
	TRMOD_LEVTYPE_NONE,    /* No level type */
	TRMOD_LEVTYPE_TR1_PC,  /* TR1 PC (PHD) */
	TRMOD_LEVTYPE_TR1G_PC, /* TR1 Gold PC (TUB) */
	TRMOD_LEVTYPE_TR1_PS,  /* TR1 PS (PSX) */
	TRMOD_LEVTYPE_TR1_SS,  /* TR1 SS (SAT) */
	TRMOD_LEVTYPE_TR2_PC,  /* TR2 PC (TR2) */
	TRMOD_LEVTYPE_TR2_PS,  /* TR2 PS (PSX) */
	TRMOD_LEVTYPE_TR3_PC,  /* TR3 PC (TR2) */
	TRMOD_LEVTYPE_TR3_PS,  /* TR3 PS (PSX) */
	TRMOD_LEVTYPE_TR4_PC,  /* TR4 PC (TR4) */
	TRMOD_LEVTYPE_TR4_PS,  /* TR4 PS (WAD) */
	TRMOD_LEVTYPE_TR5_PC,  /* TR5 PC (TRC) */
	TRMOD_LEVTYPE_TR5_PS   /* TR5 PS (WAD) */
};

/*
 * List of all the components of a level file
 */
enum levparts
{
	/* Elements that are part of a room */
	R_NUMVERTICES,      /* Number of vertices in room */
	R_NUMRECTANGLES,    /* Number of rectangles in room */
	R_NUMTRIANGLES,     /* Number of triangles in room */
	R_NUMSPRITES,       /* Number of sprites in room */
	R_NUMDOORS,         /* Number of Doors in room */
	R_NUMZSECTORS,      /* Number of Z-Sectors in room */
	R_NUMXSECTORS,      /* Number of X-Sectors in room */
	R_NUMLIGHTS,        /* Number of lights in room */
	R_NUMSTATICMESHES,  /* Number of Static meshes in room */
	R_ALTROOM,          /* Alternate room in room */
	/* Number of the above */
	NUMROOMPARTNUMS,
	
	/* Elements of which the offset is to be remembered */
	NUMTEXTILES,        /* Number of Texture tiles */
	NUMPALETTES,        /* Number of Palettes */
	NUMROOMS,           /* Number of rooms */
	OUTROOMTABLE,       /* OutsideRoomTable */
	NUMROOMMESHBOXES,   /* Number of room mesh boxes */
	NUMFLOORDATA,       /* Number of FloorData entries */
	NUMMESHDATA,        /* Size of mesh data */
	NUMMESHPOINTERS,    /* Number of mesh pointers */
	NUMANIMATIONS,      /* Number of animations */
	NUMSTATECHANGES,    /* Number of state changes */
	NUMANIMDISPATCHES,  /* Number of animation dispatches */
	NUMANIMCOMMANDS,    /* Number of animation commands */
	NUMMESHTREES,       /* Number of mesh trees */
	NUMFRAMES,          /* Number of frames */
	NUMMOVEABLES,       /* Number of moveables */
	NUMSTATICMESHES,    /* Number of static meshes */
	NUMOBJECTTEXTURES,  /* Number of object textures */
	NUMSPRITETEXTURES,  /* Number of sprite textures */
	NUMSPRITESEQUENCES, /* Number of sprite sequences */
	NUMCAMERAS,         /* Number of camera's */
	NUMSOUNDSOURCES,    /* Number of sound sources */
	NUMBOXES,           /* Number of boxes */
	NUMOVERLAPS,        /* Number of overlaps */
	ZONE,               /* Zone */
	NUMANIMTEXTURES,    /* Number of animated texutres */
	NUMENTITIES,        /* Number of Entities */
	NUMROOMTEX,         /* Number of room textures */
	NUMCINEMATICFRAMES, /* Number of cinematic frames */
	NUMDEMODATA,        /* Number of demo data */
	NUMSOUNDDETAILS,    /* Number of Sound Details */
	NUMSAMPLES,         /* Number of samples */
	NUMSAMPLEINDICES,   /* Number of sample indices */
	CODEMODULE,         /* Code module */
	/* Number of the above */
	NUMKEEPPARTS,
	
	/* Elements of which only the sizes are relevant */
	R_VERTEX,           /* Vertex in room */
	R_RECTANGLE,        /* Rectangle in room */
	R_RECTTEX,          /* Texture of a rectangle in a room */
	R_TRIANGLE,         /* Triangle in room */
	R_SPRITE,           /* Sprite in room */
	R_DOOR,             /* Door in room */
	R_SECTOR,           /* Sector in room */
	R_ROOMLIGHT,        /* RoomLight in room */
	R_LIGHT,            /* Light in room */
	R_STATICMESH,       /* Static mesh in room */
	/* Number of the above */
	NUMROOMPARTS,
	
	/* Outside of room */
	TEXTILE,            /* Texture Tile */
	PALETTE,            /* Palette */
	ROOMMESHBOX,        /* Room mesh box */
	FLOORDATA,          /* FloorData */
	MESHDATA,           /* Mesh Data */
	MESHPOINTER,        /* Mesh Pointer */
	ANIMATION,          /* Animation */
	STATECHANGE,        /* State change */
	ANIMDISPATCH,       /* Animation Dispatch */
	ANIMCOMMAND,        /* Animation Command */
	MESHTREE,           /* Mesh Tree */
	FRAME,              /* Frame */
	MOVEABLE,           /* Moveable */
	STATICMESH,         /* Static Mesh */
	OBJECTTEXTURE,      /* Object Texture */
	SPRITETEXTURE,      /* Sprite Texture */
	SPRITESEQUENCE,     /* Sprite Sequence */
	CAMERA,             /* Camera */
	SOUNDSOURCE,        /* Sound Source */
	BOX,                /* Box */
	OVERLAP,            /* Overlap */
	ANIMATEDTEXTURE,    /* Animated Texture */
	ENTITY,             /* Entity */
	ROOMTEXTURE,        /* Room Texture */
	CINEMATICFRAME,     /* Cinematic Frame */
	DEMODATA,           /* Demo Data */
	SOUNDDETAIL,        /* Sound Detail */
	SAMPLE,             /* Samples */
	SAMPLEINDEX,        /* Sample Index */
	/* Number of the above elements */
	NUMLEVPARTS,
	
	/* Special codes */
	START_ROOM,  /* Start of the room struct */
	END_ROOM,    /* End of the room struct */
	END_LEVEL,   /* End of the level */
	SKIP_BYTES,  /* Bytes to be skipped */
	SKIP_READ32, /* Bytes to be skipped from read 32-bit */
	SKIP_VAR,    /* Bytes to be skipped from variable */
	
	/* Special codes for conditionals */
	N_IF_EQUAL,   /* Start of a conditional */
	N_IF_GREATER, /* Start of a conditional */
	N_IF_SMALLER, /* Start of a conditional */
	N_THEN,       /* Consequence of a conditional */
	N_ELSE,       /* Reverse of a conditional */
	N_ENDIF,      /* End of a conditional */
	N_IMM,        /* The next number is an immediate */
	N_READ16,     /* Read a 16-bit integer */
	N_READ32,     /* Read a 32-bit integer */
	N_AND,        /* Perform an AND on the next parameters */
	N_OR,         /* Perform an OR on the next parameters */
	
	/* Special codes */
	VAR_TO_PARAM0, /* Sets variable in next integer to c_params[0] */
	NAV_SET_FLAG   /* Sets a flag */
};

/* Struct that contains offsets and values for a levelfile */
struct level
{
	FILE *file;                   /* File pointer to the level */
	char *path;                   /* Path to the level file */
	uint8 *inmem;                 /* In-memory-copy of the level */
	uint32 type;                  /* Type of level */
	int endian;                   /* File endianness differs from host */
	char *typestring;             /* String-representation of leveltype */
	char *command;                /* Command for running this program */
	uint32 *nav;                  /* Navigation array */
	uint32 nav_flags;             /* Navigation flags */
	uint32 offset[NUMKEEPPARTS];  /* Offset of variables in the level */
	uint32 value[NUMKEEPPARTS];   /* Values of variables in the level */
	uint32 partsize[NUMLEVPARTS]; /* Sizes of the parts of the level */
	int partnum[NUMLEVPARTS];     /* Associated num- to element */
	uint16 numRooms;              /* Number of rooms */
	struct room *room;            /* Offsets and values within rooms */
};

/* Struct that contains offsets and values within one room */
struct room
{
	/* Header */
	int32  x;                 /* X-offset of the room */
	int32  z;                 /* Z-offset of the room */
	int32  yBottom;           /* Y-offset of the lowest point in the room */
	int32  yTop;              /* Y-offset of the highest point in the room */
	uint32 offset[NUMROOMPARTNUMS];
	uint16 value[NUMROOMPARTNUMS];
	uint32 startpos;          /* Where this room begins */
	/* Flags */
	uint16 flags;             /* Room Flags */
};

#endif

/*
Structure of the Navigation Arrays
==================================

The navigation arrays are a form of primitive interpreted language, in which
every instruction consists of a 32-bit word, and they are linearly processed
one at a time.

All before NUMROOMPARTNUMS: A num-something to be read into memory and of which
	its position and value should be recorded, and inside a room.
	Reads number of bytes fitting its size from file, and keeps its offset and
	value inside the room-struct.
All before NUMKEEPPARTS: A num-something to be read into memory and of which
	its position and value should be recorded.
	Reads number of bytes fitting its size from file, and keeps its offset and
	value.
All before NUMLEVPARTS: The actual data that makes up the level. Skips the
	number of bytes the struct is long multiplied with the associated
	Num-something.
START_ROOM: Marks the start of the room-struct.
END_ROOM: Marks the end of the room-struct. When encountered, it increments the
	current room count. It checks whether the room counter is then equal to
	NumRooms. If it isn't, it jumps back to START_ROOM.
END_LEVEL: Marks the end of the level. When encountered, the parsing stops.
SKIP_BYTES: Special code that causes a specific number of bytes to be skipped.
	When encountered, it takes the next word in the navigation array and skips
	the number of bytes equal to that word's value. (In effect this command is
	two words wide.)
SKIP_READ32: Special code that causes a specific number of bytes to be skipped.
	When encountered, it reads 4 bytes from the file, and takes the next word
	in the navigation array, multiplies the two, and skips that number of bytes.

N_IF_EQUAL: Executes a part of the navigation array if two variables are equal.
	When encountered, it wipes the parameter variables, and executes the part
	of the navigation array between N_THEN and (N_ELSE or N_ENDIF) if the first
	two parameter variables equal each other.
N_IF_GREATER: Executes a part of the navigation array if one var is greater.
	When encountered, it wipes the parameter variables, and executes the part
	of the navigation array between N_THEN and (N_ELSE or N_ENDIF) if the first
	parameter variable is greater than the second parameter variable.
N_IF_SMALLER: Executes a part of the navigation array if one var is greater.
	When encountered, it wipes the parameter variables, and executes the part
	of the navigation array between N_THEN and (N_ELSE or N_ENDIF) if the first
	parameter variable is smaller than the second parameter variable.
N_THEN: The moment when the N_IF_* condition is tested. If the condition
	applies, it executes the navigation array until (N_ELSE or N_ENDIF) is
	encountered. If the condition doesn't apply, part of the navigation array
	is skipped until (N_ELSE of N_ENDIF) is encountered.
N_ELSE: Executes if the IF-condition fails. If it is skipped-to by N_THEN, the
	contents are just executed. If encountered in normal execution, it skips
	part of the navigation array until N_ENDIF is encountered.
N_ENDIF: Denotes the end of a conditional.
Between N_IF_* and N_THEN, every word in the navigation array is considered a
	reference to a variable in the level, which is placed in the next unused
	parameter variable.
N_IMM: Takes the next word in the navigation array, and sets the next parameter
	variable equal to it.
N_READ16: Reads 2 bytes from the level file, and sets the next parameter
	variable equal to what was read.
N_READ32: Reads 4 bytes from the level file, and sets the next parameter
	variable equal to what was read.
N_AND: Performs an AND-operation on the previous two parameters. Takes the last
	two used parameter variables, performs an AND-operation between them,
	stores the result in the first of the two, and clears the second.
N_OR: Performs an OR-operation on the previous two parameters. Takes the last
	two used parameter variables, performs an OR-operation between them,
	stores the result in the first of the two, and clears the second.

VAR_TO_PARAM0: Takes the next word in the navigation array, interprets that as
	a reference to a variable, and sets that variable equal to the current value
	in the first parameter variable used in if-statements.
NAV_SET_FLAG: Sets a navigation flag. Takes the next word in the navigation
	array, and performs an OR-operation on the navigation_flags and it.
*/
