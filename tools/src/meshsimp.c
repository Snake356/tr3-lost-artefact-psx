/*
 * Program that reduces the number of vertices in a mesh
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT8 signed char
#define INT16 signed short
#define INT32 signed int
#define INTU8 unsigned char
#define INTU16 unsigned short
#define INTU32 unsigned int

#define FACE_TEXTURED 0x0000
#define FACE_COLOURED 0x0001

#define ERROR_NONE       0
#define ERROR_PARAMETERS 1
#define ERROR_FILE_OPEN  2
#define ERROR_FILE_READ  3
#define ERROR_MEMORY     4

struct triangle
{
	INTU16 vertex[3];
	INTU16 texture;
	INTU16 type;
	INTU16 group;
};
struct rectangle
{
	INTU16 vertex[4];
	INTU16 texture;
	INTU16 type;
	INTU16 group;
};
struct vertex
{
	INT16 x, y, z;
	INTU16 used;
	INTU16 outIndex;
};

static void swapVertices(struct vertex *vertices, struct rectangle *rectangles,
                         struct triangle *triangles, struct vertex *normals,
						 INT16 *lights, INTU16 numRectangles,
                         INTU16 numTriangles, INTU16 i, INTU16 j, int normalsused);
static void mergeVertices(struct vertex *vertices, struct rectangle *rectangles,
                          struct triangle *triangles, struct vertex *normals,
                          INT16 *lights, INTU16 numRectangles,
                          INTU16 numTriangles, INTU16 i, INTU16 j);
static INTU16 distance16(INT16 x1, INT16 y1, INT16 z1,
                         INT16 x2, INT16 y2, INT16 z2);


int main(int argc, char **argv)
{
	/* Variable Declarations */
	FILE *infile = NULL;                 /* File pointer to input file */
	char *input = NULL;                  /* Input in memory */
	INTU32 insize = 0x00000000;          /* Length of the input file */
	struct vertex *vertices = NULL;      /* In-memory vertices */
	struct vertex *normals = NULL;       /* In-memory normals */
	INT16 *lights = NULL;                /* In-memory lights */
	struct triangle *triangles = NULL;   /* In-memory triangles */
	struct rectangle *rectangles = NULL; /* In-memory rectangles */
	INTU16 numVertices = 0x0000;         /* Number of vertices */
	INTU16 numNormals = 0x0000;          /* Number of normals */
	INTU16 numTriangles = 0x0000;        /* Number of triangles */
	INTU16 numRectangles = 0x0000;       /* Number of rectangles */
	int normalsused = 0;
	INTU16 curVertex = 0x0000;
	INTU16 curTriangle = 0x0000;         /* Current triangle */
	INTU16 curRectangle = 0x0000;        /* Current rectangle */
	INTU32 curpos = 0x00000000;          /* Current position in input */
	INTU16 *numGroupVerts = NULL;        /* Number of vertices in each group */
	INTU16 outVerts = 0x0000;
	INTU16 bestdist = 0x0000;
	INTU16 candidates[2];
	INTU16 i, j;
	int seekval = 0;                     /* Return value of fseek() */
	size_t readval = (size_t) 0;         /* Return value for fread() */
	INTU32 freeuint321 = 0x00000000;     /* Unsigned 32-bit integer */
	INTU16 freeuint161 = 0x0000;         /* Unsigned 16-bit integer */
	int retval = ERROR_NONE;             /* Return value for the program */
	
	/* Checks number of input parameters */
	if (argc < 3)
	{
		printf("Usage: meshsimp [file] [numvertices]\n");
		retval = ERROR_PARAMETERS;
		goto end;
	}
	
	/* Determines the input file size */
	infile = fopen(argv[1], "rb");
	if (infile == NULL)
	{
		printf("%s could not be opened\n", argv[1]);
		retval = ERROR_FILE_OPEN;
		goto end;
	}
	seekval = fseek(infile, 0l, SEEK_END);
	if (seekval != 0)
	{
		printf("%s could not be read from\n", argv[1]);
		retval = ERROR_FILE_READ;
		goto end;
	}
	insize = (INTU32) ftell(infile);
	
	/* Determines output vertices */
	outVerts = (INTU16) strtol(argv[2], NULL, 10);
	
	/* Allocates input */
	input = calloc((size_t) (insize + 0x00000020), (size_t) 1);
	if (input == NULL)
	{
		printf("Memory could not be allocated\n");
		retval = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads input into memory */
	seekval = fseek(infile, 0l, SEEK_SET);
	if (seekval != 0)
	{
		printf("%s could not be read from\n", argv[1]);
		retval = ERROR_FILE_READ;
		goto end;
	}
	readval = fread(input, (size_t) 1, (size_t) insize, infile);
	if (readval != (size_t) insize)
	{
		printf("%s could not be read from\n", argv[1]);
		retval = ERROR_FILE_READ;
		goto end;
	}
	(void) fclose(infile);
	infile = NULL;
	
	/* Counts the elements */
	for (curpos = 0x00000000; curpos < insize; ++curpos)
	{
		if (memcmp(&(input[curpos]), "vertex", 6) == 0)
		{
			++numVertices;
		}
		else if (memcmp(&(input[curpos]), "normal", 6) == 0)
		{
			++numNormals;
		}
		else if (memcmp(&(input[curpos]), "texturedrectangle", 17) == 0)
		{
			++numRectangles;
		}
		else if (memcmp(&(input[curpos]), "colouredrectangle", 17) == 0)
		{
			++numRectangles;
		}
		else if (memcmp(&(input[curpos]), "texturedtriangle", 16) == 0)
		{
			++numTriangles;
		}
		else if (memcmp(&(input[curpos]), "colouredtriangle", 16) == 0)
		{
			++numTriangles;
		}
	}
	
	/* Allocates the needed structs */
	if (numNormals > 0x0000)
	{
		normalsused = 1;
	}
	else
	{
		normalsused = 0;
	}
	vertices = calloc((size_t) numVertices, sizeof(struct vertex));
	normals = calloc((size_t) numVertices, sizeof(struct vertex));
	lights = calloc((size_t) numVertices, (size_t) 2);
	rectangles = calloc((size_t) numRectangles, sizeof(struct rectangle));
	triangles = calloc((size_t) numTriangles, sizeof(struct triangle));
	if ((vertices == NULL) || (normals == NULL) ||
	    (rectangles == NULL) || (triangles == NULL) || (lights == NULL))
	{
		printf("Memory could not be allocated\n");
		retval = ERROR_MEMORY;
		goto end;
	}

	/* Reads the elements into memory */
	numVertices = 0x0000;
	numNormals = 0x0000;
	numRectangles = 0x0000;
	numTriangles = 0x0000;
	for (curpos = 0x00000000; curpos < insize; ++curpos)
	{
		if (memcmp(&(input[curpos]), "vertex", 6) == 0)
		{
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			vertices[numVertices].x = (INT16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			vertices[numVertices].y = (INT16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			vertices[numVertices].z = (INT16) (strtol(&input[curpos], NULL, 10));
			++numVertices;
		}
		else if (memcmp(&(input[curpos]), "normal", 6) == 0)
		{
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			normals[numNormals].x = (INT16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			normals[numNormals].y = (INT16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			normals[numNormals].z = (INT16) (strtol(&input[curpos], NULL, 10));
			++numNormals;
		}
		else if (memcmp(&(input[curpos]), "light", 5) == 0)
		{
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			lights[numNormals] = (INT16) (strtol(&input[curpos], NULL, 10));
			++numNormals;
		}
		else if (memcmp(&(input[curpos]), "texturedrectangle", 17) == 0)
		{
			rectangles[numRectangles].type = FACE_TEXTURED;
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[0] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[1] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[2] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[3] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].texture = (INTU16) (strtol(&input[curpos], NULL, 10));
			++numRectangles;
		}
		else if (memcmp(&(input[curpos]), "colouredrectangle", 17) == 0)
		{
			rectangles[numRectangles].type = FACE_COLOURED;
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[0] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[1] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[2] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].vertex[3] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			rectangles[numRectangles].texture = (INTU16) (strtol(&input[curpos], NULL, 10));
			++numRectangles;
		}
		else if (memcmp(&(input[curpos]), "texturedtriangle", 16) == 0)
		{
			triangles[numTriangles].type = FACE_TEXTURED;
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[0] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[1] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[2] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].texture = (INTU16) (strtol(&input[curpos], NULL, 10));
			++numTriangles;
		}
		else if (memcmp(&(input[curpos]), "colouredtriangle", 16) == 0)
		{
			triangles[numTriangles].type = FACE_COLOURED;
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == '{')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[0] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[1] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].vertex[2] = (INTU16) (strtol(&input[curpos], NULL, 10));
			for (freeuint321 = (curpos + 0x00000001); freeuint321 < insize;
			     ++freeuint321)
			{
				if (input[freeuint321] == ',')
				{
					curpos = (freeuint321 + 0x00000001);
					break;
				}
			}
			triangles[numTriangles].texture = (INTU16) (strtol(&input[curpos], NULL, 10));
			++numTriangles;
		}
	}

	/* Determines unused vertices */
	for (curRectangle = 0x0000; curRectangle < numRectangles; ++curRectangle)
	{
		vertices[rectangles[curRectangle].vertex[0]].used++;
		vertices[rectangles[curRectangle].vertex[1]].used++;
		vertices[rectangles[curRectangle].vertex[2]].used++;
		vertices[rectangles[curRectangle].vertex[3]].used++;
	}
	for (curTriangle = 0x0000; curTriangle < numTriangles; ++curTriangle)
	{
		vertices[triangles[curTriangle].vertex[0]].used++;
		vertices[triangles[curTriangle].vertex[1]].used++;
		vertices[triangles[curTriangle].vertex[2]].used++;
	}

	/* Removes unused vertices */
	i = 0x0000;
	j = (numVertices - 0x0001);
	while (j > i)
	{
		while ((vertices[i].used > 0x0000) && (j > i))
		{
			++i;
		}
		while ((vertices[j].used == 0x0000) && (j > i))
		{
			--numVertices;
			--j;
		}
		if (j > i)
		{
			swapVertices(vertices, rectangles, triangles, normals, lights,
			             numRectangles, numTriangles, i, j, normalsused);
			--numVertices;
			--j;
		}
	}
	
	/* Reduces needed vertices through merging */
	while (numVertices > outVerts)
	{
		/* Finds the closest vertices */
		bestdist = 0xFFFF;
		for (i = 0x0000; i < numVertices; ++i)
		{
			for (j = (i + 0x0001); j < numVertices; ++j)
			{
				if ((vertices[i].x == vertices[j].x) &&
				    (vertices[i].y == vertices[j].y) &&
				    (vertices[i].z == vertices[j].z))
				{
					bestdist = 0x0000;
					candidates[0] = i;
					candidates[1] = j;
					i = numVertices;
					j = numVertices;
					break;
				}
				
				freeuint161 = distance16(vertices[i].x, vertices[i].y, vertices[i].z,
				                         vertices[j].x, vertices[j].y, vertices[j].z);
				if (freeuint161 < bestdist)
				{
					bestdist = freeuint161;
					candidates[0] = i;
					candidates[1] = j;
				}
			}
		}
		mergeVertices(vertices, rectangles, triangles, normals, lights,
		              numRectangles, numTriangles, candidates[0], candidates[1]);
		--numVertices;
		swapVertices(vertices, rectangles, triangles, normals, lights,
		             numRectangles, numTriangles, candidates[1], numVertices, normalsused);
	}
	
	/* Prints the resulting mesh */
	for (curVertex = 0x0000; curVertex < numVertices; ++curVertex)
	{
		printf("vertex(%u){%i,%i,%i}\n", curVertex, vertices[curVertex].x,
		       vertices[curVertex].y, vertices[curVertex].z);
	}
	if (normalsused == 1)
	{
		for (curVertex = 0x0000; curVertex < numVertices; ++curVertex)
		{
			printf("normal(%u){%i,%i,%i}\n", curVertex, normals[curVertex].x,
			       normals[curVertex].y, normals[curVertex].z);
		}
	}
	else
	{
		for (curVertex = 0x0000; curVertex < numVertices; ++curVertex)
		{
			printf("light(%u){%i}\n", curVertex, lights[curVertex]);
		}
	}
	for (curRectangle = 0x0000; curRectangle < numRectangles; ++curRectangle)
	{
		if (rectangles[curRectangle].type == FACE_COLOURED)
		{
			printf("colouredrectangle(){");
		}
		else
		{
			printf("texturedrectangle(){");
		}
		printf("%u,%u,%u,%u,%u}\n",
		       rectangles[curRectangle].vertex[0],
		       rectangles[curRectangle].vertex[1],
		       rectangles[curRectangle].vertex[2],
		       rectangles[curRectangle].vertex[3],
		       rectangles[curRectangle].texture);
	}
	for (curTriangle = 0x0000; curTriangle < numTriangles; ++curTriangle)
	{
		if (triangles[curTriangle].type == FACE_COLOURED)
		{
			printf("colouredtriangle(){");
		}
		else
		{
			printf("texturedtriangle(){");
		}
		printf("%u,%u,%u,%u}\n",
		       triangles[curTriangle].vertex[0],
		       triangles[curTriangle].vertex[1],
		       triangles[curTriangle].vertex[2],
		       triangles[curTriangle].texture);
	}
	
