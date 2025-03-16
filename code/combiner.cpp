/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#include <stdio.h>

#include "typedef.h"
#include "csv_parser.h"
#include "combiner.h"


void CopyTheStringInCombiner(
        char * CombineBuffer,
        char * DataBuffer,
        u16 Size) {
    u16 Index = 0;
    for (u16 x = 0; x < 1024; x++) {
        if (CombineBuffer[x] == '\0') {
            CombineBuffer[x] = ' ';
            for (u16 i = 0; i < Size; i++) {
                if (DataBuffer[i] == '\0') {
                    Index = i+1;
                    break;
                }
                CombineBuffer[x+i+1] = DataBuffer[i];
            }
            CombineBuffer[x+Index] = '\0';
            break;
        }
    }
}

void ConvertIntToTheString(
        char * CombineBuffer,
        u32 Value) {
    char buffer[10];
    u8 NulIndex = 0;
    u32 Quotient;
    u32 Remainder;
    u8 IfNotZero = 1;

    if (Value == 0) {
        IfNotZero = 0;
    } else {
        for (u8 x = 0; x < 10; x++) {
            if (Value == 0) {
                NulIndex = x;
                buffer[x] = '\0';
                break;
            }
            Quotient = Value / 10;
            Remainder = Value - (Quotient * 10);
            buffer[x] = (char) Remainder + '0';
            Value /= 10;
        }
    }

    for (u16 x = 0; x < 1024; x++) {
        if (CombineBuffer[x] == '\0') {
            CombineBuffer[x] = ' ';
            if (!IfNotZero) {
                CombineBuffer[x+1] = '0';
                CombineBuffer[x+2] = '\0';
                break;
            } else {
                for (u16 i = 0; i < NulIndex; i++) {
                    CombineBuffer[x + i + 1] = buffer[NulIndex - i - 1];
                }
            }
            CombineBuffer[x + NulIndex + 1] = '\0';
            break;
        }
    }
}

void GameCombine(
        combined * CombinedData,
        struct games * GameTable) {
    u32 SizeArray = GameTable -> uSizeArray;
    for (u32 x = 0; x < 10; x++) {
        CombinedData[x].sData[0] = '\0';
        CombinedData[x].uID = GameTable -> Data[x].uID;

        CopyTheStringInCombiner(
                CombinedData[x].sData,
                GameTable -> Data[x].sName,
                sizeof(GameTable -> Data[x].sName));

        CopyTheStringInCombiner(
                CombinedData[x].sData,
                GameTable -> Data[x].sReleaseDate,
                sizeof(GameTable -> Data[x].sReleaseDate));

        ConvertIntToTheString(
                CombinedData[x].sData,
                GameTable -> Data[x].ubIsFree);

        CopyTheStringInCombiner(
                CombinedData[x].sData,
                GameTable -> Data[x].sType,
                sizeof(GameTable -> Data[x].sType));
    }
}

/*void CategoryCombine(
        combined * CombinedData,
        struct category * GameTable) {
    u32 SizeArray = GameTable -> uSizeArray;
    for (u32 x = 0; x < 10; x++) {
    }
}*/
