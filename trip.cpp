//============================================================================
// Name        : trip.cpp
// Author      : angeltoms
// Version     :
// 这个文件做什么用的，测试？？？我也忘了
//============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <vector>
#include <string>

#include "LoadMg.h"

int main() {

	LoadMg loadmg = LoadMg("d:/tmp_test/libpadtest.so.1");

	if(!loadmg.loadMgData()) {
		printf("[-] load merge data fail");
		return -1;
	}

    int fod = open("d:/tmp_test/mergedata", O_CREAT | O_RDWR, 0755);
    if (fod < 0) {
        QUP_LOGI("[-] create file %s fail : %s", "d:/tmp_test/mergedata", strerror(errno));
        return -1;
    }

    write(fod, loadmg.getMergeData(), loadmg.getMergeDataSz());
    fsync(fod);
    close(fod);

	return 0;
}