end:
	if (input != NULL)
	{
		free(input);
	}
	if (infile != NULL)
	{
		(void) fclose(infile);
	}
	if (vertices != NULL)
	{
		free(vertices);
	}
	if (normals != NULL)
	{
		free(normals);
	}
	if (rectangles != NULL)
	{
		free(rectangles);
	}
	if (triangles != NULL)
	{
		free(triangles);
	}
	if (lights != NULL)
	{
		free(lights);
	}
	if (numGroupVerts != NULL)
	{
		free(numGroupVerts);
	}
	return retval;
}

static void swapVertices(struct vertex *vertices, struct rectangle *rectangles,
                         struct triangle *triangles, struct vertex *normals,
						 INT16 *lights, INTU16 numRectangles,
                         INTU16 numTriangles, INTU16 i, INTU16 j, int normalsused)
{
	/* Variable Declarations */
	INTU16 curVertex;
	INTU16 curRectangle;
	INTU16 curTriangle;
	struct vertex vertex;
	INT16 light;

	/* Swaps the vertices */
	memmove(&vertex, &(vertices[i]), sizeof(struct vertex));
	memmove(&(vertices[i]), &(vertices[j]), sizeof(struct vertex));
	memmove(&(vertices[j]), &vertex, sizeof(struct vertex));
	if (normalsused == 0)
	{
		light = lights[i];
		lights[i] = lights[j];
		lights[j] = light;
	}
	else
	{
		memmove(&vertex, &(normals[i]), sizeof(struct vertex));
		memmove(&(normals[i]), &(normals[j]), sizeof(struct vertex));
		memmove(&(normals[j]), &vertex, sizeof(struct vertex));
	}
	
	/* Swaps all references to them */
	for (curRectangle = 0x0000; curRectangle < numRectangles; ++curRectangle)
	{
		for (curVertex = 0x0000; curVertex < 0x0004; ++curVertex)
		{
			if (rectangles[curRectangle].vertex[curVertex] == i)
			{
				rectangles[curRectangle].vertex[curVertex] = j;
			}
			else if (rectangles[curRectangle].vertex[curVertex] == j)
			{
				rectangles[curRectangle].vertex[curVertex] = i;
			}
		}
	}
	for (curTriangle = 0x0000; curTriangle < numTriangles; ++curTriangle)
	{
		for (curVertex = 0x0000; curVertex < 0x0003; ++curVertex)
		{
			if (triangles[curTriangle].vertex[curVertex] == i)
			{
				triangles[curTriangle].vertex[curVertex] = j;
			}
			else if (triangles[curTriangle].vertex[curVertex] == j)
			{
				triangles[curTriangle].vertex[curVertex] = i;
			}
		}
	}
}

