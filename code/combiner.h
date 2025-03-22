/* ==================================================================
   $File: $
   $Data: $
   $Revision: $
   $Creator: Abhijit Rai $
   $Notice: (C) Copyright 2025 by Abhijit Rai. All Rights Reserved. $
   ================================================================= */


#ifndef W__CODE_COMBINER_H_
#define W__CODE_COMBINER_H_

#include "typedef.h"
#include "csv_parser.h"


struct combined {
    u32 uID;
    char sData[1024];
    u32 uSizeArray;
};

void GameCombine(
        struct combined * CombinedData,
        struct games * GameTable);

#endif // !W__CODE_COMBINER_H_
