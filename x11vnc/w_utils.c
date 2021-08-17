

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <math.h>
#include <inttypes.h>

#include "w_utils.h"


#define DIM(x) (sizeof(x)/sizeof(*(x)))

static const char *sizes[] = {
		"EB", "PB", "TB", "GB", "MB", "KB", "B"
};
static const uint64_t exbibytes =
		1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL;



int isDigit(char* str){

	int i;
	int end;
	end = strlen(str);
	for(i=0; i<end; i++){
		if(!isdigit(*str)){
			return 0;
		}
		str++;
	}
	return 1;
}

int isRegexMatches(const char* regexString, const char* text){

	regex_t regex;
	int reti;

	reti = regcomp(&regex, regexString, REG_EXTENDED);
	if(reti){
		printf(stderr, "Could not compile regex\n");
	    return 0;
	}

	/* Execute regular expression */
	reti = regexec(&regex, text, 0, NULL, 0);

	if (!reti) {
		regfree(&regex);
	    return 1;
	}
	else if (reti == REG_NOMATCH) {
		regfree(&regex);
		return 0;
	}
	else {
		char msgbuf[100];
	    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
	    printf(stderr, "Regex match failed: %s\n", msgbuf);
	    regfree(&regex);
	    return 0;
	}

}

int startsWith(const char *pre, const char *str){

    size_t lenpre = strlen(pre),
           lenstr = strlen(str);

    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

char* replaceAll(char *s, const char *olds, const char *news) {

	char *result, *sr;
	size_t i, count = 0;
	size_t oldlen = strlen(olds);
	if (oldlen < 1)
		return s;
	size_t newlen = strlen(news);

	if (newlen != oldlen) {
		for (i = 0; s[i] != '\0';) {
			if (memcmp(&s[i], olds, oldlen) == 0)
				count++, i += oldlen;
			else
				i++;
		}
	} else
		i = strlen(s);

	result = (char *) malloc(i + 1 + count * (newlen - oldlen));
	if (result == NULL)
		return NULL;

	sr = result;
	while (*s) {
		if (memcmp(s, olds, oldlen) == 0) {
			memcpy(sr, news, newlen);
			sr += newlen;
			s += oldlen;
		} else
			*sr++ = *s++;
	}
	*sr = '\0';

	return result;
}



char* maxLengthString(const char* str, size_t maxLength){

	size_t strLength = strlen(str);

	size_t r;
	r = maxLength - strLength;

	wLog("maxlength[%d] strlength[%d] remainLength[%d]\n", maxLength, strLength, r);

	void* v;


	if(strLength == maxLength){
		return str;
	}
	else{

		v = malloc(maxLength);

		if(strLength > maxLength){
			memcpy(v, str, maxLength);
		}
		else{
			memcpy(v, str, strLength);

			int pos;
			int i;
			char d = 0x0;
			pos = strLength;

			for(i=0; i<r; i++){
				pos++;
				memcpy(v+pos, &d, 1);
			}
		}

		return v;
	}
}

void millisToTimeFormat(struct timespec start, struct timespec end, char* runningTime){

	uint64_t duration_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	uint64_t duration = duration_us / 1000;

	uint32_t seconds = (int)(duration/1000)%60;
	uint32_t minutes = (int)((duration/(1000*60))%60);
	uint32_t hours   = (int)((duration/(1000*60*60))%24);

	sprintf(runningTime, "%02d:%02d:%02d", hours, minutes, seconds);
}

void getExecPath(char *dest){

	char path[PATH_MAX];
	memset(dest,0,sizeof(dest));
	struct stat info;
	pid_t pid = getpid();
	sprintf(path, "/proc/%d/exe", pid);
	if(readlink(path, dest, PATH_MAX) == -1)
		perror("readlink");
}

void getCommandOutput(const char* command, char* output, size_t size){
	FILE *f = popen(command, "r");
	fgets(output, size, f);
	pclose(f);
}


char* getFileSizeFormat(uint64_t size){

    char     *result = (char *) malloc(sizeof(char) * 20);
    uint64_t  multiplier = exbibytes;
    int i;

    for (i = 0; i < DIM(sizes); i++, multiplier /= 1024){

        if (size < multiplier)
            continue;
        if (size % multiplier == 0)
            sprintf(result, "%" PRIu64 " %s", size / multiplier, sizes[i]);
        else
            sprintf(result, "%.1f %s", (float) size / multiplier, sizes[i]);
        return result;
    }
    strcpy(result, "0");
    return result;
}