static void mergeVertices(struct vertex *vertices, struct rectangle *rectangles,
                          struct triangle *triangles, struct vertex *normals,
                          INT16 *lights, INTU16 numRectangles,
                          INTU16 numTriangles, INTU16 i, INTU16 j)
{
	/* Variable Declarations */
	INTU16 curVertex;
	INTU16 curRectangle;
	INTU16 curTriangle;
	
	/* Merges the vertices */
	vertices[i].x += ((vertices[j].x - vertices[i].x) / 0x0002);
	vertices[i].y += ((vertices[j].y - vertices[i].y) / 0x0002);
	vertices[i].z += ((vertices[j].z - vertices[i].z) / 0x0002);
	normals[i].x += ((normals[j].x - normals[i].x) / 0x0002);
	normals[i].y += ((normals[j].y - normals[i].y) / 0x0002);
	normals[i].z += ((normals[j].z - normals[i].z) / 0x0002);
	lights[i] += ((lights[j] - lights[i]) / 0x0002);
	
	/* Merges all references to them */
	for (curRectangle = 0x0000; curRectangle < numRectangles; ++curRectangle)
	{
		for (curVertex = 0x0000; curVertex < 0x0004; ++curVertex)
		{
			if (rectangles[curRectangle].vertex[curVertex] == j)
			{
				rectangles[curRectangle].vertex[curVertex] = i;
			}
		}
	}
	for (curTriangle = 0x0000; curTriangle < numTriangles; ++curTriangle)
	{
		for (curVertex = 0x0000; curVertex < 0x0003; ++curVertex)
		{
			if (triangles[curTriangle].vertex[curVertex] == j)
			{
				triangles[curTriangle].vertex[curVertex] = i;
			}
		}
	}
}

