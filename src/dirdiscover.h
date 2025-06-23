#ifndef SBS_DIRDISCOVER
typedef struct StrVecElement {
  struct StrVecElement *next;
  char *content;
} StrVecElement;
typedef struct StrVec {
  StrVecElement *first;
} StrVec;
StrVec *srcDiscover(char *directory, const char **allowedExts);
void delStrVec(StrVec *vector);
void strVecPush(StrVec *vector, char *element);
StrVec *mergeVecs(StrVec *a, StrVec *b);
// return a new, malloc'd string concatenating start and end
char *strcatr(const char *start, const char *end);
int strVecLength(StrVec *vector);
// library for string vectors
StrVecElement *strVecGetLast(StrVec *vector);
int strVecLength(StrVec *vector);
void delStrVec(StrVec *vector);
void strVecPush(StrVec *vector, char *element);
StrVec *mergeVecs(StrVec *a, StrVec *b);
StrVec *strVecNew();
#define SBS_DIRDISCOVER
#endif // SBS_DIRDISCOVER
