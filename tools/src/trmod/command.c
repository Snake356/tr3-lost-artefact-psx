/*
 * Library that manages interpreting commands for TRMOD
 * Copyright (C) 2022 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard input and output */
#include <stdlib.h>   /* Standard library of utility functions */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TRMOD */
#include "errors.h"   /* Error Handling for TRMOD */
#include "wrap.h"     /* Wrapper functions for actual functionality */
#include "util.h"     /* General functions for TRMOD */
#include "command.h"  /* Interpretation of commands */
#include "trmodio.h"  /* TRMOD input and output */

/* Characters allowed in the input */
static const char *allowed_chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz()[],*:/\\-";

/* Names of the commands */
static const char *cmd_string[] =
{
	"nothing",
	
	"getitem",
	"cgetitem",
	"rgetitem",
	"hgetitem",
	"additem",
	"replaceitem",
	"removeitem",
	
	"getstaticmesh",
	"cgetstaticmesh",
	"rgetstaticmesh",
	"hgetstaticmesh",
	"addstaticmesh",
	"replacestaticmesh",
	"removestaticmesh",
	
	"getlight",
	"cgetlight",
	"rgetlight",
	"hgetlight",
	"addlight",
	"replacelight",
	"removelight",
	
	"getsoundsource",
	"cgetsoundsource",
	"rgetsoundsource",
	"hgetsoundsource",
	"addsoundsource",
	"replacesoundsource",
	"removesoundsource",
	
	"getfloordata",
	"cgetfloordata",
	"rgetfloordata",
	"replacefloordata",
	"removefloordata",
	
	"getfloor",
	"cgetfloor",
	"rgetfloor",
	"hgetfloor",
	"floor",
	
	"getceiling",
	"cgetceiling",
	"rgetceiling",
	"hgetceiling",
	"ceiling",
	
	"getzone",
	"cgetzone",
	"rgetzone",
	"zone",
	
	"getstepsound",
	"cgetstepsound",
	"rgetstepsound",
	"stepsound",
	
	"getvertex",
	"cgetvertex",
	"rgetvertex",
	"hgetvertex",
	"addvertex",
	"replacevertex",
	"removevertex",
	"removegeometry",
	
	"getrectangle",
	"cgetrectangle",
	"rgetrectangle",
	"hgetrectangle",
	"addrectangle",
	"replacerectangle",
	"removerectangle",
	
	"gettriangle",
	"cgettriangle",
	"rgettriangle",
	"hgettriangle",
	"addtriangle",
	"replacetriangle",
	"removetriangle",
	
	"getsprite",
	"cgetsprite",
	"rgetsprite",
	"hgetsprite",
	"addsprite",
	"replacesprite",
	"removesprite",
	
	"getviewport",
	"cgetviewport",
	"rgetviewport",
	"addviewport",
	"replaceviewport",
	"removeviewport",
	
	"getroomlight",
	"cgetroomlight",
	"rgetroomlight",
	"roomlight",
	
	"getaltroom",
	"cgetaltroom",
	"rgetaltroom",
	"altroom",
	
	"getroomflags",
	"cgetroomflags",
	"rgetroomflags",
	
	"getwater",
	"cgetwater",
	"rgetwater",
	"water",
	"nowater",
	
	"getsfxalways",
	"cgetsfxalways",
	"rgetsfxalways",
	"sfxalways",
	"notsfxalways",
	
	"getpitchshift",
	"cgetpitchshift",
	"rgetpitchshift",
	"pitchshift",
	"nopitchshift",
	
	"getsky",
	"cgetsky",
	"rgetsky",
	"sky",
	"nosky",
	
	"getdynamic",
	"cgetdynamic",
	"rgetdynamic",
	"dynamic",
	"nodynamic",
	
	"getwind",
	"cgetwind",
	"rgetwind",
	"wind",
	"nowind",
	
	"getinside",
	"cgetinside",
	"rgetinside",
	"inside",
	"notinside",
	
	"getswamp",
	"cgetswamp",
	"rgetswamp",
	"swamp",
	"noswamp",
	
	"getroompos",
	"cgetroompos",
	"rgetroompos",
	"moveroom",
	
	"getroomsize",
	"cgetroomsize",
	"rgetroomsize",
	"resizeroom",
	
	"getbox",
	"cgetbox",
	"rgetbox",
	"addbox",
	"replacebox",
	"removebox",
	
	"addroom",
	"removerooms",
	"hexdump",
	"vardump"
};

/* Usage of the commands */
static const char *cmd_usage[] =
{
	"\0", /* nothing */
	
	"(item)", /* getitem */
	"(item)", /* cgetitem */
	"(item)", /* rgetitem */
	"(item)", /* hgetitem */
	"(id, room, x, z, y, angle, intensity1, <intensity2>, flags)", /* additem */
	"(item, <id>, <room>, <x>, <z>, <y>, <angle>, <intensity1>, <intensity2>, <flags>)", /* replaceitem */
	"(item, <raw>)", /* removeitem */
	
	"(room, mesh)", /* getstaticmesh */
	"(room, mesh)", /* cgetstaticmesh */
	"(room, mesh)", /* rgetstaticmesh */
	"(room, mesh)", /* hgetstaticmesh */
	"(id, room, x, z, y, angle, intensity1, <intensity2>)", /* addstaticmesh */
	"(room, mesh, <id>, <x>, <z>, <y>, <angle>, <intensity1>, <intensity2>)", /* replacestaticmesh */
	"(room, mesh)", /* removestaticmesh */
	
	"(room, light)", /* getlight */
	"(room, light)", /* cgetlight */
	"(room, light)", /* rgetlight */
	"(room, light)", /* hgetlight */
	"(room, x, z, y, intensity1, <intensity2>, fade1, <fade2>)", /* addlight */
	"(room, light, <x>, <z>, <y>, <intensity1>, <intensity2>, <fade1>, <fade2>)", /* replacelight */
	"(room, light)", /* removelight */
	
	"(soundsource)", /* getsoundsource */
	"(soundsource)", /* cgetsoundsource */
	"(soundsource)", /* rgetsoundsource */
	"(soundsource)", /* hgetsoundsource */
	"(id, room, x, z, y, room/altroom/both)", /* addsoundsource */
	"(soundsource, <id>, <room>, <x>, <z>, <y>, <room/altroom/both>)", /* replacesoundsource */
	"(soundsource)", /* removesoundsource */
	
	"(room, column, row)", /* getfloordata */
	"(room, column, row)", /* cgetfloordata */
	"(room, column, row)", /* rgetfloordata */
	"(room, column, row: floordata)", /* replacefloordata */
	"(room, column, row)", /* removefloordata */
	
	"(room, column, row)", /* getfloor */
	"(room, column, row)", /* cgetfloor */
	"(room, column, row)", /* rgetfloor */
	"(room, column, row)", /* hgetfloor */
	"(room, column, row, height)", /* floor */
	
	"(room, column, row)", /* getceiling */
	"(room, column, row)", /* cgetceiling */
	"(room, column, row)", /* rgetceiling */
	"(room, column, row)", /* hgetceiling */
	"(room, column, row height)", /* ceiling */
	
	"(room, column, row)", /* getzone */
	"(room, column, row)", /* cgetzone */
	"(room, column, row)", /* rgetzone */
	"(room, column, row, zone)", /* zone */
	
	"(room, column, row)", /* getstepsound */
	"(room, column, row)", /* cgetstepsound */
	"(room, column, row)", /* rgetstepsound */
	"(room, column, row, sound)", /* stepsound */
	
	"(room, vertex)", /* getvertex */
	"(room, vertex)", /* cgetvertex */
	"(room, vertex)", /* rgetvertex */
	"(room, vertex)", /* hgetvertex */
	"(room, x, z, y, intensity1, <intensity2>, <attributes>)", /* addvertex */
	"(room, vertex, <x>, <z>, <y>, <intensity1>, <intensity2>, <attributes>)", /* replacevertex */
	"(room, vertex)", /* removevertex */
	"(room)", /* removegeometry */
	
	"(room, rectangle)", /* getrectangle */
	"(room, rectangle)", /* cgetrectangle */
	"(room, rectangle)", /* rgetrectangle */
	"(room, rectangle)", /* hgetrectangle */
	"(room, vertex1, vertex2, vertex3, vertex4, texture, <doublesided>)", /* addrectangle */
	"(room, rectangle, <vertex1>, <vertex2>, <vertex3>, <vertex4>, <texture>, <doublesided>)", /* replacerectangle */
	"(room, rectangle)", /* removerectangle */
	
	"(room, triangle)", /* gettriangle */
	"(room, triangle)", /* cgettriangle */
	"(room, triangle)", /* rgettriangle */
	"(room, triangle)", /* hgettriangle */
	"(room, vertex1, vertex2, vertex3, texture, <doublesided>)", /* addtriangle */
	"(room, triangle, <vertex1>, <vertex2>, <vertex3>, <texture>, <doublesided>)", /* replacetriangle */
	"(room, triangle)", /* removetriangle */
	
	"(room, sprite)", /* getsprite */
	"(room, sprite)", /* cgetsprite */
	"(room, sprite)", /* rgetsprite */
	"(room, sprite)", /* hgetsprite */
	"(id, room, x, z, y, intensity1, <intensity2>, <attributes>)", /* addsprite */
	"(room, sprite, <id>, <x>, <z>, <y>, <intensity1>, <intensity2>, <attributes>)", /* replacesprite */
	"(room, sprite)", /* removesprite */
	
	"(room, viewport)", /* getviewport */
	"(room, viewport)", /* cgetviewport */
	"(room, viewport)", /* rgetvierport */
	"(room, adjroom, (n_x, n_z, n_y), (v1_x, v1_z, v1_y), (v2_x, v2_z, v2_y), (v3_x, v3_z, v3_y), (v4_x, v4_z, v4_y))", /* addviewport */
	"(room, viewport, <adjroom>, <(<n_x>, <n_z>, <n_y>)>, <(<v1_x>, <v1_z>, <v1_y>)>, <(<v2_x>, <v2_z>, <v2_y>)>, <(<v3_x>, <v3_z>, <v3_y>)>, <(<v4_x>, <v4_z>, <v4_y>)>)", /* replaceviewport */
	"(room, viewport)", /* removeviewport */
	
	"(room)", /* getroomlight */
	"(room)", /* cgetroomlight */
	"(room)", /* rgetroomlight */
	"(room, intensity1, <intensity2>, <lightmode>)", /* roomlight */
	
	"(room)", /* getaltroom */
	"(room)", /* cgetaltroom */
	"(room)", /* rgetaltroom */
	"(room, altroom)", /* altroom */
	
	"(room)", /* getroomflags */
	"(room)", /* cgetroomflags */
	"(room)", /* rgetroomflags */
	
	"(room)", /* getwater */
	"(room)", /* cgetwater */
	"(room)", /* rgetwater */
	"(room)", /* water */
	"(room)", /* nowater */
	
	"(room)", /* getsfxalways */
	"(room)", /* cgetsfxalways */
	"(room)", /* rgetsfxalways */
	"(room)", /* sfxalways */
	"(room)", /* nosfxalways */
	
	"(room)", /* getpitchshift */
	"(room)", /* cgetpitchshift */
	"(room)", /* rgetpitchshift */
	"(room)", /* pitchshift */
	"(room)", /* nopitchshift */
	
	"(room)", /* getsky */
	"(room)", /* cgetsky */
	"(room)", /* rgetsky */
	"(room)", /* sky */
	"(room)", /* nosky */
	
	"(room)", /* getdynamic */
	"(room)", /* cgetdynamic */
	"(room)", /* rgetdynamic */
	"(room)", /* dynamic */
	"(room)", /* nodynamic */
	
	"(room)", /* getwind */
	"(room)", /* cgetwind */
	"(room)", /* rgetwind */
	"(room)", /* wind */
	"(room)", /* nowind */
	
	"(room)", /* getinside */
	"(room)", /* cgetinside */
	"(room)", /* rgetinside */
	"(room)", /* inside */
	"(room)", /* noinside */
	
	"(room)", /* getswamp */
	"(room)", /* cgetswamp */
	"(room)", /* rgetswamp */
	"(room)", /* swamp */
	"(room)", /* noswamp */
	
	"(room)", /* getroompos */
	"(room)", /* cgetroompos */
	"(room)", /* rgetroompos */
	"(room, x, z, y)", /* moveroom */
	
	"(room)", /* getroomsize */
	"(room)", /* cgetroomsize */
	"(room)", /* rgetroomsize */
	"(room, columns, rows, height)", /* resizeroom */
	
	"(box)", /* getbox */
	"(box)", /* cgetbox */
	"(box)", /* rgetbox */
	"((room, xmin, zmin, y), (room, xmax, zmax, y), (gz1, gz2, gz3, gz4, fz), (agz1, agz2, agz3, agz4, afz), [<overlap, overlap, overlap>], <blocked/blockable/both>)", /* addbox */
	"(box, <(room, xmin, zmin, y)>, <(room, xmax, zmax, y)>, <(gz1, gz2, gz3, gz4, fz)>, <(agz1, agz2, agz3, agz4, afz)>, <[<overlap, overlap, overlap>]>, <blocked/blockable/both>)", /* replacebox */
	"\0", /* removebox */
	
	"\0", /* addroom */
	"\0", /* removerooms */
	"(<o><n><N>)", /* hexdump */
	"\0" /* vardump */
};

