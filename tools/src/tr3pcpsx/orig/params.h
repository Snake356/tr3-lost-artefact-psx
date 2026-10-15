#ifndef TR3PCPSX_PARAMS_H_
#define TR3PCPSX_PARAMS_H_

#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR3PCPSX */

/* Definition of Command-Line Flags */
#define FLAGS_VERBOSE           0x00000001
#define FLAGS_SKIP_ENTITIES     0x00000002
#define FLAGS_SKIP_BOXES        0x00000004
#define FLAGS_SKIP_SOUNDSOURCES 0x00000008
#define FLAGS_SKIP_CAMERAS      0x00000010
#define FLAGS_SKIP_FLOORDATA    0x00000020
#define FLAGS_SKIP_ROOMS        0x00000040
#define FLAGS_SKIP_MESHES       0x00000080
#define FLAGS_SKIP_ANIMATIONS   0x00000100
#define FLAGS_SKIP_MOVEABLES    0x00000200
#define FLAGS_SKIP_STATICMESHES 0x00000400
#define FLAGS_WATER_COLOUR      0x00000800
#define FLAGS_LICENSE           0x00001000
#define FLAGS_NO_SIGNATURE      0x00002000
#define FLAGS_SKIP_TEXTURES     0x00004000
#define FLAGS_DONT_FIX_TEXTURES 0x00008000
#define FLAGS_SKIP_AUDIO        0x00010000
#define FLAGS_SKIP_CINEMATICS   0x00020000
#define FLAGS_SKY_COLOUR        0x00040000
#define FLAGS_NEW_LEVEL         0x00080000
#define FLAGS_LANGUAGE          0x00F00000
#define FLAGS_LANGUAGE_UK       0x00000000
#define FLAGS_LANGUAGE_UK_REV1  0x00100000
#define FLAGS_LANGUAGE_US_10    0x00200000
#define FLAGS_LANGUAGE_US_11_12 0x00300000
#define FLAGS_LANGUAGE_DE       0x00400000
#define FLAGS_LANGUAGE_DE_REV1  0x00500000
#define FLAGS_LANGUAGE_FR       0x00600000
#define FLAGS_LANGUAGE_FR_REV1  0x00700000
#define FLAGS_LANGUAGE_IT       0x00800000
#define FLAGS_LANGUAGE_IT_REV1  0x00900000
#define FLAGS_LANGUAGE_SP       0x00A00000
#define FLAGS_LANGUAGE_SP_REV1  0x00B00000
#define FLAGS_LANGUAGE_JP_JP    0x00C00000
#define FLAGS_LANGUAGE_JP_EN    0x00D00000
#define FLAGS_LANGUAGE_ASIA_JP  0x00E00000
#define FLAGS_LANGUAGE_ASIA_EN  0x00F00000
#define FLAGS_WEATHER           0x03000000
#define FLAGS_WEATHER_RAIN      0x01000000
#define FLAGS_WEATHER_SNOW      0x02000000
#define FLAGS_WEATHER_SPECLE    0x03000000
#define FLAGS_SKIP_MODULES      0x04000000
#define FLAGS_ELECTRIC_DEATH    0x08000000
#define FLAGS_NO_SECOND_MIPMAP  0x10000000
#define FLAGS_NO_TEXTOPTIMISE   0x20000000
#define FLAGS_NO_MIPMAP_AT_ALL  0x40000000

/* Macro that sets a flag */
#define setflag(p) flags |= p;
/* Macro that unsets a flag */
#define unsetflag(p) flags &= (~p);
/* Macro that checks that a flag was set */
#define isflag(p) ((flags & p) != 0x00000000)
/* Macro that checks that a flag was not set */
#define notflag(p) ((flags & p) == 0x00000000)

/* Function Declarations */
int interpretParameters(int argc, char *argv[], int *pcparam, int *psxparam,
                        INTU32 *flagsptr, struct water_colour *waterColour,
                        INTU32 *skyColour, char **sfxPath);

#endif
