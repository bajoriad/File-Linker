/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry
{
	char label[7];
	char location;
	int offset;
};

struct RelocationTableEntry
{
	int offset;
	char inst[7];
	char label[7];
	int file;
};

struct FileData
{
	int textSize;
	int dataSize;
	int symbolTableSize;
	int relocationTableSize;
	int textStartingLine; // in final executable
	int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles
{
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
	int textSize;
	int dataSize;
	int symTableSize;
	int relocTableSize;
};

int totaltextsize = 0;
int totaldatasize = 0;
char stack[] = "Stack";
char defineddata = 'D';
char definedtext = 'T';
char undefined = 'U';
char lw[] = "lw";
char sw[] = "sw";
char fill[] = ".fill";

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	int i, j;

	if (argc <= 2)
	{
		printf("error: usage: %s <obj file> ... <output-exe-file>\n", argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL)
	{
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData files[MAXFILES];

	// read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; i++)
	{
		inFileString = argv[i + 1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL)
		{
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		int sizeText, sizeData, sizeSymbol, sizeReloc;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d", &sizeText, &sizeData, &sizeSymbol, &sizeReloc);

		files[i].textSize = sizeText;
		files[i].dataSize = sizeData;
		files[i].symbolTableSize = sizeSymbol;
		files[i].relocationTableSize = sizeReloc;

		// read in text section
		int instr;
		for (j = 0; j < sizeText; j++)
		{
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = atoi(line);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < sizeData; j++)
		{
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = atoi(line);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		int addr;
		for (j = 0; j < sizeSymbol; j++)
		{
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d", label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < sizeReloc; j++)
		{
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s", &addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file = i;
		}
		fclose(inFilePtr);
	} // end reading files

	// counting the total text size and data size of all the files
	for (int i = 0; i < argc - 2; ++i)
	{
		totaltextsize = totaltextsize + files[i].textSize;
		totaldatasize = totaldatasize + files[i].dataSize;
	}

	// error checking for a defined stack label
	for (int j = 0; j < argc - 2; ++j)
	{
		for (int u = 0; u < files[j].symbolTableSize; ++u)
		{
			if (strcmp(files[j].symbolTable[u].label, stack) == 0)
			{
				// strcpy(location, files[j].symbolTable[u].location);
				if ((files[j].symbolTable[u].location == definedtext) || (files[j].symbolTable[u].location == defineddata))
				{
					printf("defined Stack label");
					exit(1);
				}
			}
		}
	}

	// error checking for duplicate defined global labels
	for (int j = 0; j < argc - 2; ++j)
	{
		for (int u = 0; u < files[j].symbolTableSize; ++u)
		{
			if ((files[j].symbolTable[u].location == defineddata) || (files[j].symbolTable[u].location == definedtext))
			{
				char labelincheck[7] = " ";
				strcpy(labelincheck, files[j].symbolTable[u].label);

				for (int j1 = 0; j1 < argc - 2; ++j1)
				{
					if (j1 != j)
					{
						for (int u1 = 0; u1 < files[j1].symbolTableSize; ++u1)
						{
							if (strcmp(files[j1].symbolTable[u1].label, labelincheck) == 0)
							{
								if ((files[j1].symbolTable[u1].location == defineddata) ||
									(files[j1].symbolTable[u1].location == definedtext))
								{
									printf("global label defined more than once ");
									exit(1);
								}
							}
						}
					}
				}
			}
		}
	}

	// calculating textstarting line and datastarting line for each file in file linked files
	for (int i = 0; i < argc - 2; ++i)
	{
		files[i].textStartingLine = 0;
		files[i].dataStartingLine = totaltextsize;
		for (int k = 0; k < i; ++k)
		{
			files[i].textStartingLine = files[i].textStartingLine + files[k].textSize;
			files[i].dataStartingLine = files[i].dataStartingLine + files[k].dataSize;
		}
	}

	// running the main program now
	//  going through relocation table if global going through symbole table
	for (int i = 0; i < argc - 2; ++i)
	{
		for (int i1 = 0; i1 < files[i].relocationTableSize; ++i1)
		{
			if (strcmp(files[i].relocTable[i1].inst, lw) == 0 || strcmp(files[i].relocTable[i1].inst, sw) == 0)
			{
				if (strcmp(files[i].relocTable[i1].label, stack) == 0)
				{
					files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + totaltextsize + totaldatasize;
				}
				else if (files[i].relocTable[i1].label[0] >= 'a' && files[i].relocTable[i1].label[0] <= 'z')
				{
					int offset = files[i].text[files[i].relocTable[i1].offset] & 0x0000FFFF;
					if (offset < files[i].textSize)
					{
						files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[i].textStartingLine;
					}
					else
					{
						files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
					}
				}
				else
				{
					char labeltocheck[7] = " ";
					strcpy(labeltocheck, files[i].relocTable[i1].label);
					// int instruction = files[i].text[files[i].relocTable[i1].offset] & 0xFFFF0000;
					for (int j = 0; j < files[i].symbolTableSize; ++j)
					{
						if (strcmp(labeltocheck, files[i].symbolTable[j].label) == 0)
						{
							if (files[i].symbolTable[j].location == definedtext)
							{
								// int offset = files[i].symbolTable[j].offset;
								files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// if (offset < files[i].textSize)
								// {
								// 	files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// }
								// else
								// {
								// 	files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
								// }
								// instruction = instruction + files[i].symbolTable[j].offset + files[i].textStartingLine;
								// files[i].text[files[i].relocTable[i1].offset] = instruction;
							}
							else if (files[i].symbolTable[j].location == defineddata)
							{
								files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[i].dataStartingLine - files[i].textSize;
								// int offset = files[i].data[files[i].symbolTable[j].offset] & 0x0000FFFF;
								// if (offset < files[i].textSize)
								// {
								// 	files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// }
								// else
								// {
								// 	files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
								// }
								// instruction = instruction + files[i].symbolTable[j].offset + files[i].dataStartingLine;
								// files[i].text[files[i].relocTable[i1].offset] = instruction;
							}
							else if (files[i].symbolTable[j].location == undefined)
							{
								int globallabeldefinedornot = 0;
								for (int k = 0; k < argc - 2; ++k)
								{
									if (i != k)
									{
										for (int k1 = 0; k1 < files[k].symbolTableSize; ++k1)
										{
											if (strcmp(files[k].symbolTable[k1].label, labeltocheck) == 0)
											{
												if (files[k].symbolTable[k1].location == defineddata)
												{
													files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[k].dataStartingLine + files[k].symbolTable[k1].offset;
													// instruction = instruction + files[k].symbolTable[k1].offset + files[k].dataStartingLine;
													// files[i].text[files[i].relocTable[i1].offset] = instruction;
													globallabeldefinedornot = 1;
												}
												else if (files[k].symbolTable[k1].location == definedtext)
												{
													files[i].text[files[i].relocTable[i1].offset] = files[i].text[files[i].relocTable[i1].offset] + files[k].textStartingLine + files[k].symbolTable[k1].offset;
													// instruction = instruction + files[k].symbolTable[k1].offset + files[k].textStartingLine;
													// files[i].text[files[i].relocTable[i1].offset] = instruction;
													globallabeldefinedornot = 1;
												}
											}
										}
									}
								}
								if (globallabeldefinedornot == 0)
								{
									printf("undefined global address");
									exit(1);
								}
							}
						}
					}
				}
			}
			else if (strcmp(files[i].relocTable[i1].inst, fill) == 0)
			{
				if (strcmp(files[i].relocTable[i1].label, stack) == 0)
				{
					files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + totaltextsize + totaldatasize;
				}
				else if (files[i].relocTable[i1].label[0] >= 'a' && files[i].relocTable[i1].label[0] <= 'z')
				{
					int offset = files[i].data[files[i].relocTable[i1].offset];
					if (offset < files[i].textSize)
					{
						files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[i].textStartingLine;
					}
					else
					{
						files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
					}
				}
				else
				{
					char labeltocheck[7] = " ";
					// int instruction = files[i].text[files[i].relocTable[i1].offset] & 0xFFFF0000;
					strcpy(labeltocheck, files[i].relocTable[i1].label);
					for (int j = 0; j < files[i].symbolTableSize; ++j)
					{
						if (strcmp(labeltocheck, files[i].symbolTable[j].label) == 0)
						{
							if (files[i].symbolTable[j].location == definedtext)
							{
								files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// int offset = files[i].text[files[i].symbolTable[j].offset] & 0x0000FFFF;
								// if (offset < files[i].textSize)
								// {
								// 	files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// }
								// else
								// {
								// 	files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
								// }
								// instruction = instruction + files[i].symbolTable[j].offset + files[i].textStartingLine;
								// files[i].text[files[i].relocTable[i1].offset] = instruction;
							}
							else if (files[i].symbolTable[j].location == defineddata)
							{
								files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
								// int offset = files[i].data[files[i].symbolTable[j].offset] & 0x0000FFFF;
								// if (offset < files[i].textSize)
								// {
								// 	files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[i].textStartingLine;
								// }
								// else
								// {
								// 	files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] - files[i].textSize + files[i].dataStartingLine;
								// }
								// instruction = instruction + files[i].symbolTable[j].offset + files[i].dataStartingLine;
								// files[i].text[files[i].relocTable[i1].offset] = instruction;
							}
							else if (files[i].symbolTable[j].location == undefined)
							{
								int globallabeldefinedornot = 0;
								for (int k = 0; k < argc - 2; ++k)
								{
									if (i != k)
									{
										for (int k1 = 0; k1 < files[k].symbolTableSize; ++k1)
										{
											if (strcmp(files[k].symbolTable[k1].label, labeltocheck) == 0)
											{
												if (files[k].symbolTable[k1].location == defineddata)
												{
													files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[k].dataStartingLine + files[k].symbolTable[k1].offset;
													// files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[k].dataStartingLine + files[k].symbolTable[k1].offset;
													// instruction = instruction + files[k].symbolTable[k1].offset + files[k].dataStartingLine;
													// files[i].text[files[i].relocTable[i1].offset] = instruction;
													globallabeldefinedornot = 1;
												}
												else if (files[k].symbolTable[k1].location == definedtext)
												{
													files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[k].textStartingLine + files[k].symbolTable[k1].offset;
													// files[i].data[files[i].relocTable[i1].offset] = files[i].data[files[i].relocTable[i1].offset] + files[k].textStartingLine + files[k].symbolTable[k1].offset;
													// instruction = instruction + files[k].symbolTable[k1].offset + files[k].textStartingLine;
													// files[i].text[files[i].relocTable[i1].offset] = instruction;
													globallabeldefinedornot = 1;
												}
											}
										}
									}
								}
								if (globallabeldefinedornot == 0)
								{
									printf("undefined global address");
									exit(1);
								}
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < argc - 2; i++)
	{
		for (int j = 0; j < files[i].textSize; ++j)
		{
			fprintf(outFilePtr, "%d\n", files[i].text[j]);
		}
	}

	for (int i = 0; i < argc - 2; i++)
	{
		for (int j = 0; j < files[i].dataSize; ++j)
		{
			fprintf(outFilePtr, "%d\n", files[i].data[j]);
		}
	}

} // main
