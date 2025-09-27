/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */

#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;
typedef uint64_t u64;

u32 Comments;
u32 Code;
u32 Blank;

u32 TotalComments;
u32 TotalCode;
u32 TotalBlank;


struct stat
FileSize(char *pfileName)
{
    struct stat Stat;
    stat(pfileName, &Stat);
    return Stat;
}

u32
BigComment(char* File, u64 Size, u32 Index)
{
    u32 ReturnValue = 0;

    for (u32 x = Index; x < Size; x++)
    {
        if (File[x] == '*' && File[x+1] == '/')
        {
            ReturnValue = x+4;
            break;
        }
        
        if (File[x] == '\n')
        {
            if (File[x+1] != '\n')
            {
                Comments++;
            }

            if (File[x+1] == '\n')
            {
                Blank++;
                continue;
            }
        }
    }

    Comments++;
    return ReturnValue;
}

u32
SmallComments(char * File)
{
    u32 Index = 0;

    while (File[Index] != '\n')
    {
        Index++;
    }

    Comments++;
    return Index;
}

void
FileLineCounter(char * FileName)
{
    FILE *FilePtr = fopen(FileName, "rb");

    if (FilePtr == NULL)
    {
        perror("File is not being read");
    }

    struct stat Stat = FileSize(FileName);
    char * FileBuffer = (char*) malloc(Stat.st_size);

    fread(FileBuffer, sizeof(char), Stat.st_size, FilePtr);

    for (u32 x = 0; x < Stat.st_size; x++)
    {
        if (FileBuffer[x] == '/')
        {
            if (FileBuffer[x + 1] == '*')
            {
                x = BigComment(FileBuffer, Stat.st_size, x);
            }

            if (FileBuffer[x+1] == '/')
            {
                x += SmallComments(&FileBuffer[x]);
            }
        }

        if (FileBuffer[x] == '\n')
        {
            if (FileBuffer[x+1] != '\n')
            {
                Code++;
            }

            if (FileBuffer[x+1] == 13)
            {
                if (FileBuffer[x+2] == '\n')
                {
                    Blank++;
                    x+=2;
                }
            }
        }
    }
}

int main(int ArgC, char ** Argv)
{
    u32 Total;
    printf("\n\n%-50s %*s %*s %*s %*s\n", "File Name", 10, "Code", 10, "Blank", 10,
            "Comments", 10, "Total");

    for (u32 x = 1; x < ArgC; x++)
    {
        FileLineCounter(Argv[x]);
        Total = Code + Blank + Comments;
        printf("%-50s %10u %10u %10u %10u\n", Argv[x], Code, Blank,
                Comments, Total);

        TotalCode += Code;
        TotalComments += Comments;
        TotalBlank += Blank;

        Code = 0;
        Comments = 0;
        Blank = 0;
    }

    Total = TotalCode + TotalComments + TotalBlank;
    printf("\n%-50s %10u %10u %10u %10u\n\n\n", "Total lines of Code", TotalCode,
            TotalBlank, TotalComments, Total);

    return 0;
}
