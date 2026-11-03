#ifndef TR3PCPSX_STRUCTS_H_
#define TR3PCPSX_STRUCTS_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Struct that contains offsets and values for a levelfile */
struct level_info
{
	INTU32 p_NumTexTiles;         /* Offset of NumTextiles */
	INTU32 v_NumTexTiles;         /* Value of NumTextiles */
	INTU32 p_NumPalettes;         /* Offset of NumPalettes */
	INTU32 v_NumPalettes;         /* Value of NumPalettes */
	INTU32 p_NumRooms;            /* Offset of NumRooms */
	INTU16 v_NumRooms;            /* Value of NumRooms */
	struct room_info *room;       /* Offsets and values within rooms */
	INTU32 p_OutRoomTableLen;     /* Offset of OutsideRoomTableLength */
	INTU32 v_OutRoomTableLen;     /* Value of OutsideRoomTableLength */
	INTU32 p_NumRoomMeshBoxes;    /* Offset of NumRoomMeshBoundingBoxes */
	INTU32 v_NumRoomMeshBoxes;    /* Value of NumRoomMeshBoundingBoxes */
	INTU32 p_NumFloorData;        /* Offset of NumFloorData */
	INTU32 v_NumFloorData;        /* Value of NumFloorData */
	INTU32 p_NumMeshData;         /* Offset of NumMeshData */
	INTU32 v_NumMeshData;         /* Value of NumMeshData */
	INTU32 p_NumMeshPointers;     /* Offset of NumMeshPointers */
	INTU32 v_NumMeshPointers;     /* Value of NumMeshPointers */
	INTU32 p_NumAnimations;       /* Offset of NumAnimations */
	INTU32 v_NumAnimations;       /* Value of NumAnimations */
	INTU32 p_NumStateChanges;     /* Offset of NumAnimations */
	INTU32 v_NumStateChanges;     /* Value of NumAnimations */
	INTU32 p_NumAnimDispatches;   /* Offset of NumAnimDispatches */
	INTU32 v_NumAnimDispatches;   /* Value of NumAnimDispatches */
	INTU32 p_NumAnimCommands;     /* Offset of NumAnimCommands */
	INTU32 v_NumAnimCommands;     /* Value of NumAnimCommands */
	INTU32 p_NumMeshTrees;        /* Offset of NumMeshTrees */
	INTU32 v_NumMeshTrees;        /* Value of NumMeshTrees */
	INTU32 p_NumFrames;           /* Offset of NumFrames */
	INTU32 v_NumFrames;           /* Value of NumFrames */
	INTU32 p_NumMoveables;        /* Offset of NumMoveables */
	INTU32 v_NumMoveables;        /* Value of NumMoveables */
	INTU32 p_NumStaticMeshes;     /* Offset of NumStaticMeshes */
	INTU32 v_NumStaticMeshes;     /* Value of NumStaticMeshes */
	INTU32 p_NumObjectTextures;   /* Offset of NumObjectTextures */
	INTU32 v_NumObjectTextures;   /* Value of NumObjectTextures */
	INTU32 p_NumSpriteTextures;   /* Offset of NumSpriteTextures */
	INTU32 v_NumSpriteTextures;   /* Value of NumSpriteTextures */
	INTU32 p_NumSpriteSequences;  /* Offset of NumSpriteSequences */
	INTU32 v_NumSpriteSequences;  /* Value of NumSpriteSequences */
	INTU32 p_NumCameras;          /* Offset of NumCameras */
	INTU32 v_NumCameras;          /* Value of NumCameras */
	INTU32 p_NumSoundSources;     /* Offset of NumSoundSources */
	INTU32 v_NumSoundSources;     /* Value of NumSoundSources */
	INTU32 p_NumBoxes;            /* Offset of NumBoxes */
	INTU32 v_NumBoxes;            /* Value of NumBoxes */
	INTU32 p_NumOverlaps;         /* Offset of NumOverlaps */
	INTU32 v_NumOverlaps;         /* Value of NumOverlaps */
	INTU32 p_Zones;               /* Offset of Zones */
	INTU32 p_NumAnimatedTextures; /* Offset of NumAnimatedTextures */
	INTU32 v_NumAnimatedTextures; /* Value of NumAnimatedTextures */
	INTU32 p_NumEntities;         /* Offset of NumEntities */
	INTU32 v_NumEntities;         /* Value of NumEntities */
	INTU32 p_NumRoomTextures;     /* Offset of NumRoomTextures */
	INTU32 v_NumRoomTextures;     /* Value of NumRoomTextures */
	INTU32 p_NumCinematicFrames;  /* Offset of NumCinematicFrames */
	INTU16 v_NumCinematicFrames;  /* Value of NumCinematicFrames */
	INTU32 p_NumDemoData;         /* Offset of NumDemoData */
	INTU32 v_NumDemoData;         /* Value of NumDemoData */
	INTU32 p_NumSoundDetails;     /* Offset of NumSoundDetails */
	INTU32 v_NumSoundDetails;     /* Value of NumSoundDetails */
	INTU32 p_NumSamples;          /* Offset of NumSamples */
	INTU32 v_NumSamples;          /* Value of NumSamples */
	INTU32 p_NumSampleIndices;    /* Offset of NumSampleIndices */
	INTU32 v_NumSampleIndices;    /* Value of NumSampleIndices */
	INTU32 p_FirstCodeModule;     /* Offset of the first Code Module */
};

