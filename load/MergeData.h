/* 
 * File:   MergeData.h
 * Author: Angeltoms
 *
 */

#ifndef MERGEDATA_H
#define	MERGEDATA_H

#include "QupLog.h"
#include "Types.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PADDING_SEG_DATA_SIZE 128
#define PADDING_TAIL_DATA_SIZE 200

typedef struct {
    char magic[8]; // 混淆魔术
    size_t addr[2]; //混淆后merge data 结构偏移
    size_t textOff; //text节偏移
    size_t textlen; //text节大小
} MergeDataIndex;

typedef struct {
    u4 mgDataCrc; // 该结构crc校验,2-8
    u4 mgStructSz; // 该结构大小， 虽然可能用不到
    u4 textCrc; // 原始so的text节crc
    u4 mgDataSz; // 合并的bin的大小
    u4 fsz; // 第一块的大小
    u4 foff; // 第一块相对结构头的偏移
    u4 soff; // 第二块相对头的偏移
    u4 binCrc; // 合并bin的crc
} MergeData;

#ifdef	__cplusplus
extern "C" {
#endif

    const char MERGE_BIN_MAGIC [] = {'$', '0', '^'};


#ifdef	__cplusplus
}
#endif

#endif	/* MERGEDATA_H */

