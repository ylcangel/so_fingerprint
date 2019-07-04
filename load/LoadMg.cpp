/*
 * LoadMg.cpp
 * Author: AngelToms
 */

#include "LoadMg.h"

#include "QupLog.h"
#include "Debug.h"
#include "crc32.h"
#include "MergeData.h"
#include "SysUtil.h"
#include "ExElf.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

LoadMg::LoadMg(const char* mgPath) :
		mfd(0), mMgPath(mgPath), mBase(NULL), mflen(0), mData(NULL), mDataSz(0) {
	QUP_LOGI("[*] load mg path = %s", mgPath);

	mfd = open(mMgPath, O_RDONLY);

	if (mfd < 0) {
		QUP_LOGI("[-] load mg open %s fail, reason : %s", mMgPath,
				strerror(errno));
		exit(-1);
	}

	memset(&mmap, 0, sizeof(mmap));
	if (sysMapFileInShmemWritableReadOnly(mfd, &mmap) < 0) {
		QUP_LOGI("[-] mmap file %d in shem fail", mfd);
		exit(-1);
	}

	mBase = (u1*) mmap.addr;
	mflen = mmap.length;
	QUP_LOGI("[*] file %s len = %u", mMgPath, mflen);
}

LoadMg::~LoadMg() {
	sysReleaseShmem(&mmap);

	if (mfd)
		close(mfd);

	delete mData;
	mData = NULL;
}

bool LoadMg::isEqMagic(const char* magic, size_t index) {
	QUP_LOGI("[*] magic index = %d", index);
	switch (index) {
	case 0:
		if (memcmp(magic, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 1:
		if (memcmp(magic + 1, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 2:
		if (memcmp(magic + 2, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 3:
		if (memcmp(magic + 3, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 4:
		if (memcmp(magic + 4, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 5:
		if (memcmp(magic + 5, MERGE_BIN_MAGIC, 3) == 0)
			return true;
		break;
	case 6:
		if (memcmp(magic + 6, MERGE_BIN_MAGIC, 2) == 0
				&& memcmp(magic, MERGE_BIN_MAGIC + 2, 1) == 0)
			return true;
		break;
	case 7:
		if (memcmp(magic + 7, MERGE_BIN_MAGIC, 1) == 0
				&& memcmp(magic, MERGE_BIN_MAGIC + 1, 2) == 0)
			return true;
		break;
	default:
		return false;
	}

	return false;
}

u4 LoadMg::soTextCrc(size_t off, size_t len) {
	u1* textBase = mBase + off;
	u4 crc = compute_crc32(0, textBase, len);

	QUP_LOGI("[*] So .text section crc32 = 0x%.8x", crc);

	return crc;
}

size_t LoadMg::getMergeDataOff(size_t* mixOff) {
	u1* p = (u1*) mixOff;
	size_t mgOff = 0;
	size_t* mgOffP = &mgOff;
	u1* offP = (u1*) mgOffP;

	//都以小端处理 TODO;
	if (sizeof(size_t) == 4) { //32bits
		memcpy(offP, p, 1);
		memcpy(offP + 1, p + 4, 1);
		memcpy(offP + 2, p + 6, 1);
		memcpy(offP + 3, p + 2, 1);
	} else {
		memcpy(offP, p, 1);
		memcpy(offP + 1, p + 8, 1);
		memcpy(offP + 2, p + 12, 1);
		memcpy(offP + 3, p + 10, 1);
		memcpy(offP + 4, p + 4, 1);
		memcpy(offP + 5, p + 14, 1);
		memcpy(offP + 6, p + 2, 1);
		memcpy(offP + 7, p + 6, 1);
	}
	QUP_LOGI("[*] Merge data off = 0x%.8x", mgOff);
	return mgOff;
}

bool LoadMg::loadMgData() {
	QUP_LOGI("[*] Load mg - begin to load merge data ...");

	size_t tailOff = mflen - PADDING_TAIL_DATA_SIZE;
	u1* p = mBase + tailOff;
	int m = mflen % 150;
	QUP_LOGI("[*] mg tail mod = %d", m);
	p += m;
	MergeDataIndex* mgDataIndex = (MergeDataIndex*) p;

	if (!isEqMagic(mgDataIndex->magic, mflen % 8)) {
		QUP_LOGI("[*] magic was not correct");
		return false;
	}

	Elf_Off mgdataOff = getMergeDataOff(mgDataIndex->addr);
	MergeData* mergeData = (MergeData*) (mBase + mgdataOff);

	QUP_LOGI("[*] Section text off = 0x%.8x, len = 0x%.8x",
			mgDataIndex->textOff, mgDataIndex->textlen);

	u4 textCrc = soTextCrc(mgDataIndex->textOff, mgDataIndex->textlen);

	if (mergeData->textCrc != textCrc) {
		QUP_LOGI("[*] elf text crc was not match");
		return false;
	}

	u4 mgCrc = compute_crc32(0, ((u1*) mergeData + sizeof(u4)),
			mergeData->mgStructSz - sizeof(u4));
	QUP_LOGI("[*] merge data struct crc = 0x%.8x, mg crc = 0x%.8x",
			mergeData->mgDataCrc, mgCrc);
	if (mgCrc != mergeData->mgDataCrc) {
		QUP_LOGI("[-] merge data struct was not correct");
		return false;
	}

	p = mBase + mgdataOff;
	mDataSz = mergeData->mgDataSz;
	QUP_LOGI("[*] merge data size = %u", mDataSz);

	mData = new u1[mDataSz];
	if (!mData) {
		QUP_LOGI("[-] alloc merge data buf fail");
		return false;
	}

	memset(mData, 0, mDataSz);
	//first seg
	memcpy(mData, p + mergeData->foff, mergeData->fsz);
	//second seg
	if (mergeData->fsz < mDataSz)
		memcpy(mData + mergeData->fsz, p + mergeData->soff,
				(mDataSz - mergeData->fsz));

	u4 mgdataCrc = compute_crc32(0, mData, mDataSz);
	QUP_LOGI("[*] merge data crc = 0x%.8x, save crc = 0x%.8x", mgdataCrc,
			mergeData->binCrc);
	if (mgdataCrc != mergeData->binCrc) {
		QUP_LOGI("[-] merge data crc was not correct");
		return false;
	}

	QUP_LOGI("[*] Load mg - end to load merge data ...");
	return true;

}

u4 LoadMg::getMergeDataSz() {
	return mDataSz;
}
u1* LoadMg::getMergeData() {
	return mData;
}

