#ifndef SBS_UTILS
char *pathNormalize(const char *path);
int createParentDirs(const char *path);
char *pathPush(const char *path, const char *pushed);
void pathPushR(char **path, const char *pushed);
char **split_whitespace(const char *input);
long getFileModTime(const char *path);
#define SBS_UTILS
#endif
