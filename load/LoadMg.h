/*
 * LoadMg.h
 * Author: AngelToms
 */

#ifndef LOADMG_H_
#define LOADMG_H_

#include <stdlib.h>
#include <stdio.h>

#include "Types.h"
#include "SysUtil.h"

class LoadMg {
public:
	LoadMg(const char* mgPath);
	virtual ~LoadMg();

	bool loadMgData();
	u4 getMergeDataSz();
	u1* getMergeData();

private:
	u4 soTextCrc(size_t off, size_t len);
	bool isEqMagic(const char* magic, size_t index);
	size_t getMergeDataOff(size_t* mixOff);

private:
	struct MemMapping mmap;
	int mfd;
	const char* mMgPath;
	u1* mBase;
	size_t mflen;

	u1* mData;
	u4 mDataSz;
};

#endif /* LOADMG_H_ */