/*
 * Function that calculates the distance between two vertices.
 * Parameters:
 * * x1 = First colour's red value
 * * y1 = First colour's green value
 * * z1 = First colour's blue value
 * * x2 = Second colour's red value
 * * y2 = Second colour's green value
 * * z2 = Second colour's blue value
 * Returns the distance.
 */
static INTU16 distance16(INT16 x1, INT16 y1, INT16 z1,
                         INT16 x2, INT16 y2, INT16 z2)
{
	/* Variable Declarations */
	INTU16 diffx, diffy,
	       diffz;            /* Differences on each colour */
	INTU32 totaldiffsquared; /* Total of squared differences on every colour */
	INTU32 res, bit, tmp;    /* Variables used in square root calculation */
	
	/* Determines the differences on every colour */
	if (x1 > x2)
	{
		diffx = (INTU16) (x1 - x2);
	}
	else
	{
		diffx = (INTU16) (x2 - x1);
	}
	if (y1 > y2)
	{
		diffy = (INTU16) (y1 - y2);
	}
	else
	{
		diffy = (INTU16) (y2 - y1);
	}
	if (z1 > z2)
	{
		diffz = (INTU16) (z1 - z2);
	}
	else
	{
		diffz = (INTU16) (z2 - z1);
	}
	
	/* Squares the differences */
	diffx *= diffx;
	diffy *= diffy;
	diffz *= diffz;
	
	/* Adds the differences together */
	totaldiffsquared = (INTU32) diffx;
	totaldiffsquared += diffy;
	totaldiffsquared += diffz;
	
	/* Calculates the square root of this number */
	res = 0x00000000;
	bit = 0x40000000;
	while (bit > totaldiffsquared)
	{
		bit >>= 2;
	}
	while (bit != 0x00000000)
	{
		tmp = (res + bit);
		if (totaldiffsquared >= tmp)
		{
			totaldiffsquared -= tmp;
			res >>= 1;
			res += bit;
		}
		else
		{
			res >>= 1;
		}
		bit >>= 2;
	}
	
	/* Returns the resulting distance */
	diffx = (INTU16) res;
	return diffx;
}
