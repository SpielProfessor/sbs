#include "dirdiscover.h"
#include "main.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int hasAllowedExtension(const char *filename, const char **allowedExts) {
  const char *dot = strrchr(filename, '.');
  if (!dot) {
    return 0; // no extension
  }

  for (int i = 0; allowedExts[i] != NULL; i++) {
    if (strcmp(dot, allowedExts[i]) == 0) {
      return 1;
    }
  }

  return 0;
}

// discover all source files
StrVec *srcDiscover(char *directory, const char **allowedExts) {
  StrVec *returned = malloc(sizeof(StrVec));
  returned->first = NULL;

  DIR *dir = opendir(directory);
  if (!dir) {
    printf("Couldn't open directory %s\n", directory);
    exit(-1);
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    // Construct full path
    char *full_path = malloc(strlen(directory) + strlen(entry->d_name) + 2);
    if (!full_path) {
      perror("malloc");
      exit(-2);
    }
    sprintf(full_path, "%s/%s", directory, entry->d_name);

    if (IS_DIR(full_path) /*entry->d_type == DT_DIR*/) {
      // Recurse into subdirectory
      returned = mergeVecs(returned, srcDiscover(full_path, allowedExts));
      free(full_path);
    } else {
      if (hasAllowedExtension(entry->d_name, allowedExts)) {
        strVecPush(returned, full_path); // freed via delStrVec
      } else {
        free(full_path);
      }
    }
  }

  closedir(dir);
  return returned;
}

// library for string vectors
StrVecElement *strVecGetLast(StrVec *vector) {
  StrVecElement *entry = vector->first;
  while (entry != NULL) {
    if (entry->next == NULL) {
      break;
    }
    entry = entry->next;
  }
  return entry;
}
int strVecLength(StrVec *vector) {
  StrVecElement *entry = vector->first;
  int out = 0;
  while (entry != NULL) {
    if (entry->next == NULL) {
      break;
    }
    entry = entry->next;
    out++;
  }
  return out;
}
void delStrVec(StrVec *vector) {
  // clear elements
  StrVecElement *entry = vector->first;
  while (entry != NULL) {
    StrVecElement *nentry = entry->next;
    free(entry->content);
    free(entry);
    entry = nentry;
  }
  // delete vector
  free(vector);
}

void strVecPush(StrVec *vector, char *element) {
  StrVecElement *last = strVecGetLast(vector);
  if (last) {
    last->next = malloc(sizeof(StrVecElement));
    last->next->content = element;
    last->next->next = NULL;
  } else {
    if (vector->first == NULL) {
      vector->first = malloc(sizeof(StrVecElement));
      vector->first->content = element;
      vector->first->next = NULL;
    } else {
      puts("LIBSTRVEC: ERROR: couldn't find last element");
      exit(-2);
    }
  }
}

StrVec *mergeVecs(StrVec *a, StrVec *b) {
  if (!a->first) {
    free(a);
    return b;
  }
  if (!b->first) {
    free(b);
    return a;
  }
  strVecGetLast(a)->next = b->first;
  free(b); // only free container, not contents
  return a;
}

char *strcatr(const char *start, const char *end) {
  size_t len1 = strlen(start);
  size_t len2 = strlen(end);
  char *returned = malloc(len1 + len2 + 1); // +1 for '\0'
  if (!returned) {
    return NULL;
  }
  strcpy(returned, start);
  strcat(returned, end);
  return returned;
}

StrVec *strVecNew() {
  StrVec *returned = malloc(sizeof(StrVec));
  returned->first = NULL;
  return returned;
}