/* Struct that contains offsets and values within one room */
struct room_info
{
	/* Header */
	INT32  x;                 /* X-offset of the room */
	INT32  z;                 /* Z-offset of the room */
	INT32  yBottom;           /* Y-offset of the lowest point in the room */
	INT32  yTop;              /* Y-offset of the highest point in the room */
	/* Other Room Structs */
	INTU32 p_NumVertices;     /* Offsets of NumVertices */
	INTU16 v_NumVertices;     /* Values of NumVertices */
	INTU32 p_NumRectangles;   /* Offsets of NumRectangles */
	INTU16 v_NumRectangles;   /* Values of NumRectangles */
	INTU32 p_NumTriangles;    /* Offsets of NumTriangles */
	INTU16 v_NumTriangles;    /* Values of NumTriangles */
	INTU32 p_NumSprites;      /* Offsets of NumSprites */
	INTU16 v_NumSprites;      /* Values of NumSprites */
	INTU32 p_NumDoors;        /* Offsets of NumDoors */
	INTU16 v_NumDoors;        /* Values of NumDoors */
	INTU32 p_NumZSectors;     /* Offsets of NumZSectors */
	INTU16 v_NumZSectors;     /* Values of NumZSectors */
	INTU32 p_NumXSectors;     /* Offsets of NumXSectors */
	INTU16 v_NumXSectors;     /* Values of NumXSectors */
	INTU32 p_NumLights;       /* Offsets of NumLights */
	INTU16 v_NumLights;       /* Values of NumLights */
	INTU32 p_NumStaticMeshes; /* Offsets of NumStaticMeshes */
	INTU16 v_NumStaticMeshes; /* Values of NumStaticMeshes */
	INTU32 p_AlternateRoom;   /* Offsets of AlternateRoom */
	/* Flags */
	INTU16 flags;             /* Room Flags */
};

