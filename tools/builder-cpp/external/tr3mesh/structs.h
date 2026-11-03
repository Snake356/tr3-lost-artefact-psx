#ifndef TR2PCPSX_STRUCTS_H_
#define TR2PCPSX_STRUCTS_H_

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
};

/* Struct that contains offsets and values within one room */
struct room_info
{
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
};

/* Macro that sets all variables in a level_info struct to NULL */
#define level_info_wipe(in)      \
	in.p_NumTexTiles = 0;        \
	in.v_NumTexTiles = 0;        \
	in.p_NumPalettes = 0;        \
	in.v_NumPalettes = 0;        \
	in.p_NumRooms = 0;           \
	in.v_NumRooms = 0;           \
	in.room = NULL;              \
	in.p_NumFloorData = 0;       \
	in.v_NumFloorData = 0;       \
	in.p_NumMeshData = 0;        \
	in.v_NumMeshData = 0;        \
	in.p_NumMeshPointers = 0;    \
	in.v_NumMeshPointers = 0;    \
	in.p_NumAnimations = 0;      \
	in.v_NumAnimations = 0;      \
	in.p_NumStateChanges = 0;    \
	in.v_NumStateChanges = 0;    \
	in.p_NumAnimDispatches = 0;  \
	in.v_NumAnimDispatches = 0;  \
	in.p_NumAnimCommands = 0;    \
	in.v_NumAnimCommands = 0;    \
	in.p_NumMeshTrees = 0;       \
	in.v_NumMeshTrees = 0;       \
	in.p_NumFrames = 0;          \
	in.v_NumFrames = 0;          \
	in.p_NumMoveables = 0;       \
	in.v_NumMoveables = 0;       \
	in.p_NumStaticMeshes = 0;    \
	in.v_NumStaticMeshes = 0;    \
	in.p_NumObjectTextures = 0;  \
	in.v_NumObjectTextures = 0;  \
	in.p_NumSpriteTextures = 0;  \
	in.v_NumSpriteTextures = 0;  \
	in.p_NumSpriteSequences = 0; \
	in.v_NumSpriteSequences = 0; \
	in.p_NumCameras = 0;         \
	in.v_NumCameras = 0;         \
	in.p_NumSoundSources = 0;    \
	in.v_NumSoundSources = 0;    \
	in.p_NumBoxes = 0;           \
	in.v_NumBoxes = 0;           \
	in.p_NumOverlaps = 0;        \
	in.v_NumOverlaps = 0;        \
	in.p_Zones = 0;              \
	in.p_NumAnimatedTextures = 0;\
	in.v_NumAnimatedTextures = 0;\
	in.p_NumEntities = 0;        \
	in.v_NumEntities = 0;        \
	in.p_NumCinematicFrames = 0; \
	in.v_NumCinematicFrames = 0; \
	in.p_NumDemoData = 0;        \
	in.v_NumDemoData = 0;        \
	in.p_NumSoundDetails = 0;    \
	in.v_NumSoundDetails = 0;    \
	in.p_NumSamples = 0;         \
	in.v_NumSamples = 0;         \
	in.p_NumSampleIndices = 0;   \
	in.v_NumSampleIndices = 0;

#endif