/* Minimum number of parameters per command */
static const uint16 cmd_min_params[] =
{
	0x0000, /* nothing */
	
	0x0001, /* getitem */
	0x0001, /* cgetitem */
	0x0001, /* rgetitem */
	0x0001, /* hgetitem */
	0x0008, /* additem */
	0x0002, /* replaceitem */
	0x0001, /* removeitem */
	
	0x0002, /* getstaticmesh */
	0x0002, /* cgetstaticmesh */
	0x0002, /* rgetstaticmesh */
	0x0002, /* hgetstaticmesh */
	0x0007, /* addstaticmesh */
	0x0003, /* replacestaticmesh */
	0x0002, /* removestaticmesh */
	
	0x0002, /* getlight */
	0x0002, /* cgetlight */
	0x0002, /* rgetlight */
	0x0002, /* hgetlight */
	0x0006, /* addlight */
	0x0003, /* replacelight */
	0x0002, /* removelight */
	
	0x0001, /* getsoundsource */
	0x0001, /* cgetsoundsource */
	0x0001, /* rgetsoundsource */
	0x0001, /* hgetsoundsource */
	0x0006, /* addsoundsource */
	0x0002, /* replacesoundsource */
	0x0001, /* removesoundsource */
	
	0x0003, /* getfloordata */
	0x0003, /* cgetfloordata */
	0x0003, /* rgetfloordata */
	0x0003, /* replacefloordata */
	0x0003, /* removefloordata */
	
	0x0003, /* getfloor */
	0x0003, /* cgetfloor */
	0x0003, /* rgetfloor */
	0x0003, /* hgetfloor */
	0x0004, /* floor */
	
	0x0003, /* getceiling */
	0x0003, /* cgetceiling */
	0x0003, /* rgetceiling */
	0x0003, /* hgetceiling */
	0x0004, /* ceiling */
	
	0x0003, /* getzone */
	0x0003, /* cgetzone */
	0x0003, /* rgetzone */
	0x0004, /* zone */
	
	0x0003, /* getstepsound */
	0x0003, /* cgetstepsound */
	0x0003, /* rgetstepsound */
	0x0004, /* stepsound */
	
	0x0002, /* getvertex */
	0x0002, /* cgetvertex */
	0x0002, /* rgetvertex */
	0x0002, /* hgetvertex */
	0x0005, /* addvertex */
	0x0003, /* replacevertex */
	0x0002, /* removevertex */
	0x0001, /* removegeometry */
	
	0x0002, /* getrectangle */
	0x0002, /* cgetrectangle */
	0x0002, /* rgetrectangle */
	0x0002, /* hgetrectangle */
	0x0006, /* addrectangle */
	0x0003, /* replacerectangle */
	0x0002, /* removerectangle */
	
	0x0002, /* gettriangle */
	0x0002, /* cgettriangle */
	0x0002, /* rgettriangle */
	0x0002, /* hgettriangle */
	0x0005, /* addtriangle */
	0x0003, /* replacetriangle */
	0x0002, /* removetriangle */
	
	0x0002, /* getsprite */
	0x0002, /* cgetsprite */
	0x0002, /* rgetsprite */
	0x0002, /* hgetsprite */
	0x0006, /* addsprite */
	0x0003, /* replacesprite */
	0x0002, /* removesprite */
	
	0x0002, /* getviewport */
	0x0002, /* cgetviewport */
	0x0002, /* rgetvierport */
	0x0007, /* addviewport */
	0x0003, /* replaceviewport */
	0x0002, /* removeviewport */
	
	0x0001, /* getroomlight */
	0x0001, /* cgetroomlight */
	0x0001, /* rgetroomlight */
	0x0002, /* roomlight */
	
	0x0001, /* getaltroom */
	0x0001, /* cgetaltroom */
	0x0001, /* rgetaltroom */
	0x0002, /* altroom */
	
	0x0001, /* getroomflags */
	0x0001, /* cgetroomflags */
	0x0001, /* rgetroomflags */
	
	0x0001, /* getwater */
	0x0001, /* cgetwater */
	0x0001, /* rgetwater */
	0x0001, /* water */
	0x0001, /* nowater */
	
	0x0001, /* getsfxalways */
	0x0001, /* cgetsfxalways */
	0x0001, /* rgetsfxalways */
	0x0001, /* sfxalways */
	0x0001, /* nosfxalways */
	
	0x0001, /* getpitchshift */
	0x0001, /* cgetpitchshift */
	0x0001, /* rgetpitchshift */
	0x0001, /* pitchshift */
	0x0001, /* nopitchshift */
	
	0x0001, /* getsky */
	0x0001, /* cgetsky */
	0x0001, /* rgetsky */
	0x0001, /* sky */
	0x0001, /* nosky */
	
	0x0001, /* getdynamic */
	0x0001, /* cgetdynamic */
	0x0001, /* rgetdynamic */
	0x0001, /* dynamic */
	0x0001, /* nodynamic */
	
	0x0001, /* getwind */
	0x0001, /* cgetwind */
	0x0001, /* rgetwind */
	0x0001, /* wind */
	0x0001, /* nowind */
	
	0x0001, /* getinside */
	0x0001, /* cgetinside */
	0x0001, /* rgetinside */
	0x0001, /* inside */
	0x0001, /* noinside */
	
	0x0001, /* getswamp */
	0x0001, /* cgetswamp */
	0x0001, /* rgetswamp */
	0x0001, /* swamp */
	0x0001, /* noswamp */
	
	0x0001, /* getroompos */
	0x0001, /* cgetroompos */
	0x0001, /* rgetroompos */
	0x0004, /* moveroom */
	
	0x0001, /* getroomsize */
	0x0001, /* cgetroomsize */
	0x0001, /* rgetroomsize */
	0x0004, /* resizeroom */
	
	0x0001, /* getbox */
	0x0001, /* cgetbox */
	0x0001, /* rgetbox */
	0x0005, /* addbox */
	0x0002, /* replacebox */
	0x0000, /* removebox */
	
	0x0000, /* addroom */
	0x0000, /* removerooms */
	0x0000, /* hexdump */
	0x0000  /* vardump */
};

/* Maximum number of parameters per command */
static const uint16 cmd_max_params[] =
{
	0x0000, /* nothing */
	
	0x0001, /* getitem */
	0x0001, /* cgetitem */
	0x0001, /* rgetitem */
	0x0001, /* hgetitem */
	0x0009, /* additem */
	0x000A, /* replaceitem */
	0x0002, /* removeitem */
	
	0x0002, /* getstaticmesh */
	0x0002, /* cgetstaticmesh */
	0x0002, /* rgetstaticmesh */
	0x0002, /* hgetstaticmesh */
	0x0008, /* addstaticmesh */
	0x0009, /* replacestaticmesh */
	0x0002, /* removestaticmesh */
	
	0x0002, /* getlight */
	0x0002, /* cgetlight */
	0x0002, /* rgetlight */
	0x0002, /* hgetlight */
	0x0009, /* addlight */
	0x000A, /* replacelight */
	0x0002, /* removelight */
	
	0x0001, /* getsoundsource */
	0x0001, /* cgetsoundsource */
	0x0001, /* rgetsoundsource */
	0x0001, /* hgetsoundsource */
	0x0006, /* addsoundsource */
	0x0007, /* replacesoundsource */
	0x0001, /* removesoundsource */
	
	0x0003, /* getfloordata */
	0x0003, /* cgetfloordata */
	0x0003, /* rgetfloordata */
	0x0003, /* replacefloordata */
	0x0003, /* removefloordata */
	
	0x0003, /* getfloor */
	0x0003, /* cgetfloor */
	0x0003, /* rgetfloor */
	0x0003, /* hgetfloor */
	0x0004, /* floor */
	
	0x0003, /* getceiling */
	0x0003, /* cgetceiling */
	0x0003, /* rgetceiling */
	0x0003, /* hgetceiling */
	0x0004, /* ceiling */
	
	0x0003, /* getzone */
	0x0003, /* cgetzone */
	0x0003, /* rgetzone */
	0x0004, /* zone */
	
	0x0003, /* getstepsound */
	0x0003, /* cgetstepsound */
	0x0003, /* rgetstepsound */
	0x0004, /* stepsound */
	
	0x0002, /* getvertex */
	0x0002, /* cgetvertex */
	0x0002, /* rgetvertex */
	0x0002, /* hgetvertex */
	0x0007, /* addvertex */
	0x0009, /* replacevertex */
	0x0002, /* removevertex */
	0x0001, /* removegeometry */
	
	0x0002, /* getrectangle */
	0x0002, /* cgetrectangle */
	0x0002, /* rgetrectangle */
	0x0002, /* hgetrectangle */
	0x0007, /* addrectangle */
	0x0008, /* replacerectangle */
	0x0002, /* removerectangle */
	
	0x0002, /* gettriangle */
	0x0002, /* cgettriangle */
	0x0002, /* rgettriangle */
	0x0002, /* hgettriangle */
	0x0006, /* addtriangle */
	0x0007, /* replacetriangle */
	0x0002, /* removetriangle */
	
	0x0002, /* getsprite */
	0x0002, /* cgetsprite */
	0x0002, /* rgetsprite */
	0x0002, /* hgetsprite */
	0x0008, /* addsprite */
	0x0009, /* replacesprite */
	0x0002, /* removesprite */
	
	0x0002, /* getviewport */
	0x0002, /* cgetviewport */
	0x0002, /* rgetvierport */
	0x0007, /* addviewport */
	0x0008, /* replaceviewport */
	0x0002, /* removeviewport */
	
	0x0001, /* getroomlight */
	0x0001, /* cgetroomlight */
	0x0001, /* rgetroomlight */
	0x0004, /* roomlight */
	
	0x0001, /* getaltroom */
	0x0001, /* cgetaltroom */
	0x0001, /* rgetaltroom */
	0x0002, /* altroom */
	
	0x0001, /* getroomflags */
	0x0001, /* cgetroomflags */
	0x0001, /* rgetroomflags */
	
	0x0001, /* getwater */
	0x0001, /* cgetwater */
	0x0001, /* rgetwater */
	0x0001, /* water */
	0x0001, /* nowater */
	
	0x0001, /* getsfxalways */
	0x0001, /* cgetsfxalways */
	0x0001, /* rgetsfxalways */
	0x0001, /* sfxalways */
	0x0001, /* nosfxalways */
	
	0x0001, /* getpitchshift */
	0x0001, /* cgetpitchshift */
	0x0001, /* rgetpitchshift */
	0x0001, /* pitchshift */
	0x0001, /* nopitchshift */
	
	0x0001, /* getsky */
	0x0001, /* cgetsky */
	0x0001, /* rgetsky */
	0x0001, /* sky */
	0x0001, /* nosky */
	
	0x0001, /* getdynamic */
	0x0001, /* cgetdynamic */
	0x0001, /* rgetdynamic */
	0x0001, /* dynamic */
	0x0001, /* nodynamic */
	
	0x0001, /* getwind */
	0x0001, /* cgetwind */
	0x0001, /* rgetwind */
	0x0001, /* wind */
	0x0001, /* nowind */
	
	0x0001, /* getinside */
	0x0001, /* cgetinside */
	0x0001, /* rgetinside */
	0x0001, /* inside */
	0x0001, /* noinside */
	
	0x0001, /* getswamp */
	0x0001, /* cgetswamp */
	0x0001, /* rgetswamp */
	0x0001, /* swamp */
	0x0001, /* noswamp */
	
	0x0001, /* getroompos */
	0x0001, /* cgetroompos */
	0x0001, /* rgetroompos */
	0x0004, /* moveroom */
	
	0x0001, /* getroomsize */
	0x0001, /* cgetroomsize */
	0x0001, /* rgetroomsize */
	0x0004, /* resizeroom */
	
	0x0001, /* getbox */
	0x0001, /* cgetbox */
	0x0001, /* rgetbox */
	0x0006, /* addbox */
	0x0007, /* replacebox */
	0x0000, /* removebox */
	
	0x0000, /* addroom */
	0x0000, /* removerooms */
	0x0003, /* hexdump */
	0x0000  /* vardump */
};

/* Internal function declarations */
static int cmd_strtoint(char *in);

/* Macro declarations */
/*
 * Macro for use in interpretCommand that checks whether a parameter exists
 * Parameters:
 * * n = The number of the parameter (counted from 0)
 * * p = The number of parameters
 * * s = The array of strings
 */
#define if_param_exists(n, p, s)       \
	param_exists = 0;                  \
	if (p > n)                         \
	{                                  \
		if (strlen(s[n]) > (size_t) 0) \
		{                              \
			param_exists = 1;          \
		}                              \
	}                                  \
	if (param_exists == 1)

/*
 * Function that identifies what command was entered
 * Parameters:
 * * Command in string form
 * Returns command code
 */
static int cmd_strtoint(char *in)
{
	/* Variable Declarations */
	int cur = 0; /* Current command */
	int cmp = 0; /* Return value of the comparison */
	
	/* Compares the commands in the list */
	for (cur = (CMD_NONE + 1); cur < NUM_COMMANDS; ++cur)
	{
		cmp = compareString(in, (char *) cmd_string[cur],
		                    strlen(cmd_string[cur]));
		if (cmp == 0)
		{
			return cur;
		}
	}
	
	return CMD_INVALID;
}

/*
 * Function that interprets the entered command and calls needed functions
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * in = The command (in string-form)
 */
