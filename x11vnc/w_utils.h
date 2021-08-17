
#ifndef W_UTILS_H_
#define W_UTILS_H_


#define REGEX_IPADDRESS "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"
#define REGEX_DOMAIN "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$"

int isDigit(char* str);
int isRegexMatches(const char* regexString, const char* text);
int startsWith(const char *pre, const char *str);
char* replaceAll(char *s, const char *olds, const char *news);
char* maxLengthString(const char* str, size_t maxLength);
void millisToTimeFormat(struct timespec start, struct timespec end, char* runningTime);
void getExecPath(char *dest);
void getCommandOutput(const char* command, char* output, size_t size);
char* getFileSizeFormat(uint64_t size);

#endif
