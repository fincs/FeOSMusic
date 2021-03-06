#include "FeOSMusic.h"

#define FILTER_PATH ("/data/FeOSMusic/filters")

char* nameBlock;
int* nameList;
int blockSize;
int numFilters;
int selected = -1;
FILTER filter;
int isEnabled;

void loadFilters(void)
{
	DIR *pdir;
	struct dirent *pent;
	if((pdir=opendir(FILTER_PATH))) {
		while ((pent=readdir(pdir))!=NULL) {
			if(strstr(pent->d_name, ".fx2")) {
				void* tmp = realloc(nameBlock, blockSize+strlen(pent->d_name)+1);
				if(tmp) {
					nameBlock = tmp;
					char* dst = &nameBlock[blockSize];
					strcpy(dst, pent->d_name);
					blockSize+=strlen(pent->d_name)+1;
					numFilters++;
					tmp = realloc(nameList, sizeof(int)*numFilters);
					if(tmp) {
						nameList = tmp;
						nameList[numFilters-1] = dst - nameBlock;
					}
				}
			}
		}
	}
	closedir(pdir);
}

void unloadFilters(void)
{
	free(nameBlock);
	free(nameList);
	/* Don't leak filters */
	if(selected >= 0)
		unloadFilter(&filter);
	numFilters = 0;
}

void printFilterInfo(void)
{
	char* string = "Filtering disabled";
	if(selected>=0)
		string = nameBlock + nameList[selected];
	consoleClearLine(23);
	setConsoleCoo(0, 23);
	print("%s\n", string);
}

void updateFilters(GUI_STATE stat)
{
	switch(stat) {
	case GUI_STREAMING:
		if(isEnabled) {
			if(keysPres & KEY_SELECT) {
				int prev = selected;
				selected++;
				CYCLE(selected, -1, (numFilters-1));
				/* unload filter if it became useless */
				if(prev!=selected) {
					if(prev>=0)
						unloadFilter(&filter);
				}

				/* Load a new filter */
				if(selected>=0 && selected!=prev) {
					char buf[256];
					snprintf(buf, sizeof(buf), "%s/%s", FILTER_PATH, nameBlock + nameList[selected]);
					if(!loadFilter(&filter, buf))
						selected = -1;
					setListedDir();
				}
				printFilterInfo();
			}
		}
		break;
	case GUI_BROWSING:
		if(keysPres & KEY_TOUCH) {
			if(TouchinArea(256-FL_ICONSZ, 0, 256, FL_ICONSZ)) {
				isEnabled++;
				CYCLE(isEnabled, 0, 1);
				setFrame(iconFrames[FILTER_FRAMES+isEnabled], false, FILTER_ICON, SUB_SCREEN);
				if(isEnabled)
					enableFiltering(SOUNDBUF_0x6020000, false);
				else
					disableFiltering();
			}
		}
		break;
	}
}