/* Struct that contains instructions for calculating water colour */
struct water_colour
{
	INTU16 instruction;   /* Instructions for calculating water colour */
	INTU16 red_operand;   /* Operand to calculate red with */
	INTU16 green_operand; /* Operand to calculate green with */
	INTU16 blue_operand;  /* Operand to calculate blue with */
	/*
	 * The structure of these integers is a bit complicated:
	 *
	 * instruction: 0000 0000 0000 0000
	 *               ||| |||| |||| ||||
	 *               ACO BBBB GGGG RRRR
	 * The R, G, and B are separate values for what to do with the red, green
	 * and blue colour values using the operands.
	 * 0 = Do nothing, 1 = add, 2 = subtract, 3 = multiply, 4 = divide, 5 = AND,
	 * 6 = OR, 7 = XOR, 8 = NOT, 9 = Left shift, 10 = Right shift, 11 = Modulo,
	 * 12 = Assignment, 13 = Scale. O is a simple flag for whether to allow
	 * overflow to occur or not (1=allow overflow, 0=prevent overflow). C is a
	 * simple flag for whether to compound changes (meaning using the old
	 * underwater palette as a basis, rather than the normal palette). A is a
	 * simple flag for whether to alter the normal palette instead of the
	 * underwater palette.
	 *
	 * *_operand: 0000 0000 0000 0000
	 *              ||         | ||||
	 *              CC         L LLLL
	 * C and L are separate values, and one of them must be 0. If L contains a
	 * value, the instruction operates on the value in L. If C contains a value,
	 * the instruction operates on a colour (0=none, 1=red, 2=green, 3=blue).
	 */
};

/* Macro that sets all variables in a level_info struct to NULL */
#define level_info_wipe(in)       \
	in.p_NumTexTiles = 0;         \
	in.v_NumTexTiles = 0;         \
	in.p_NumPalettes = 0;         \
	in.v_NumPalettes = 0;         \
	in.p_NumRooms = 0;            \
	in.v_NumRooms = 0;            \
	in.room = NULL;               \
	in.p_NumFloorData = 0;        \
	in.v_NumFloorData = 0;        \
	in.p_OutRoomTableLen = 0;     \
	in.v_OutRoomTableLen = 0;     \
	in.p_NumRoomMeshBoxes = 0;    \
	in.v_NumRoomMeshBoxes = 0;    \
	in.p_NumMeshData = 0;         \
	in.v_NumMeshData = 0;         \
	in.p_NumMeshPointers = 0;     \
	in.v_NumMeshPointers = 0;     \
	in.p_NumAnimations = 0;       \
	in.v_NumAnimations = 0;       \
	in.p_NumStateChanges = 0;     \
	in.v_NumStateChanges = 0;     \
	in.p_NumAnimDispatches = 0;   \
	in.v_NumAnimDispatches = 0;   \
	in.p_NumAnimCommands = 0;     \
	in.v_NumAnimCommands = 0;     \
	in.p_NumMeshTrees = 0;        \
	in.v_NumMeshTrees = 0;        \
	in.p_NumFrames = 0;           \
	in.v_NumFrames = 0;           \
	in.p_NumMoveables = 0;        \
	in.v_NumMoveables = 0;        \
	in.p_NumStaticMeshes = 0;     \
	in.v_NumStaticMeshes = 0;     \
	in.p_NumObjectTextures = 0;   \
	in.v_NumObjectTextures = 0;   \
	in.p_NumSpriteTextures = 0;   \
	in.v_NumSpriteTextures = 0;   \
	in.p_NumSpriteSequences = 0;  \
	in.v_NumSpriteSequences = 0;  \
	in.p_NumCameras = 0;          \
	in.v_NumCameras = 0;          \
	in.p_NumSoundSources = 0;     \
	in.v_NumSoundSources = 0;     \
	in.p_NumBoxes = 0;            \
	in.v_NumBoxes = 0;            \
	in.p_NumOverlaps = 0;         \
	in.v_NumOverlaps = 0;         \
	in.p_Zones = 0;               \
	in.p_NumAnimatedTextures = 0; \
	in.v_NumAnimatedTextures = 0; \
	in.p_NumEntities = 0;         \
	in.v_NumEntities = 0;         \
	in.p_NumRoomTextures = 0;     \
	in.v_NumRoomTextures = 0;     \
	in.p_NumCinematicFrames = 0;  \
	in.v_NumCinematicFrames = 0;  \
	in.p_NumDemoData = 0;         \
	in.v_NumDemoData = 0;         \
	in.p_NumSoundDetails = 0;     \
	in.v_NumSoundDetails = 0;     \
	in.p_NumSamples = 0;          \
	in.v_NumSamples = 0;          \
	in.p_NumSampleIndices = 0;    \
	in.v_NumSampleIndices = 0;    \
	in.p_FirstCodeModule = 0;

#endif