void interpretCommand(struct level *level, struct error *error, char *in)
{
	/* Variable Declarations */
	int command = CMD_NONE;       /* The code for the entered command */
	uint16 numParams = 0x0000;    /* The number of parameters in the command */
	uint16 numSubParams = 0x0000; /* The number of subparameters in parameter */
	uint16 paramstart = 0x0000;   /* Where the parameters start */
	int param_exists = 0;         /* Variable used to check parameters */
	
	/* Variables used for loops */
	uint32 l_u32[1] = {0x00000000U};
	uint32 h_u32[1] = {0x00000000U};
	uint16 l_u16[3] = {0x0000U, 0x0000U, 0x0000U};
	uint16 h_u16[3] = {0x0000U, 0x0000U, 0x0000U};
	
	/* Variables for parameters (in output format) */
	char  *p_str[3] = {NULL, NULL, NULL};
	char  *p_strcpy[3] = {NULL, NULL, NULL};
	int    p_int[2] = {0, 0};
	uint32 p_u32[5] = {0x00000000U, 0x00000000U,
	                   0x00000000U, 0x00000000U, 0x00000000U};
	int32  p_s32[5] = {0x00000000, 0x00000000,
	                   0x00000000, 0x00000000, 0x00000000};
	uint16 p_u16[9] = {0x0000U, 0x0000U, 0x0000U, 0x0000U, 0x0000U,
	                   0x0000U, 0x0000U, 0x0000U, 0x0000U};
	int16  p_s16[15] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	                    0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	                    0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
	uint8 p_u8[1] = {0x00U};
	
	/* Parameters (in string format) */
	char *params[10] =                   /* Separated parameters as strings */
		{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char **paramptr = (char **) &params; /* Pointer to pass to splitString */
	char **subParams = NULL;             /* Separated subparameters */
	
	/* Filters out undesired characters */
	filterchars(in, (char *) allowed_chars);
	
	/* Determines what command was entered */
	command = cmd_strtoint(in);
	if (command == CMD_INVALID)
	{
		error->code = ERROR_INVALID_COMMAND;
		error->string[0] = in;
		goto end;
	}
	
	/* Special case for floordata syntax */
	if (command == REPLACE_FLOORDATA)
	{
		paramstart = charindex(in, (uint16) strlen(in), ':', 1);
		if (paramstart == 0xFFFF)
		{
			trmod_printf("Usage: %s%s\n", cmd_string[command],
			             cmd_usage[command]);
			error->code = ERROR_SILENT;
			goto end;
		}
		in[paramstart] = ')';
		++paramstart;
		p_strcpy[0] = calloc((strlen(&(in[paramstart])) + (size_t) 1U),
		                     (size_t) 1U);
		if (p_strcpy[0] == NULL)
		{
			error->code = ERROR_MEMORY;
			goto end;
		}
		p_str[0] = &(in[paramstart]);
		memcpy(p_strcpy[0], p_str[0], strlen(p_str[0]));
		in[paramstart] = '\0';
	}
	
	/* Counts the parameters */
	paramstart = charindex(in, (uint16) strlen(in), '(', 1);
	if (paramstart == 0xFFFF)
	{
		numParams = 0x0000;
	}
	else
	{
		/* Determines the number of parameters */
		in = &(in[(paramstart + 0x0001)]);
		paramstart = charindex(in, (uint16) strlen(in), ')', -1);
		if (paramstart != 0xFFFF)
		{
			in[paramstart] = '\0';
		}
		numParams = splitString(in, NULL, (uint16) strlen(in),
		                        ',', 1, 2, error);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
	}
	
	/* Checks minimum and maximum parameters */
	if ((numParams < cmd_min_params[command]) ||
	    (numParams > cmd_max_params[command]))
	{
		trmod_printf("Usage: %s%s\n", cmd_string[command], cmd_usage[command]);
		error->code = ERROR_SILENT;
		goto end;
	}
	
	/* Splits the parameters */
	if (numParams > 0x0000U)
	{
		numParams = splitString(in, &paramptr,
		                        (uint16) strlen(in), ',', 1, 0, error);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
	}
	
	/* Converts the parameters to the needed formats */
	switch (command)
	{
		/* (item) */
		case GET_ITEM:
		case CGET_ITEM:
		case RGET_ITEM:
		case HGET_ITEM:
		case GET_SOUNDSOURCE:
		case CGET_SOUNDSOURCE:
		case RGET_SOUNDSOURCE:
		case HGET_SOUNDSOURCE:
		case REMOVE_SOUNDSOURCE:
		case GET_BOX:
		case CGET_BOX:
		case RGET_BOX:
			/* Item */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u32[0] = ALL_U32;
			}
			else if (params[0][0] == '*')
			{
				p_u32[0] = ALL_U32;
			}
			else
			{
				p_u32[0] = (uint32) strtol(params[0], NULL, 10);
			}
			break;
		/* (room) */
		case REMOVE_GEOMETRY:
		case GET_ROOMLIGHT:
		case CGET_ROOMLIGHT:
		case RGET_ROOMLIGHT:
		case GET_ALTROOM:
		case CGET_ALTROOM:
		case RGET_ALTROOM:
		case GET_ROOMFLAGS:
		case CGET_ROOMFLAGS:
		case RGET_ROOMFLAGS:
		case GET_WATER:
		case CGET_WATER:
		case RGET_WATER:
		case SET_WATER_ON:
		case SET_WATER_OFF:
		case GET_SFXALWAYS:
		case CGET_SFXALWAYS:
		case RGET_SFXALWAYS:
		case SET_SFXALWAYS_ON:
		case SET_SFXALWAYS_OFF:
		case GET_PITCHSHIFT:
		case CGET_PITCHSHIFT:
		case RGET_PITCHSHIFT:
		case SET_PITCHSHIFT_ON:
		case SET_PITCHSHIFT_OFF:
		case GET_WIND:
		case CGET_WIND:
		case RGET_WIND:
		case SET_WIND_ON:
		case SET_WIND_OFF:
		case GET_INSIDE:
		case CGET_INSIDE:
		case RGET_INSIDE:
		case SET_INSIDE_ON:
		case SET_INSIDE_OFF:
		case GET_SKY:
		case CGET_SKY:
		case RGET_SKY:
		case SET_SKY_ON:
		case SET_SKY_OFF:
		case GET_DYNAMIC:
		case CGET_DYNAMIC:
		case RGET_DYNAMIC:
		case SET_DYNAMIC_ON:
		case SET_DYNAMIC_OFF:
		case GET_SWAMP:
		case CGET_SWAMP:
		case RGET_SWAMP:
		case SET_SWAMP_ON:
		case SET_SWAMP_OFF:
		case GET_ROOMPOS:
		case CGET_ROOMPOS:
		case RGET_ROOMPOS:
		case GET_ROOMSIZE:
		case CGET_ROOMSIZE:
		case RGET_ROOMSIZE:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			break;
		/* (room, item) */
		case GET_STATICMESH:
		case CGET_STATICMESH:
		case RGET_STATICMESH:
		case HGET_STATICMESH:
		case REMOVE_STATICMESH:
		case GET_LIGHT:
		case CGET_LIGHT:
		case RGET_LIGHT:
		case HGET_LIGHT:
		case REMOVE_LIGHT:
		case GET_VERTEX:
		case CGET_VERTEX:
		case RGET_VERTEX:
		case HGET_VERTEX:
		case REMOVE_VERTEX:
		case GET_RECTANGLE:
		case CGET_RECTANGLE:
		case RGET_RECTANGLE:
		case HGET_RECTANGLE:
		case REMOVE_RECTANGLE:
		case GET_TRIANGLE:
		case CGET_TRIANGLE:
		case RGET_TRIANGLE:
		case HGET_TRIANGLE:
		case REMOVE_TRIANGLE:
		case GET_SPRITE:
		case CGET_SPRITE:
		case RGET_SPRITE:
		case HGET_SPRITE:
		case REMOVE_SPRITE:
		case GET_VIEWPORT:
		case CGET_VIEWPORT:
		case RGET_VIEWPORT:
		case REMOVE_VIEWPORT:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Item */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			break;
		/* (room, column, row) */
		case GET_FLOORDATA:
		case CGET_FLOORDATA:
		case RGET_FLOORDATA:
		case REMOVE_FLOORDATA:
		case GET_FLOOR:
		case CGET_FLOOR:
		case RGET_FLOOR:
		case HGET_FLOOR:
		case GET_CEILING:
		case CGET_CEILING:
		case RGET_CEILING:
		case HGET_CEILING:
		case GET_ZONE:
		case CGET_ZONE:
		case RGET_ZONE:
		case GET_STEPSOUND:
		case CGET_STEPSOUND:
		case RGET_STEPSOUND:
		case REPLACE_FLOORDATA:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Column */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			/* Row */
			if (strlen(params[2]) == (size_t) 0)
			{
				p_u16[2] = ALL_U16;
			}
			else if (params[2][0] == '*')
			{
				p_u16[2] = ALL_U16;
			}
			else
			{
				p_u16[2] = (uint16) strtol(params[2], NULL, 10);
			}
			break;
		/* (room, column, row, height) */
		case SET_FLOOR:
		case SET_CEILING:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Column */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			/* Row */
			if (strlen(params[2]) == (size_t) 0)
			{
				p_u16[2] = ALL_U16;
			}
			else if (params[2][0] == '*')
			{
				p_u16[2] = ALL_U16;
			}
			else
			{
				p_u16[2] = (uint16) strtol(params[2], NULL, 10);
			}
			/* Height */
			if (command == SET_FLOOR)
			{
				p_u16[3] = 0x0000U;
			}
			else
			{
				p_u16[3] = 0x0001U;
			}
			if (strlen(params[3]) == (size_t) 0)
			{
				p_u16[3] |= 0x0002U;
			}
			else if ((params[3][0] == 'W') || (params[3][0] == 'w'))
			{
				p_u16[3] |= 0x0002U;
			}
			else
			{
				p_s32[0] = (int32) strtol(params[3], NULL, 10);
			}
			break;
		/* (room, column, row, item) */
		case SET_ZONE:
		case SET_STEPSOUND:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Column */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			/* Row */
			if (strlen(params[2]) == (size_t) 0)
			{
				p_u16[2] = ALL_U16;
			}
			else if (params[2][0] == '*')
			{
				p_u16[2] = ALL_U16;
			}
			else
			{
				p_u16[2] = (uint16) strtol(params[2], NULL, 10);
			}
			/* Value */
			p_u16[3] = (uint16) strtol(params[3], NULL, 10);
			p_s16[0] = (int16) strtol(params[3], NULL, 10);
			break;
		case ADD_ITEM:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* ID */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
			p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
			p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
			p_s16[0] = (int16) strtol(params[5], NULL, 10); /* Angle */
			p_str[0] = params[6]; /* Intensity1 */
			if (numParams == 0x0008)
			{
				p_u16[2] = (uint16) strtol(params[7], NULL, 16); /* Flags */
			}
			else
			{
				p_str[1] = params[7]; /* Intensity2 */
				p_u16[2] = (uint16) strtol(params[8], NULL, 16); /* Flags */
			}
			break;
		case REPLACE_ITEM:
			/* Item */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u32[0] = ALL_U32;
			}
			else if (params[0][0] == '*')
			{
				p_u32[0] = ALL_U32;
			}
			else
			{
				p_u32[0] = (uint32) strtol(params[0], NULL, 10);
			}
			if_param_exists(0x0001U, numParams, params)
			{
				p_u16[0] = (uint16) strtol(params[1], NULL, 10); /* ID */
				p_u16[3] |= 0x0001U;
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[2], NULL, 10); /* Room */
				p_u16[3] |= 0x0002U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_s32[0] = (int32) strtol(params[3], NULL, 10); /* X */
				p_u16[3] |= 0x0004U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_s32[1] = (int32) strtol(params[5], NULL, 10); /* Y */
				p_u16[3] |= 0x0008U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_s32[2] = (int32) strtol(params[4], NULL, 10); /* Z */
				p_u16[3] |= 0x0010U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_s16[0] = (int16) strtol(params[6], NULL, 10); /* Angle */
				p_u16[3] |= 0x0020U;
			}
			if_param_exists(0x0007U, numParams, params)
			{
				p_str[0] = params[7]; /* Intensity1 */
				p_u16[3] |= 0x0040U;
			}
			if_param_exists(0x0009U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[9], NULL, 16); /* Flags */
				p_u16[3] |= 0x0080U;
				if_param_exists(0x0008U, numParams, params)
				{
					p_str[1] = params[8]; /* Intensity2 */
					p_u16[3] |= 0x0100U;
				}
			}
			else
			{
				if_param_exists(0x0008U, numParams, params)
				{
					p_u16[2] = (uint16) strtol(params[8], NULL, 16); /* Flags */
					p_u16[3] |= 0x0080U;
				}
			}
			break;
		case REMOVE_ITEM:
			/* Item */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u32[0] = ALL_U32;
			}
			else if (params[0][0] == '*')
			{
				p_u32[0] = ALL_U32;
			}
			else
			{
				p_u32[0] = (uint32) strtol(params[0], NULL, 10);
			}
			/* Raw-flag */
			p_u16[0] = 0x0001U;
			if_param_exists(0x0001U, numParams, params)
			{
				if ((params[1][0] == 'R') || (params[1][0] == 'r'))
				{
					p_u16[0] &= 0xFFFEU;
				}
			}
			break;
		case ADD_STATICMESH:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* ID */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
			p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
			p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
			p_s16[0] = (int16) strtol(params[5], NULL, 10); /* Angle */
			p_str[0] = params[6]; /* Intensity1 */
			if_param_exists(0x0007U, numParams, params)
			{
				p_str[1] = params[7]; /* Intensity2 */
			}
			break;
		case REPLACE_STATICMESH:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* StaticMesh */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[2], NULL, 10); /* ID */
				p_u16[3] |= 0x0001U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_s32[0] = (int32) strtol(params[3], NULL, 10); /* X */
				p_u16[3] |= 0x0002U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_s32[1] = (int32) strtol(params[5], NULL, 10); /* Y */
				p_u16[3] |= 0x0004U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_s32[2] = (int32) strtol(params[4], NULL, 10); /* Z */
				p_u16[3] |= 0x0008U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_s16[0] = (int16) strtol(params[6], NULL, 10); /* Angle */
				p_u16[3] |= 0x0010U;
			}
			if_param_exists(0x0007U, numParams, params)
			{
				p_str[0] = params[7]; /* Intensity1 */
				p_u16[3] |= 0x0020U;
			}
			if_param_exists(0x0008U, numParams, params)
			{
				p_str[1] = params[8]; /* Intensity2 */
				p_u16[3] |= 0x0040U;
			}
			break;
		case ADD_LIGHT:
			switch (level->type)
			{
				case TRMOD_LEVTYPE_TR1_PC:
				case TRMOD_LEVTYPE_TR1G_PC:
				case TRMOD_LEVTYPE_TR1_PS:
				case TRMOD_LEVTYPE_TR2_PC:
				case TRMOD_LEVTYPE_TR2_PS:
					p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
					p_s32[0] = (int32) strtol(params[1], NULL, 10); /* X */
					p_s32[1] = (int32) strtol(params[3], NULL, 10); /* Y */
					p_s32[2] = (int32) strtol(params[2], NULL, 10); /* Z */
					p_str[0] = params[4]; /* Intensity1 */
					if_param_exists(0x0006U, numParams, params)
					{
						p_str[1] = params[5]; /* Intensity2 */
						p_u32[0] = (uint32) strtol(params[6],
						                           NULL, 10); /* Fade1 */
						if_param_exists(0x0007U, numParams, params)
						{
							p_u32[1] = (uint32) strtol(params[7],
							                           NULL, 10); /* Fade2 */
						}
					}
					else
					{
						if_param_exists(0x0005U, numParams, params)
						{
							p_u32[0] = (uint32) strtol(params[5],
							                           NULL, 10); /* Fade1 */
						}
					}
					break;
				case TRMOD_LEVTYPE_TR3_PC:
				case TRMOD_LEVTYPE_TR3_PS:
					p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
					p_s32[0] = (int32) strtol(params[1], NULL, 10); /* X */
					p_s32[1] = (int32) strtol(params[3], NULL, 10); /* Y */
					p_s32[2] = (int32) strtol(params[2], NULL, 10); /* Z */
					p_u32[1] = (uint32) strtol(params[4],
					                           NULL, 16); /* Colour */
					p_u8[0] = (uint8) 0x00U;
					if (compareString(params[5], "sun", 3) == 0) /* Type */
					{
						p_u8[0] = (uint8) 0x01U;
					}
					/* Checks number of parameters */
					if ((p_u8[0] == (uint8) 0x00U) && (numParams < 0x0008U))
					{
						trmod_printf("Usage: %s(room,x,z,y,colour,spotlight,"
						             "intensity,fade)\n", cmd_string[command]);
						error->code = ERROR_SILENT;
						goto end;
					}
					else if ((p_u8[0] == (uint8) 0x01U) &&
					         (numParams < 0x0009U))
					{
						trmod_printf("Usage: %s(room,x,z,y,colour,sunlight,"
						             "nX,nZ,nY)\n", cmd_string[command]);
						error->code = ERROR_SILENT;
						goto end;
					}
					if (p_u8[0] == (uint8) 0x00)
					{
						/* Spotlight */
						p_str[0] = params[6];
						p_u32[0] = (uint32) strtol(params[7], NULL, 10);
					}
					else
					{
						/* Sunlight */
						p_s16[0] = (int16) strtol(params[6], NULL, 10);
						p_s16[1] = (int16) strtol(params[8], NULL, 10);
						p_s16[2] = (int16) strtol(params[7], NULL, 10);
					}
					break;
			}
			break;
		case REPLACE_LIGHT:
			switch (level->type)
			{
				case TRMOD_LEVTYPE_TR1_PC:
				case TRMOD_LEVTYPE_TR1G_PC:
				case TRMOD_LEVTYPE_TR1_PS:
				case TRMOD_LEVTYPE_TR2_PC:
				case TRMOD_LEVTYPE_TR2_PS:
					/* Room */
					if (strlen(params[0]) == (size_t) 0)
					{
						p_u16[0] = ALL_U16;
					}
					else if (params[0][0] == '*')
					{
						p_u16[0] = ALL_U16;
					}
					else
					{
						p_u16[0] = (uint16) strtol(params[0], NULL, 10);
					}
					/* Light */
					if (strlen(params[1]) == (size_t) 0)
					{
						p_u16[1] = ALL_U16;
					}
					else if (params[1][0] == '*')
					{
						p_u16[1] = ALL_U16;
					}
					else
					{
						p_u16[1] = (uint16) strtol(params[1], NULL, 10);
					}
					if_param_exists(0x0002U, numParams, params)
					{
						p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
						p_u16[2] |= 0x0001U;
					}
					if_param_exists(0x0004U, numParams, params)
					{
						p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
						p_u16[2] |= 0x0002U;
					}
					if_param_exists(0x0003U, numParams, params)
					{
						p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
						p_u16[2] |= 0x0004U;
					}
					if_param_exists(0x0005U, numParams, params)
					{
						p_str[0] = params[5]; /* Intensity1 */
						p_u16[2] |= 0x0008U;
					}
					if_param_exists(0x0007U, numParams, params)
					{
						p_u32[0] = (uint32) strtol(params[7],
						                           NULL, 10); /* Fade1 */
						p_u16[2] |= 0x0020;
						if_param_exists(0x0006U, numParams, params)
						{
							p_str[1] = params[6]; /* Intensity2 */
							p_u16[2] |= 0x0010U;
						}
						if_param_exists(0x0008U, numParams, params)
						{
							p_u32[1] = (uint32) strtol(params[8],
							                           NULL, 10); /* Fade2 */
							p_u16[2] |= 0x0040U;
						}
					}
					else
					{
						if_param_exists(0x0006U, numParams, params)
						{
							p_u32[0] = (uint32) strtol(params[6],
							                           NULL, 10); /* Fade1 */
							p_u16[2] |= 0x0020U;
						}
					}
					break;
				case TRMOD_LEVTYPE_TR3_PC:
				case TRMOD_LEVTYPE_TR3_PS:
					/* Room */
					if (strlen(params[0]) == (size_t) 0)
					{
						p_u16[0] = ALL_U16;
					}
					else if (params[0][0] == '*')
					{
						p_u16[0] = ALL_U16;
					}
					else
					{
						p_u16[0] = (uint16) strtol(params[0], NULL, 10);
					}
					/* Light */
					if (strlen(params[1]) == (size_t) 0)
					{
						p_u16[1] = ALL_U16;
					}
					else if (params[1][0] == '*')
					{
						p_u16[1] = ALL_U16;
					}
					else
					{
						p_u16[1] = (uint16) strtol(params[1], NULL, 10);
					}
					p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
					p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
					p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
					p_u32[1] = (uint32) strtol(params[5],
					                           NULL, 16); /* Colour */
					p_u8[0] = (uint8) 0x00U;
					if (compareString(params[6], "sun", 3) == 0) /* Type */
					{
						p_u8[0] = (uint8) 0x01U;
					}
					/* Checks number of parameters */
					if ((p_u8[0] == (uint8) 0x00U) && (numParams < 0x0009U))
					{
						trmod_printf("Usage: %s(room,light,x,z,y,"
						             "colour,spotlight,intensity,fade)\n",
						             cmd_string[command]);
						error->code = ERROR_SILENT;
						goto end;
					}
					else if ((p_u8[0] == (uint8) 0x01U) &&
					         (numParams < 0x000AU))
					{
						trmod_printf("Usage: %s(room,light,x,z,y,"
						             "colour,sunlight,nX,nZ,nY)\n",
						             cmd_string[command]);
						error->code = ERROR_SILENT;
						goto end;
					}
					if (p_u8[0] == (uint8) 0x00)
					{
						/* Spotlight */
						p_str[0] = params[7];
						p_u32[0] = (uint32) strtol(params[8], NULL, 10);
					}
					else
					{
						/* Sunlight */
						p_s16[0] = (int16) strtol(params[7], NULL, 10);
						p_s16[1] = (int16) strtol(params[9], NULL, 10);
						p_s16[2] = (int16) strtol(params[8], NULL, 10);
					}
					p_u16[2] = 0xFFFFU;
					break;
			}
			break;
		case ADD_SOUNDSOURCE:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* ID */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
			p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
			p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
			/* Flags */
			if_param_exists(0x0005U, numParams, params)
			{
				if ((params[5][0] == 'a') || (params[5][0] == 'A'))
				{
					p_u16[2] = 0x0040U;
				}
				else if ((params[5][0] == 'b') || (params[5][0] == 'B'))
				{
					p_u16[2] = 0x00C0U;
				}
				else if ((params[5][0] == 'r') || (params[5][0] == 'R'))
				{
					p_u16[2] = 0x0080U;
				}
				else
				{
					p_u16[2] = (uint16) strtol(params[5], NULL, 16);
				}
			}
			else
			{
				p_u16[2] = 0x0080U;
			}
			break;
		case REPLACE_SOUNDSOURCE:
			/* Sound source */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u32[3] = ALL_U32;
			}
			else if (params[0][0] == '*')
			{
				p_u32[3] = ALL_U32;
			}
			else
			{
				p_u32[3] = (uint32) strtol(params[0], NULL, 10);
			}
			if_param_exists(0x0001U, numParams, params)
			{
				p_u16[0] = (uint16) strtol(params[1], NULL, 10); /* ID */
				p_u16[2] |= 0x0001U;
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[2], NULL, 10); /* Room */
				p_u16[2] |= 0x0002U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_s32[0] = (int32) strtol(params[3], NULL, 10); /* X */
				p_u16[2] |= 0x0004U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_s32[1] = (int32) strtol(params[5], NULL, 10); /* Y */
				p_u16[2] |= 0x0008U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_s32[2] = (int32) strtol(params[4], NULL, 10); /* Z */
				p_u16[2] |= 0x0010U;
			}
			/* Flags */
			if_param_exists(0x0006U, numParams, params)
			{
				if ((params[6][0] == 'a') || (params[6][0] == 'A'))
				{
					p_u16[3] = 0x0040U;
				}
				else if ((params[6][0] == 'b') || (params[6][0] == 'B'))
				{
					p_u16[3] = 0x00C0U;
				}
				else if ((params[6][0] == 'r') || (params[6][0] == 'R'))
				{
					p_u16[3] = 0x0080U;
				}
				else
				{
					p_u16[3] = (uint16) strtol(params[6], NULL, 16);
				}
				p_u16[2] |= 0x0020U;
			}
			break;
		case ADD_VERTEX:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[1], NULL, 10); /* X */
			p_s32[1] = (int32) strtol(params[3], NULL, 10); /* Y */
			p_s32[2] = (int32) strtol(params[2], NULL, 10); /* Z */
			p_str[0] = params[4]; /* Intensity1 */
			if_param_exists(0x0005U, numParams, params)
			{
				p_str[1] = params[5]; /* Intensity2 */
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[6], NULL, 16); /* Attrib */
			}
			break;
		case REPLACE_VERTEX:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Vertex */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
				p_u16[3] |= 0x0001U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
				p_u16[3] |= 0x0002U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
				p_u16[3] |= 0x0004U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_str[0] = params[5]; /* Intensity1 */
				p_u16[3] |= 0x0008U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_str[1] = params[6]; /* Intensity2 */
				p_u16[3] |= 0x0010U;
			}
			if_param_exists(0x0007U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[7], NULL, 16); /* Attrib */
				p_u16[3] |= 0x0020U;
			}
			break;
		case ADD_RECTANGLE:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Vertex 1 */
			p_u16[2] = (uint16) strtol(params[2], NULL, 10); /* Vertex 2 */
			p_u16[3] = (uint16) strtol(params[3], NULL, 10); /* Vertex 3 */
			p_u16[4] = (uint16) strtol(params[4], NULL, 10); /* Vertex 4 */
			p_u16[5] = (uint16) strtol(params[5], NULL, 10); /* Texture */
			if_param_exists(0x0006U, numParams, params)
			{
				if ((params[6][0] == 'D') || (params[6][0] == 'd'))
				{
					p_u16[6] = 0x0001U; /* Double-sided */
				}
			}
			break;
		case REPLACE_RECTANGLE:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Rectangle */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[7] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[7] = ALL_U16;
			}
			else
			{
				p_u16[7] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[2], NULL, 10); /* Vertex 1 */
				p_u16[8] |= 0x0001U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[3], NULL, 10); /* Vertex 2 */
				p_u16[8] |= 0x0002U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_u16[3] = (uint16) strtol(params[4], NULL, 10); /* Vertex 3 */
				p_u16[8] |= 0x0004U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_u16[4] = (uint16) strtol(params[5], NULL, 10); /* Vertex 4 */
				p_u16[8] |= 0x0008U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_u16[5] = (uint16) strtol(params[6], NULL, 10); /* Texture */
				p_u16[8] |= 0x0010U;
			}
			if_param_exists(0x0007U, numParams, params)
			{
				if ((params[7][0] == 'D') || (params[7][0] == 'd'))
				{
					p_u16[6] = 0x0001U; /* Double-sided */
				}
				p_u16[8] |= 0x0020U;
			}
			break;
		case ADD_TRIANGLE:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Vertex 1 */
			p_u16[2] = (uint16) strtol(params[2], NULL, 10); /* Vertex 2 */
			p_u16[3] = (uint16) strtol(params[3], NULL, 10); /* Vertex 3 */
			p_u16[4] = (uint16) strtol(params[4], NULL, 10); /* Texture */
			if_param_exists(0x0005U, numParams, params)
			{
				if ((params[5][0] == 'D') || (params[5][0] == 'd'))
				{
					p_u16[5] = 0x0001U; /* Double-sided */
				}
			}
			break;
		case REPLACE_TRIANGLE:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Triangle */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[7] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[7] = ALL_U16;
			}
			else
			{
				p_u16[7] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[2], NULL, 10); /* Vertex 1 */
				p_u16[8] |= 0x0001U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[3], NULL, 10); /* Vertex 2 */
				p_u16[8] |= 0x0002U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_u16[3] = (uint16) strtol(params[4], NULL, 10); /* Vertex 3 */
				p_u16[8] |= 0x0004U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_u16[5] = (uint16) strtol(params[5], NULL, 10); /* Texture */
				p_u16[8] |= 0x0008U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				if ((params[6][0] == 'D') || (params[6][0] == 'd'))
				{
					p_u16[6] = 0x0001U; /* Double-sided */
				}
				p_u16[8] |= 0x0010U;
			}
			break;
		case ADD_SPRITE:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* ID */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[2], NULL, 10); /* X */
			p_s32[1] = (int32) strtol(params[4], NULL, 10); /* Y */
			p_s32[2] = (int32) strtol(params[3], NULL, 10); /* Z */
			p_str[0] = params[5]; /* Intensity1 */
			if_param_exists(0x0006U, numParams, params)
			{
				p_str[1] = params[6]; /* Intensity2 */
			}
			if_param_exists(0x0007U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[7], NULL, 16); /* Attrib */
			}
			break;
		case REPLACE_SPRITE:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Sprite */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[3] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[3] = ALL_U16;
			}
			else
			{
				p_u16[3] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[0] = (uint16) strtol(params[2], NULL, 10); /* ID */
				p_u16[4] |= 0x0001U;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_s32[0] = (int32) strtol(params[3], NULL, 10); /* X */
				p_u16[4] |= 0x0002U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				p_s32[1] = (int32) strtol(params[5], NULL, 10); /* Y */
				p_u16[4] |= 0x0004U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				p_s32[2] = (int32) strtol(params[4], NULL, 10); /* Z */
				p_u16[4] |= 0x0008U;
			}
			if_param_exists(0x0006U, numParams, params)
			{
				p_str[0] = params[6]; /* Intensity1 */
				p_u16[4] |= 0x0010U;
			}
			if_param_exists(0x0007U, numParams, params)
			{
				p_str[1] = params[7]; /* Intensity2 */
				p_u16[4] |= 0x0020U;
			}
			if_param_exists(0x0008U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[8], NULL, 16); /* Attrib */
				p_u16[4] |= 0x0040U;
			}
			break;
		case ADD_VIEWPORT:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Adj room */
			l_u16[1] = 0x0000U;
			for (l_u16[0] = 0x0002U; l_u16[0] < 0x0007U; l_u16[0]++)
			{
				/* Splits the subparameters */
				filterchars(params[l_u16[0]], "0123456789,-");
				numSubParams = splitString(params[l_u16[0]], &subParams,
				                           (uint16) strlen(params[l_u16[0]]),
				                           ',', 1, 1, error);
				if (numSubParams != 0x0003U)
				{
					trmod_printf("Usage: %s%s\n", cmd_string[command],
					             cmd_usage[command]);
					error->code = ERROR_SILENT;
					goto end;
				}
				/* Gets the coordinates out */
				p_s16[l_u16[1]] = (int16) strtol(subParams[0], NULL, 10);
				l_u16[1]++;
				p_s16[l_u16[1]] = (int16) strtol(subParams[1], NULL, 10);
				l_u16[1]++;
				p_s16[l_u16[1]] = (int16) strtol(subParams[2], NULL, 10);
				l_u16[1]++;
			}
			break;
		case REPLACE_VIEWPORT:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			/* Viewport */
			if (strlen(params[1]) == (size_t) 0)
			{
				p_u16[1] = ALL_U16;
			}
			else if (params[1][0] == '*')
			{
				p_u16[1] = ALL_U16;
			}
			else
			{
				p_u16[1] = (uint16) strtol(params[1], NULL, 10);
			}
			if_param_exists(0x0002U, numParams, params)
			{
				p_u16[2] = (uint16) strtol(params[2], NULL, 10); /* Adj Room */
				p_u16[3] |= 0x0001U;
			}
			l_u16[1] = 0x0000U;
			for (l_u16[0] = 0x0003U; l_u16[0] < 0x0008U; l_u16[0]++)
			{
				if_param_exists(l_u16[0], numParams, params)
				{
					/* Splits the subparameters */
					filterchars(params[l_u16[0]], "0123456789,-");
					numSubParams = splitString(params[l_u16[0]], &subParams,
					                           (uint16)strlen(params[l_u16[0]]),
					                           ',', 1, 1, error);
					/* Gets the coordinates out */
					if_param_exists(0x0000U, numSubParams, subParams)
					{
						p_s16[l_u16[1]] = (int16) strtol(subParams[0],
						                                 NULL, 10); /* X */
						p_u16[3] |= (uint16) (0x0001U << (l_u16[1] + 0x0001U));
					}
					l_u16[1]++;
					if_param_exists(0x0002U, numSubParams, subParams)
					{
						p_s16[l_u16[1]] = (int16) strtol(subParams[2],
						                                 NULL, 10); /* Y */
						p_u16[3] |= (uint16) (0x0001U << (l_u16[1] + 0x0001U));
					}
					l_u16[1]++;
					if_param_exists(0x0001U, numSubParams, subParams)
					{
						p_s16[l_u16[1]] = (int16) strtol(subParams[1],
						                                 NULL, 10); /* Z */
						p_u16[3] |= (uint16) (0x0001U << (l_u16[1] + 0x0001U));
					}
					l_u16[1]++;
				}
			}
			break;
		case SET_ROOMLIGHT:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			p_str[0] = params[1]; /* Intensity1 */
			if_param_exists(0x0002U, numParams, params)
			{
				p_str[1] = params[2]; /* Intensity2 */
			}
			if_param_exists(0x0003U, numParams, params)
			{
				p_u16[1] = (uint16) strtol(params[3], NULL, 16); /* Lightmode */
			}
			break;
		case SET_ALTROOM:
			/* Room */
			if (strlen(params[0]) == (size_t) 0)
			{
				p_u16[0] = ALL_U16;
			}
			else if (params[0][0] == '*')
			{
				p_u16[0] = ALL_U16;
			}
			else
			{
				p_u16[0] = (uint16) strtol(params[0], NULL, 10);
			}
			p_s16[0] = (int16) strtol(params[1], NULL, 10); /* Alternate room */
			break;
		case MOVE_ROOM:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(params[1], NULL, 10); /* X */
			p_s32[2] = (int32) strtol(params[2], NULL, 10); /* Z */
			p_s32[1] = (int32) strtol(params[3], NULL, 10); /* Y */
			break;
		case RESIZE_ROOM:
			p_u16[0] = (uint16) strtol(params[0], NULL, 10); /* Room */
			p_u16[1] = (uint16) strtol(params[1], NULL, 10); /* Columns */
			p_u16[2] = (uint16) strtol(params[2], NULL, 10); /* Rows */
			p_s32[0] = (int32) strtol(params[3], NULL, 10); /* Height */
			break;
		case ADD_BOX:
			filterchars(params[0], "0123456789,-");
			numSubParams = splitString(params[0], &subParams,
			                           (uint16) strlen(params[0]),
			                           ',', 1, 1, error);
			if (numSubParams != 0x0004U)
			{
				trmod_printf("Usage: %s%s\n",
				             cmd_string[command], cmd_usage[command]);
				error->code = ERROR_SILENT;
				goto end;
			}
			p_u16[0] = (uint16) strtol(subParams[0], NULL, 10); /* Room */
			p_s32[0] = (int32) strtol(subParams[1], NULL, 10); /* Xmin */
			p_s32[1] = (int32) strtol(subParams[2], NULL, 10); /* Zmin */
			p_s16[0] = (int16) strtol(subParams[3], NULL, 10); /* Y */
			filterchars(params[1], "0123456789,-");
			numSubParams = splitString(params[1], &subParams,
			                           (uint16) strlen(params[1]),
			                           ',', 1, 1, error);
			if (numSubParams < 0x0003U)
			{
				trmod_printf("Usage: %s%s\n",
				             cmd_string[command], cmd_usage[command]);
				error->code = ERROR_SILENT;
				goto end;
			}
			p_s32[2] = (int32) strtol(subParams[1], NULL, 10); /* Xmax */
			p_s32[3] = (int32) strtol(subParams[2], NULL, 10); /* Zmax */
			filterchars(params[2], "0123456789,-");
			numSubParams = splitString(params[2], &subParams,
			                           (uint16) strlen(params[2]),
			                           ',', 1, 1, error);
			if (numSubParams != 0x0005U)
			{
				trmod_printf("Usage: %s%s\n",
				             cmd_string[command], cmd_usage[command]);
				error->code = ERROR_SILENT;
				goto end;
			}
			p_s16[1] = (int16) strtol(subParams[0], NULL, 10); /* gz1 */
			p_s16[2] = (int16) strtol(subParams[1], NULL, 10); /* gz2 */
			p_s16[3] = (int16) strtol(subParams[2], NULL, 10); /* gz3 */
			p_s16[4] = (int16) strtol(subParams[3], NULL, 10); /* gz4 */
			p_s16[5] = (int16) strtol(subParams[4], NULL, 10); /* fz */
			filterchars(params[3], "0123456789,-");
			numSubParams = splitString(params[3], &subParams,
			                           (uint16) strlen(params[3]),
			                           ',', 1, 1, error);
			if (numSubParams != 0x0005U)
			{
				trmod_printf("Usage: %s%s\n",
				             cmd_string[command], cmd_usage[command]);
				error->code = ERROR_SILENT;
				goto end;
			}
			p_s16[6] = (int16) strtol(subParams[0], NULL, 10); /* agz1 */
			p_s16[7] = (int16) strtol(subParams[1], NULL, 10); /* agz2 */
			p_s16[8] = (int16) strtol(subParams[2], NULL, 10); /* agz3 */
			p_s16[9] = (int16) strtol(subParams[3], NULL, 10); /* agz4 */
			p_s16[10] = (int16) strtol(subParams[4], NULL, 10); /* afz */
			filterchars(params[4], "0123456789,-");
			p_str[0] = params[4]; /* Overlaps */
			if_param_exists(0x0005U, numParams, params) /* Blocking flags */
			{
				if (compareString(params[5], "blocke", 6) == 0)
				{
					p_int[0] = 1;
				}
				else if (compareString(params[5], "blocka", 6) == 0)
				{
					p_int[1] = 1;
				}
				else if (compareString(params[5], "bot", 3) == 0)
				{
					p_int[0] = 1;
					p_int[1] = 1;
				}
			}
			break;
		case REPLACE_BOX:
			p_u32[0] = (uint32) strtol(params[0], NULL, 10); /* Box */
			if_param_exists(0x0001U, numParams, params)
			{
				filterchars(params[1], "0123456789,-");
				numSubParams = splitString(params[1], &subParams,
				                           (uint16) strlen(params[1]),
				                           ',', 1, 1, error);
				if (numSubParams != 0x0004U)
				{
					trmod_printf("Usage: %s%s\n",
					             cmd_string[command], cmd_usage[command]);
					error->code = ERROR_SILENT;
					goto end;
				}
				p_u16[0] = (uint16) strtol(subParams[0], NULL, 10); /* Room */
				p_s32[0] = (int32) strtol(subParams[1], NULL, 10); /* Xmin */
				p_s32[1] = (int32) strtol(subParams[2], NULL, 10); /* Zmin */
				p_s16[0] = (int16) strtol(subParams[3], NULL, 10); /* Y */
				p_u32[1] |= 0x00000015U;
			}
			if_param_exists(0x0002U, numParams, params)
			{
				filterchars(params[2], "0123456789,-");
				numSubParams = splitString(params[2], &subParams,
				                           (uint16) strlen(params[2]),
				                           ',', 1, 1, error);
				if (numSubParams < 0x0003U)
				{
					trmod_printf("Usage: %s%s\n",
					             cmd_string[command], cmd_usage[command]);
					error->code = ERROR_SILENT;
					goto end;
				}
				p_s32[2] = (int32) strtol(subParams[1], NULL, 10); /* Xmax */
				p_s32[3] = (int32) strtol(subParams[2], NULL, 10); /* Zmax */
				p_u32[1] |= 0x0000000AU;
			}
			if_param_exists(0x0003U, numParams, params)
			{
				filterchars(params[3], "0123456789,-");
				numSubParams = splitString(params[3], &subParams,
				                           (uint16) strlen(params[3]),
				                           ',', 1, 1, error);
				if (numSubParams != 0x0005U)
				{
					trmod_printf("Usage: %s%s\n",
					             cmd_string[command], cmd_usage[command]);
					error->code = ERROR_SILENT;
					goto end;
				}
				p_s16[1] = (int16) strtol(subParams[0], NULL, 10); /* gz1 */
				p_s16[2] = (int16) strtol(subParams[1], NULL, 10); /* gz2 */
				p_s16[3] = (int16) strtol(subParams[2], NULL, 10); /* gz3 */
				p_s16[4] = (int16) strtol(subParams[3], NULL, 10); /* gz4 */
				p_s16[5] = (int16) strtol(subParams[4], NULL, 10); /* fz */
				p_u32[1] |= 0x000003E0U;
			}
			if_param_exists(0x0004U, numParams, params)
			{
				filterchars(params[4], "0123456789,-");
				numSubParams = splitString(params[4], &subParams,
				                           (uint16) strlen(params[4]),
				                           ',', 1, 1, error);
				if (numSubParams != 0x0005U)
				{
					trmod_printf("Usage: %s%s\n",
					             cmd_string[command], cmd_usage[command]);
					error->code = ERROR_SILENT;
					goto end;
				}
				p_s16[6] = (int16) strtol(subParams[0], NULL, 10); /* agz1 */
				p_s16[7] = (int16) strtol(subParams[1], NULL, 10); /* agz2 */
				p_s16[8] = (int16) strtol(subParams[2], NULL, 10); /* agz3 */
				p_s16[9] = (int16) strtol(subParams[3], NULL, 10); /* agz4 */
				p_s16[10] = (int16) strtol(subParams[4], NULL, 10); /* afz */
				p_u32[1] |= 0x00007C00U;
			}
			if_param_exists(0x0005U, numParams, params)
			{
				filterchars(params[5], "0123456789,-");
				p_str[0] = params[5]; /* Overlaps */
				p_u32[1] |= 0x00008000U;
			}
			if_param_exists(0x0006U, numParams, params) /* Blocking flags */
			{
				if (compareString(params[6], "blocke", 6) == 0)
				{
					p_int[0] = 1;
					p_u32[1] |= 0x00030000U;
				}
				else if (compareString(params[6], "blocka", 6) == 0)
				{
					p_int[1] = 1;
					p_u32[1] |= 0x00030000U;
				}
				else if (compareString(params[6], "bot", 3) == 0)
				{
					p_int[0] = 1;
					p_int[1] = 1;
					p_u32[1] |= 0x00030000U;
				}
			}
			break;
		case HEXDUMP:
			for (l_u16[0] = 0x0000U; l_u16[0] < numParams; ++l_u16[0])
			{
				for (l_u16[1] = 0x0000U;
				     l_u16[1] < (uint16) strlen(params[l_u16[0]]);
				     ++l_u16[1])
				{
					if (params[l_u16[0]][l_u16[1]] == 'o')
					{
						p_u16[0] |= 0x0004U;
					}
					else if (params[l_u16[0]][l_u16[1]] == 'n')
					{
						p_u16[0] |= 0x0001U;
					}
					else if (params[l_u16[0]][l_u16[1]] == 'N')
					{
						p_u16[0] |= 0x0002U;
					}
				}
			}
			break;
	}
	
	/* Makes string copies of parameters if needed */
	for (l_u16[0] = 0x0000U; l_u16[0] < 0x0003U; ++l_u16[0])
	{
		if ((p_str[l_u16[0]] != NULL) && (p_strcpy[l_u16[0]] == NULL))
		{
			p_strcpy[l_u16[0]] = calloc((strlen(p_str[l_u16[0]]) + (size_t) 1U),
			                            (size_t) 1U);
			if (p_strcpy[l_u16[0]] == NULL)
			{
				error->code = ERROR_MEMORY;
				goto end;
			}
			memcpy(p_strcpy[l_u16[0]], p_str[l_u16[0]],
			       strlen(p_str[l_u16[0]]));
		}
	}
	
	/* Calls the desired functions */
	switch (command)
	{
		/* (item, print) */
		case CGET_ITEM:
		case RGET_ITEM:
		case HGET_ITEM:
		case GET_ITEM:
		case GET_SOUNDSOURCE:
		case CGET_SOUNDSOURCE:
		case RGET_SOUNDSOURCE:
		case HGET_SOUNDSOURCE:
		case GET_BOX:
		case CGET_BOX:
		case RGET_BOX:
			/* Determines print flags */
			switch (command)
			{
				case CGET_ITEM:
				case CGET_BOX:
				case CGET_SOUNDSOURCE:
					p_u16[0] = 0x0001U;
					break;
				case RGET_ITEM:
				case RGET_SOUNDSOURCE:
				case RGET_BOX:
					p_u16[0] = 0x0002U;
					break;
				case HGET_ITEM:
				case HGET_SOUNDSOURCE:
					p_u16[0] = 0x0004U;
					break;
			}
			/* Determines what items/soundsource/boxes to get */
			if (p_u32[0] == ALL_U32)
			{
				l_u32[0] = 0x00000000U;
				switch (command)
				{
					case CGET_ITEM:
					case RGET_ITEM:
					case HGET_ITEM:
					case GET_ITEM:
						h_u32[0] = level->value[NUMENTITIES];
						break;
					case GET_SOUNDSOURCE:
					case CGET_SOUNDSOURCE:
					case RGET_SOUNDSOURCE:
					case HGET_SOUNDSOURCE:
						h_u32[0] = level->value[NUMSOUNDSOURCES];
						break;
					case GET_BOX:
					case CGET_BOX:
					case RGET_BOX:
						h_u32[0] = level->value[NUMBOXES];
						break;
				}
			}
			else
			{
				l_u32[0] = p_u32[0];
				h_u32[0] = (p_u32[0] + 0x00000001U);
			}
			/* Calls the functions */
			for (; l_u32[0] < h_u32[0]; ++l_u32[0])
			{
				switch (command)
				{
					case CGET_ITEM:
					case RGET_ITEM:
					case HGET_ITEM:
					case GET_ITEM:
						get_item(level, error, l_u32[0], p_u16[0]);
						break;
					case GET_SOUNDSOURCE:
					case CGET_SOUNDSOURCE:
					case RGET_SOUNDSOURCE:
					case HGET_SOUNDSOURCE:
						get_soundsource(level, error, l_u32[0], p_u16[0]);
						break;
					case GET_BOX:
					case CGET_BOX:
					case RGET_BOX:
						get_box(level, error, l_u32[0], p_u16[0]);
						break;
				}
				if (error->code != ERROR_NONE)
				{
					goto end;
				}
			}
			break;
		/* (room, <print>) */
		case GET_ROOMFLAGS:
		case CGET_ROOMFLAGS:
		case RGET_ROOMFLAGS:
		case GET_WATER:
		case CGET_WATER:
		case RGET_WATER:
		case GET_SFXALWAYS:
		case CGET_SFXALWAYS:
		case RGET_SFXALWAYS:
		case GET_PITCHSHIFT:
		case CGET_PITCHSHIFT:
		case RGET_PITCHSHIFT:
		case GET_WIND:
		case CGET_WIND:
		case RGET_WIND:
		case GET_INSIDE:
		case CGET_INSIDE:
		case RGET_INSIDE:
		case GET_SKY:
		case CGET_SKY:
		case RGET_SKY:
		case GET_DYNAMIC:
		case CGET_DYNAMIC:
		case RGET_DYNAMIC:
		case GET_SWAMP:
		case CGET_SWAMP:
		case RGET_SWAMP:
		case GET_ROOMLIGHT:
		case CGET_ROOMLIGHT:
		case RGET_ROOMLIGHT:
		case SET_ROOMLIGHT:
		case GET_ALTROOM:
		case CGET_ALTROOM:
		case RGET_ALTROOM:
		case SET_ALTROOM:
		case GET_ROOMPOS:
		case CGET_ROOMPOS:
		case RGET_ROOMPOS:
		case GET_ROOMSIZE:
		case CGET_ROOMSIZE:
		case RGET_ROOMSIZE:
		case SET_WATER_ON:
		case SET_WATER_OFF:
		case SET_SFXALWAYS_ON:
		case SET_SFXALWAYS_OFF:
		case SET_PITCHSHIFT_ON:
		case SET_PITCHSHIFT_OFF:
		case SET_WIND_ON:
		case SET_WIND_OFF:
		case SET_INSIDE_ON:
		case SET_INSIDE_OFF:
		case SET_SKY_ON:
		case SET_SKY_OFF:
		case SET_DYNAMIC_ON:
		case SET_DYNAMIC_OFF:
		case SET_SWAMP_ON:
		case SET_SWAMP_OFF:
			/* Determines print flags */
			switch (command)
			{
				case CGET_ROOMFLAGS:
				case CGET_WATER:
				case CGET_SFXALWAYS:
				case CGET_PITCHSHIFT:
				case CGET_WIND:
				case CGET_INSIDE:
				case CGET_SKY:
				case CGET_DYNAMIC:
				case CGET_SWAMP:
				case CGET_ROOMLIGHT:
				case RGET_ROOMLIGHT:
				case CGET_ALTROOM:
				case RGET_ALTROOM:
				case CGET_ROOMPOS:
				case RGET_ROOMPOS:
				case CGET_ROOMSIZE:
				case RGET_ROOMSIZE:
					p_u16[1] = 0x0001U;
					break;
				case RGET_ROOMFLAGS:
				case RGET_WATER:
				case RGET_SFXALWAYS:
				case RGET_PITCHSHIFT:
				case RGET_WIND:
				case RGET_INSIDE:
				case RGET_SKY:
				case RGET_DYNAMIC:
				case RGET_SWAMP:
					p_u16[1] = 0x0003U;
					break;
				case SET_WATER_ON:
					p_u16[1] = 0x0001U;
					break;
				case SET_WATER_OFF:
					p_u16[1] = 0xFFFEU;
					break;
				case SET_SFXALWAYS_ON:
					p_u16[1] = 0x0002U;
					break;
				case SET_SFXALWAYS_OFF:
					p_u16[1] = 0xFFFDU;
					break;
				case SET_PITCHSHIFT_ON:
					p_u16[1] = 0x0004U;
					break;
				case SET_PITCHSHIFT_OFF:
					p_u16[1] = 0xFFFBU;
					break;
				case SET_WIND_ON:
					p_u16[1] = 0x0020U;
					break;
				case SET_WIND_OFF:
					p_u16[1] = 0xFFDFU;
					break;
				case SET_INSIDE_ON:
					p_u16[1] = 0x0040U;
					break;
				case SET_INSIDE_OFF:
					p_u16[1] = 0xFFBFU;
					break;
				case SET_SKY_ON:
					p_u16[1] = 0x0008U;
					break;
				case SET_SKY_OFF:
					p_u16[1] = 0xFFF7;
					break;
				case SET_DYNAMIC_ON:
					p_u16[1] = 0x0010U;
					break;
				case SET_DYNAMIC_OFF:
					p_u16[1] = 0xFFEF;
					break;
				case SET_SWAMP_ON:
					p_u16[1] = 0x0080U;
					break;
				case SET_SWAMP_OFF:
					p_u16[1] = 0xFF7FU;
					break;
			}
			/* Determines what rooms to get */
			if (p_u16[0] == ALL_U16)
			{
				l_u16[0] = 0x0000U;
				h_u16[0] = level->numRooms;
			}
			else
			{
				l_u16[0] = p_u16[0];
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loops through the rooms */
			for (; l_u16[0] < h_u16[0]; ++l_u16[0])
			{
				switch (command)
				{
					case GET_ROOMFLAGS:
					case CGET_ROOMFLAGS:
					case RGET_ROOMFLAGS:
						get_roomflags(level, error, l_u16[0],
						              0xFFFFU, p_u16[1]);
						break;
					case GET_WATER:
					case CGET_WATER:
					case RGET_WATER:
						get_roomflags(level, error, l_u16[0],
						              0x0001U, p_u16[1]);
						break;
					case GET_SFXALWAYS:
					case CGET_SFXALWAYS:
					case RGET_SFXALWAYS:
						get_roomflags(level, error, l_u16[0],
						              0x0002U, p_u16[1]);
						break;
					case GET_PITCHSHIFT:
					case CGET_PITCHSHIFT:
					case RGET_PITCHSHIFT:
						get_roomflags(level, error, l_u16[0],
						              0x0004U, p_u16[1]);
						break;
					case GET_WIND:
					case CGET_WIND:
					case RGET_WIND:
						get_roomflags(level, error, l_u16[0],
						              0x0020U, p_u16[1]);
						break;
					case GET_INSIDE:
					case CGET_INSIDE:
					case RGET_INSIDE:
						get_roomflags(level, error, l_u16[0],
						              0x0020U, p_u16[1]);
						break;
					case GET_SKY:
					case CGET_SKY:
					case RGET_SKY:
						get_roomflags(level, error, l_u16[0],
						              0x0008U, p_u16[1]);
						break;
					case GET_DYNAMIC:
					case CGET_DYNAMIC:
					case RGET_DYNAMIC:
						get_roomflags(level, error, l_u16[0],
						              0x0010U, p_u16[1]);
						break;
					case GET_SWAMP:
					case CGET_SWAMP:
					case RGET_SWAMP:
						get_roomflags(level, error, l_u16[0],
						              0x0080U, p_u16[1]);
						break;
					case GET_ROOMLIGHT:
					case CGET_ROOMLIGHT:
					case RGET_ROOMLIGHT:
						get_roomlight(level, error, l_u16[0], p_u16[1]);
						break;
					case SET_ROOMLIGHT:
						roomlight(level, error, l_u16[0],
						          p_str[0], p_str[1], p_u16[1]);
						break;
					case GET_ALTROOM:
					case CGET_ALTROOM:
					case RGET_ALTROOM:
						get_altroom(level, error, l_u16[0], p_u16[1]);
						break;
					case SET_ALTROOM:
						altroom(level, error, l_u16[0], p_s16[0]);
						break;
					case GET_ROOMPOS:
					case CGET_ROOMPOS:
					case RGET_ROOMPOS:
						get_roompos(level, error, l_u16[0], p_u16[1]);
						break;
					case GET_ROOMSIZE:
					case CGET_ROOMSIZE:
					case RGET_ROOMSIZE:
						get_roomsize(level, error, l_u16[0], p_u16[1]);
						break;
					case SET_WATER_ON:
					case SET_SFXALWAYS_ON:
					case SET_PITCHSHIFT_ON:
					case SET_WIND_ON:
					case SET_INSIDE_ON:
					case SET_SKY_ON:
					case SET_DYNAMIC_ON:
					case SET_SWAMP_ON:
						roomflags(level, error, l_u16[0], p_u16[1], 1);
						break;
					case SET_WATER_OFF:
					case SET_SFXALWAYS_OFF:
					case SET_PITCHSHIFT_OFF:
					case SET_WIND_OFF:
					case SET_INSIDE_OFF:
					case SET_SKY_OFF:
					case SET_DYNAMIC_OFF:
					case SET_SWAMP_OFF:
						roomflags(level, error, l_u16[0], p_u16[1], 0);
						break;
				}
				if (error->code != ERROR_NONE)
				{
					goto end;
				}
			}
			break;
		/* (room, item, print) */
		case GET_STATICMESH:
		case CGET_STATICMESH:
		case RGET_STATICMESH:
		case HGET_STATICMESH:
		case GET_LIGHT:
		case CGET_LIGHT:
		case RGET_LIGHT:
		case HGET_LIGHT:
		case GET_VERTEX:
		case CGET_VERTEX:
		case RGET_VERTEX:
		case HGET_VERTEX:
		case GET_RECTANGLE:
		case CGET_RECTANGLE:
		case RGET_RECTANGLE:
		case HGET_RECTANGLE:
		case GET_TRIANGLE:
		case CGET_TRIANGLE:
		case RGET_TRIANGLE:
		case HGET_TRIANGLE:
		case GET_SPRITE:
		case CGET_SPRITE:
		case RGET_SPRITE:
		case HGET_SPRITE:
		case GET_VIEWPORT:
		case CGET_VIEWPORT:
		case RGET_VIEWPORT:
			/* Determines print flags */
			switch (command)
			{
				case CGET_STATICMESH:
				case CGET_LIGHT:
				case CGET_VERTEX:
				case CGET_RECTANGLE:
				case CGET_TRIANGLE:
				case CGET_SPRITE:
				case CGET_VIEWPORT:
					p_u16[2] = 0x0001U;
					break;
				case RGET_STATICMESH:
				case RGET_LIGHT:
				case RGET_VERTEX:
				case RGET_RECTANGLE:
				case RGET_TRIANGLE:
				case RGET_SPRITE:
				case RGET_VIEWPORT:
					p_u16[2] = 0x0002U;
					break;
				case HGET_STATICMESH:
				case HGET_LIGHT:
				case HGET_RECTANGLE:
				case HGET_TRIANGLE:
				case HGET_SPRITE:
					p_u16[2] = 0x0004U;
					break;
				case HGET_VERTEX:
					p_u16[2] = 0x0010U;
					break;
			}
			/* Determines what rooms to get */
			if (p_u16[0] == ALL_U16)
			{
				l_u16[0] = 0x0000U;
				h_u16[0] = level->numRooms;
			}
			else
			{
				l_u16[0] = p_u16[0];
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Calls the loops through the rooms */
			for (; l_u16[0] < h_u16[0]; ++l_u16[0])
			{
				/* Determines what staticmeshes/lights/vertices/
				 * rectangles/triangles/viewports/sprites to get */
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000U;
					switch (command)
					{
						case GET_STATICMESH:
						case CGET_STATICMESH:
						case RGET_STATICMESH:
						case HGET_STATICMESH:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMSTATICMESHES];
							break;
						case GET_LIGHT:
						case CGET_LIGHT:
						case RGET_LIGHT:
						case HGET_LIGHT:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMLIGHTS];
							break;
						case GET_VERTEX:
						case CGET_VERTEX:
						case RGET_VERTEX:
						case HGET_VERTEX:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMVERTICES];
							break;
						case GET_RECTANGLE:
						case CGET_RECTANGLE:
						case RGET_RECTANGLE:
						case HGET_RECTANGLE:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMRECTANGLES];
							break;
						case GET_TRIANGLE:
						case CGET_TRIANGLE:
						case RGET_TRIANGLE:
						case HGET_TRIANGLE:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMTRIANGLES];
							break;
						case GET_SPRITE:
						case CGET_SPRITE:
						case RGET_SPRITE:
						case HGET_SPRITE:
							h_u16[1] =
								level->room[l_u16[0]].value[R_NUMSPRITES];
							break;
						case GET_VIEWPORT:
						case CGET_VIEWPORT:
						case RGET_VIEWPORT:
							h_u16[1] = level->room[l_u16[0]].value[R_NUMDOORS];
							break;
					}
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				
				/*
				 * Gets the staticmeshes/lights/vertices/
				 * rectangles/triangles/viewports/sprites
				 */
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					switch (command)
					{
						case GET_STATICMESH:
						case CGET_STATICMESH:
						case RGET_STATICMESH:
						case HGET_STATICMESH:
							get_staticmesh(level, error, l_u16[0],
							               l_u16[1], p_u16[2]);
							if (error->code == ERROR_STATICMESH_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_LIGHT:
						case CGET_LIGHT:
						case RGET_LIGHT:
						case HGET_LIGHT:
							get_light(level, error, l_u16[0],
							          l_u16[1], p_u16[2]);
							if (error->code == ERROR_LIGHT_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_VERTEX:
						case CGET_VERTEX:
						case RGET_VERTEX:
						case HGET_VERTEX:
							get_vertex(level, error, l_u16[0],
							           l_u16[1], p_u16[2]);
							if (error->code == ERROR_VERTEX_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_RECTANGLE:
						case CGET_RECTANGLE:
						case RGET_RECTANGLE:
						case HGET_RECTANGLE:
							get_rectangle(level, error, l_u16[0],
							              l_u16[1], p_u16[2]);
							if (error->code == ERROR_RECTANGLE_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_TRIANGLE:
						case CGET_TRIANGLE:
						case RGET_TRIANGLE:
						case HGET_TRIANGLE:
							get_triangle(level, error, l_u16[0],
							             l_u16[1], p_u16[2]);
							if (error->code == ERROR_TRIANGLE_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_SPRITE:
						case CGET_SPRITE:
						case RGET_SPRITE:
						case HGET_SPRITE:
							get_sprite(level, error, l_u16[0],
							           l_u16[1], p_u16[2]);
							if (error->code == ERROR_SPRITE_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
						case GET_VIEWPORT:
						case CGET_VIEWPORT:
						case RGET_VIEWPORT:
							get_viewport(level, error, l_u16[0],
							             l_u16[1], p_u16[2]);
							if (error->code == ERROR_VIEWPORT_DOESNT_EXIST)
							{
								error->code = ERROR_NONE;
							}
							break;
					}
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
				}
			}
			break;
		/* (room, column, row, <type>, print) */
		case GET_FLOORDATA:
		case CGET_FLOORDATA:
		case RGET_FLOORDATA:
		case GET_FLOOR:
		case CGET_FLOOR:
		case RGET_FLOOR:
		case HGET_FLOOR:
		case GET_CEILING:
		case CGET_CEILING:
		case RGET_CEILING:
		case HGET_CEILING:
		case GET_ZONE:
		case CGET_ZONE:
		case RGET_ZONE:
		case GET_STEPSOUND:
		case CGET_STEPSOUND:
		case RGET_STEPSOUND:
			/* Determines print flags */
			switch (command)
			{
				case CGET_FLOORDATA:
				case RGET_FLOORDATA:
				case CGET_FLOOR:
				case RGET_FLOOR:
				case CGET_CEILING:
				case RGET_CEILING:
				case CGET_ZONE:
				case RGET_ZONE:
				case CGET_STEPSOUND:
				case RGET_STEPSOUND:
					p_u16[3] = 0x0001U;
					break;
				case HGET_FLOOR:
				case HGET_CEILING:
					p_u16[3] = 0x0002U;
					break;
			}
			/* Determines what rooms to get */
			if (p_u16[0] == ALL_U16)
			{
				l_u16[0] = 0x0000U;
				h_u16[0] = level->numRooms;
			}
			else
			{
				l_u16[0] = p_u16[0];
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loops through the rooms */
			for (; l_u16[0] < h_u16[0]; ++l_u16[0])
			{
				/* Determines what columns to get */
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000U;
					h_u16[1] = level->room[l_u16[0]].value[R_NUMXSECTORS];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				
				/* Loops through the columns */
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					/* Determines what rows to get */
					if (p_u16[2] == ALL_U16)
					{
						l_u16[2] = 0x0000U;
						h_u16[2] = level->room[l_u16[0]].value[R_NUMZSECTORS];
					}
					else
					{
						l_u16[2] = p_u16[2];
						h_u16[2] = (p_u16[2] + 0x0001U);
					}
					
					/* Loops through the rows */
					for (; l_u16[2] < h_u16[2]; ++l_u16[2])
					{
						switch (command)
						{
							case GET_FLOORDATA:
							case CGET_FLOORDATA:
							case RGET_FLOORDATA:
								get_floordata(level, error, l_u16[0], l_u16[1],
								              l_u16[2], p_u16[3]);
								break;
							case GET_FLOOR:
							case CGET_FLOOR:
							case RGET_FLOOR:
							case HGET_FLOOR:
								get_floor_ceiling(level, error, l_u16[0],
								                  l_u16[1], l_u16[2], 0,
								                  p_u16[3]);
								break;
							case GET_CEILING:
							case CGET_CEILING:
							case RGET_CEILING:
							case HGET_CEILING:
								get_floor_ceiling(level, error, l_u16[0],
								                  l_u16[1], l_u16[2], 1,
								                  p_u16[3]);
								break;
							case GET_ZONE:
							case CGET_ZONE:
							case RGET_ZONE:
								get_zone(level, error, l_u16[0], l_u16[1],
								         l_u16[2], p_u16[3]);
								break;
							case GET_STEPSOUND:
							case CGET_STEPSOUND:
							case RGET_STEPSOUND:
								get_stepsound(level, error, l_u16[0], l_u16[1],
								              l_u16[2], p_u16[3]);
								break;
						}
						if (error->code != ERROR_NONE)
						{
							goto end;
						}
					}
				}
			}
			break;
		/* (item, <flags>) */
		case REMOVE_ITEM:
			/* Determines what items to remove */
			if (p_u32[0] == ALL_U32)
			{
				remove_all_items(level, error);
			}
			else
			{
				remove_item(level, error, p_u32[0], p_u16[0]);
			}
			break;
		case REMOVE_SOUNDSOURCE:
			/* Determines what soundsources to remove */
			if (p_u32[0] == ALL_U32)
			{
				remove_all_soundsources(level, error);
			}
			else
			{
				remove_soundsource(level, error, p_u32[0]);
			}
			break;
		/* (room, item) */
		case REMOVE_STATICMESH:
		case REMOVE_LIGHT:
		case REMOVE_VERTEX:
		case REMOVE_GEOMETRY:
		case REMOVE_RECTANGLE:
		case REMOVE_TRIANGLE:
		case REMOVE_SPRITE:
		case REMOVE_VIEWPORT:
			/* Determines what rooms to get */
			if (p_u16[0] == ALL_U16)
			{
				l_u16[0] = 0x0000U;
				h_u16[0] = level->numRooms;
			}
			else
			{
				l_u16[0] = p_u16[0];
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loops through the rooms */
			for (; l_u16[0] < h_u16[0]; ++l_u16[0])
			{
				/* Remove all of them */
				if (p_u16[1] == ALL_U16)
				{
					switch (command)
					{
						case REMOVE_STATICMESH:
							remove_all_staticmeshes(level, error, l_u16[0]);
							break;
						case REMOVE_LIGHT:
							remove_all_lights(level, error, l_u16[0]);
							break;
						case REMOVE_VERTEX:
						case REMOVE_GEOMETRY:
							remove_geometry(level, error, l_u16[0]);
							break;
						case REMOVE_RECTANGLE:
							remove_all_rectangles(level, error, l_u16[0]);
							break;
						case REMOVE_TRIANGLE:
							remove_all_triangles(level, error, l_u16[0]);
							break;
						case REMOVE_SPRITE:
							remove_all_sprites(level, error, l_u16[0]);
							break;
						case REMOVE_VIEWPORT:
							remove_all_viewports(level, error, l_u16[0]);
							break;
					}
				}
				/* Only remove one of them */
				else
				{
					switch (command)
					{
						case REMOVE_STATICMESH:
							remove_staticmesh(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_LIGHT:
							remove_light(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_VERTEX:
							remove_vertex(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_GEOMETRY:
							remove_geometry(level, error, l_u16[0]);
							break;
						case REMOVE_RECTANGLE:
							remove_rectangle(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_TRIANGLE:
							remove_triangle(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_SPRITE:
							remove_sprite(level, error, l_u16[0], p_u16[1]);
							break;
						case REMOVE_VIEWPORT:
							remove_viewport(level, error, l_u16[0], p_u16[1]);
							break;
					}
				}
				if (error->code != ERROR_NONE)
				{
					goto end;
				}
			}
			break;
		/* (room, column, row, parameter) */
		case REMOVE_FLOORDATA:
			/* Special case: runs remove_all_floordata if needed */
			if ((p_u16[0] == ALL_U16) && (p_u16[1] == ALL_U16) &&
			    (p_u16[2] == ALL_U16))
			{
				remove_all_floordata(level, error);
				break;
			}
		case REPLACE_FLOORDATA:
		case SET_FLOOR:
		case SET_CEILING:
		case SET_ZONE:
		case SET_STEPSOUND:
			/* Determines what rooms to get */
			if (p_u16[0] == ALL_U16)
			{
				l_u16[0] = 0x0000U;
				h_u16[0] = level->numRooms;
			}
			else
			{
				l_u16[0] = p_u16[0];
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loops through the rooms */
			for (; l_u16[0] < h_u16[0]; ++l_u16[0])
			{
				/* Determines what columns to get */
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000U;
					h_u16[1] = level->room[l_u16[0]].value[R_NUMXSECTORS];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				
				/* Loops through the columns */
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					/* Determines what rows to get */
					if (p_u16[2] == ALL_U16)
					{
						l_u16[2] = 0x0000U;
						h_u16[2] = level->room[l_u16[0]].value[R_NUMZSECTORS];
					}
					else
					{
						l_u16[2] = p_u16[2];
						h_u16[2] = (p_u16[2] + 0x0001U);
					}
					
					/* Loops through the rows */
					for (; l_u16[2] < h_u16[2]; ++l_u16[2])
					{
						switch (command)
						{
							case REPLACE_FLOORDATA:
								memcpy(p_str[0], p_strcpy[0],
								       strlen(p_strcpy[0]));
								replace_floordata(level, error, l_u16[0],
								                  l_u16[1], l_u16[2],
								                  p_str[0], 0x0001);
								break;
							case REMOVE_FLOORDATA:
								remove_floordata(level, error, l_u16[0],
								                 l_u16[1], l_u16[2]);
								break;
							case SET_FLOOR:
							case SET_CEILING:
								set_floor_ceiling(level, error, l_u16[0],
								                  l_u16[1], l_u16[2],
								                  p_s32[0], p_u16[3]);
								break;
							case SET_ZONE:
								set_zone(level, error, l_u16[0],
								         l_u16[1], l_u16[2], p_s16[0]);
								break;
							case SET_STEPSOUND:
								set_stepsound(level, error, l_u16[0],
								              l_u16[1], l_u16[2], p_u16[3]);
								break;
						}
						if (error->code != ERROR_NONE)
						{
							goto end;
						}
					}
				}
			}
			break;
		case ADD_ITEM:
			add_item(level, error, p_u16[0], p_u16[1], p_s32[0], p_s32[1],
			         p_s32[2], p_s16[0], p_str[0], p_str[1], p_u16[2]);
			break;
		case REPLACE_ITEM:
			if (p_u32[0] == ALL_U32)
			{
				for (p_u32[0] = 0x00000000;
				     p_u32[0] < level->value[NUMENTITIES]; ++p_u32[0])
				{
					if (p_str[0] != NULL)
					{
						memcpy(p_str[0], p_strcpy[0], strlen(p_strcpy[0]));
					}
					if (p_str[1] != NULL)
					{
						memcpy(p_str[1], p_strcpy[1], strlen(p_strcpy[1]));
					}
					replace_item(level, error, p_u32[0], p_u16[0],
					             p_u16[1], p_s32[0], p_s32[1], p_s32[2],
					             p_s16[0], p_str[0], p_str[1], p_u16[2],
					             p_u16[3]);
				}
			}
			else
			{
				replace_item(level, error, p_u32[0], p_u16[0],
				             p_u16[1], p_s32[0], p_s32[1], p_s32[2],
				             p_s16[0], p_str[0], p_str[1], p_u16[2], p_u16[3]);
			}
			break;
		case ADD_STATICMESH:
			add_staticmesh(level, error, p_u16[0], p_u16[1], p_s32[0],
			               p_s32[1], p_s32[2], p_s16[0], p_str[0], p_str[1]);
			break;
		case REPLACE_STATICMESH:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMSTATICMESHES];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					if (p_str[0] != NULL)
					{
						memcpy(p_str[0], p_strcpy[0], strlen(p_strcpy[0]));
					}
					if (p_str[1] != NULL)
					{
						memcpy(p_str[1], p_strcpy[1], strlen(p_strcpy[1]));
					}
					replace_staticmesh(level, error, l_u16[1], p_u16[2],
					                   p_u16[0], p_s32[0], p_s32[1], p_s32[2],
					                   p_s16[0], p_str[0], p_str[1], p_u16[3]);
				}
			}
			break;
		case ADD_LIGHT:
			add_light(level, error, p_u16[0], p_s32[0], p_s32[1],
			          p_s32[2], p_str[0], p_str[1], p_u32[0], p_u32[1],
			          p_u8[0], p_s16[0], p_s16[1], p_s16[2]);
			break;
		case REPLACE_LIGHT:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMLIGHTS];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					if (p_str[0] != NULL)
					{
						memcpy(p_str[0], p_strcpy[0], strlen(p_strcpy[0]));
					}
					if (p_str[1] != NULL)
					{
						memcpy(p_str[1], p_strcpy[1], strlen(p_strcpy[1]));
					}
					replace_light(level, error, p_u16[0], l_u16[1],
					              p_s32[0], p_s32[1], p_s32[2], p_str[0],
					              p_str[1], p_u32[0], p_u32[1], p_u8[0],
					              p_s16[0], p_s16[1], p_s16[2], p_u16[2]);
				}
			}
			break;
		case ADD_SOUNDSOURCE:
			add_soundsource(level, error, p_u16[0], p_u16[1],
			                p_s32[0], p_s32[1], p_s32[2], p_u16[2]);
			break;
		case REPLACE_SOUNDSOURCE:
			if (p_u32[3] == ALL_U32)
			{
				for (p_u32[3] = 0x00000000;
				     p_u32[3] < level->value[NUMSOUNDSOURCES];
				     ++p_u32[3])
				{
					replace_soundsource(level, error, p_u32[3], p_u16[0],
					                    p_u16[1], p_s32[0], p_s32[1],
					                    p_s32[2], p_u16[3], p_u16[2]);
				}
			}
			else
			{
				replace_soundsource(level, error, p_u32[3], p_u16[0],
				                    p_u16[1], p_s32[0], p_s32[1],
				                    p_s32[2], p_u16[3], p_u16[2]);
			}
			break;
		case ADD_VERTEX:
			add_vertex(level, error, p_u16[0], p_s32[0], p_s32[1],
			           p_s32[2], p_str[0], p_str[1], p_u16[1]);
			break;
		case REPLACE_VERTEX:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMVERTICES];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					if (p_str[0] != NULL)
					{
						memcpy(p_str[0], p_strcpy[0], strlen(p_strcpy[0]));
					}
					if (p_str[1] != NULL)
					{
						memcpy(p_str[1], p_strcpy[1], strlen(p_strcpy[1]));
					}
					replace_vertex(level, error, p_u16[0], l_u16[1],
					               p_s32[0], p_s32[1], p_s32[2],
					               p_str[0], p_str[1], p_u16[2], p_u16[3]);
				}
			}
			break;
		case ADD_RECTANGLE:
			add_rectangle(level, error, p_u16[0], p_u16[1], p_u16[2],
			              p_u16[3], p_u16[4], p_u16[5], p_u16[6]);
			break;
		case REPLACE_RECTANGLE:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[7] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMRECTANGLES];
				}
				else
				{
					l_u16[1] = p_u16[7];
					h_u16[1] = (p_u16[7] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					replace_rectangle(level, error, p_u16[0], l_u16[1],
					                  p_u16[1], p_u16[2], p_u16[3],
					                  p_u16[4], p_u16[5], p_u16[6], p_u16[8]);
				}
			}
			break;
		case ADD_TRIANGLE:
			add_triangle(level, error, p_u16[0], p_u16[1],
			             p_u16[2], p_u16[3], p_u16[4], p_u16[5]);
			break;
		case REPLACE_TRIANGLE:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[7] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMTRIANGLES];
				}
				else
				{
					l_u16[1] = p_u16[7];
					h_u16[1] = (p_u16[7] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					replace_triangle(level, error, p_u16[0], l_u16[1],
					                 p_u16[1], p_u16[2], p_u16[3],
					                 p_u16[5], p_u16[6], p_u16[8]);
				}
			}
			break;
		case ADD_SPRITE:
			add_sprite(level, error, p_u16[0], p_u16[1], p_s32[0],
			           p_s32[1], p_s32[2], p_str[0], p_str[1], p_u16[2]);
			break;
		case REPLACE_SPRITE:
			if (p_u16[1] == ALL_U16)
			{
				p_u16[1] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[1] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[1] < h_u16[0]; ++p_u16[1])
			{
				if (p_u16[3] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[1]].value[R_NUMSPRITES];
				}
				else
				{
					l_u16[1] = p_u16[3];
					h_u16[1] = (p_u16[3] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					if (p_str[0] != NULL)
					{
						memcpy(p_str[0], p_strcpy[0], strlen(p_strcpy[0]));
					}
					if (p_str[1] != NULL)
					{
						memcpy(p_str[1], p_strcpy[1], strlen(p_strcpy[1]));
					}
					replace_sprite(level, error, p_u16[1], l_u16[1],
					               p_u16[0], p_s32[0], p_s32[1], p_s32[2],
					               p_str[0], p_str[1], p_u16[2], p_u16[4]);
				}
			}
			break;
		case ADD_VIEWPORT:
			add_viewport(level, error, p_u16[0], p_u16[1], p_s16[0], p_s16[2],
			             p_s16[1], p_s16[3], p_s16[5], p_s16[4], p_s16[6],
			             p_s16[8], p_s16[7], p_s16[9], p_s16[11], p_s16[10],
			             p_s16[12], p_s16[14], p_s16[13]);
			break;
		case REPLACE_VIEWPORT:
			if (p_u16[0] == ALL_U16)
			{
				p_u16[0] = 0x0000;
				h_u16[0] = level->value[NUMROOMS];
			}
			else
			{
				h_u16[0] = (p_u16[0] + 0x0001U);
			}
			/* Loop through needed rooms */
			for (; p_u16[0] < h_u16[0]; ++p_u16[0])
			{
				if (p_u16[1] == ALL_U16)
				{
					l_u16[1] = 0x0000;
					h_u16[1] = level->room[p_u16[0]].value[R_NUMDOORS];
				}
				else
				{
					l_u16[1] = p_u16[1];
					h_u16[1] = (p_u16[1] + 0x0001U);
				}
				for (; l_u16[1] < h_u16[1]; ++l_u16[1])
				{
					replace_viewport(level, error, p_u16[0], l_u16[1], p_u16[2],
					                 p_s16[0], p_s16[1], p_s16[2], p_s16[3],
					                 p_s16[4], p_s16[5], p_s16[6], p_s16[7],
					                 p_s16[8], p_s16[9], p_s16[10], p_s16[11],
					                 p_s16[12], p_s16[13], p_s16[14], p_u16[3]);
				}
			}
			break;
		case MOVE_ROOM:
			move_room(level, error, p_u16[0],
			          p_s32[0], p_s32[1], p_s32[2], 0x003FU);
			break;
		case RESIZE_ROOM:
			resize_room(level, error, p_u16[0], p_u16[1], p_u16[2], p_s32[0]);
			break;
		case ADD_BOX:
			add_box(level, error, p_u16[0], p_s32[0], p_s32[2], p_s32[1],
			        p_s32[3], p_s16[0], p_s16[1], p_s16[2], p_s16[3], p_s16[4],
			        p_s16[5], p_s16[6], p_s16[7], p_s16[8], p_s16[9], p_s16[10],
			        p_str[0], p_int[0], p_int[1]);
			break;
		case REPLACE_BOX:
			replace_box(level, error, p_u32[0], p_u16[0], p_s32[0], p_s32[2],
			            p_s32[1], p_s32[3], p_s16[0], p_s16[1], p_s16[2],
			            p_s16[3], p_s16[4], p_s16[5], p_s16[6], p_s16[7],
			            p_s16[8], p_s16[9], p_s16[10], p_str[0], p_int[0],
			            p_int[1], p_u32[1]);
			break;
		case REMOVE_ALL_BOXES:
			remove_all_boxes(level, error);
			break;
		case ADD_ROOM:
			add_room(level, error);
			break;
		case REMOVE_ALL_ROOMS:
			remove_all_rooms(level, error);
			break;
		case HEXDUMP:
			hexdump(level, error, p_u16[0]);
			break;
		case VARDUMP:
			vardump(level);
			break;
	}
	
end:/* Frees allocated memory */
	if (subParams != NULL)
	{
		free(subParams);
	}
	for (l_u16[0] = 0x0000U; l_u16[0] < 0x0003U; ++l_u16[0])
	{
		if (p_strcpy[l_u16[0]] != NULL)
		{
			free(p_strcpy[l_u16[0]]);
		}
	}
}

/*
 * Function that prints the usage for every function in the program
 * Parameters:
 * * arg0 = The command that started this program
 */
void showUsage(char *arg0)
{
	/* Variable Declarations */
	int cur = 1; /* Current command */
	
	for (cur = 1; cur < NUM_COMMANDS; ++cur)
	{
		trmod_printf("%s [type] [level] \"%s%s\"\n",
		             arg0, cmd_string[cur], cmd_usage[cur]);
	}
}
