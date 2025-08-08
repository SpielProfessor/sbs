#include "utils.h"
#include "main.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

long getFileModTime(const char *path) {
  struct stat attr;
  if (stat(path, &attr) == 0) {
    return attr.st_mtime;
  } else {
    return -1;
  }
}

// pushes the given path `push` to `path`
// returns a malloc'd string that you must free.
// returns NULL if it couldn't handle the request.
char *pathPush(const char *path, const char *pushed) {
  char *returned = NULL;
  if (!(strlen(pushed) > 0)) {
    return strdup(path);
  }
  if ((pushed[0] == '/' && path[strlen(path) - 1] != '/') ||
      (pushed[0] != '/' && path[strlen(path) - 1] == '/')) {
    // one has a /
    returned = malloc(strlen(path) + strlen(pushed) + 1); // +1 for \0
    if (!returned)
      return NULL;
    strcpy(returned, path);
    strcat(returned, pushed);
  } else if (pushed[0] == '/' && path[strlen(path) - 1] == '/') {
    // both have a /
    returned = malloc(strlen(path) +
                      strlen(pushed)); // no +1 because we will remove one /,
                                       // of which the space will be used
    if (!returned)
      return NULL;
    strcpy(returned, path);
    returned[strlen(returned) - 1] = '\0';
    strcat(returned, pushed);
  } else {
    // neither has a /
    returned = malloc(strlen(path) + strlen(pushed) + 2); // +2, for \0 and /
    if (!returned)
      return NULL;
    strcpy(returned, path);
    strcat(returned, "/");
    strcat(returned, pushed);
  }
  return returned;
}
// push the given path 'push' to 'path', reallocating path.
// turns path into NULL if it didn't work.
void pathPushR(char **path, const char *pushed) {
  char *returned = pathPush(*path, pushed);
  free(*path);
  *path = returned;
}

// Normalize a path: remove redundant slashes, ./, ../ etc.
// Returns a malloc'd string you must free.
char *pathNormalize(const char *path) {
  if (!path)
    return NULL;

  size_t len = strlen(path);
  char **stack = malloc(sizeof(char *) * (len + 1)); // max components
  int top = 0;

  char *path_copy = strdup(path);
  if (!path_copy) {
    free(stack);
    return NULL;
  }

  char *token = strtok(path_copy, "/");
  int absolute = (path[0] == '/');

  while (token != NULL) {
    if (strcmp(token, "..") == 0) {
      if (top > 0 && strcmp(stack[top - 1], "..") != 0) {
        free(stack[--top]); // go up one directory
      } else if (!absolute) {
        // relative path can go above root
        stack[top++] = strdup("..");
      }
    } else if (strcmp(token, ".") == 0) {
      // skip current dir tokens
    } else if (*token != '\0') {
      stack[top++] = strdup(token);
    }
    token = strtok(NULL, "/");
  }

  size_t out_len = 0;
  if (absolute)
    out_len = 1; // for leading slash
  for (int i = 0; i < top; i++) {
    out_len += strlen(stack[i]) + 1; // +1 for '/'
  }
  if (out_len == 0)
    out_len = 1; // for "." or "/"

  char *normalized = malloc(out_len + 1);
  if (!normalized) {
    for (int i = 0; i < top; i++)
      free(stack[i]);
    free(stack);
    free(path_copy);
    return NULL;
  }

  char *p = normalized;
  if (absolute) {
    *p++ = '/';
  }
  for (int i = 0; i < top; i++) {
    size_t slen = strlen(stack[i]);
    memcpy(p, stack[i], slen);
    p += slen;
    if (i != top - 1) {
      *p++ = '/';
    }
    free(stack[i]);
  }
  if (top == 0) {
    if (absolute) {
      *p = '\0';
    } else {
      *p++ = '.';
      *p = '\0';
    }
  } else {
    *p = '\0';
  }

  free(stack);
  free(path_copy);

  return normalized;
}

// Create parent directories recursively for a path (must be normalized)
int createParentDirs(const char *path) {
  if (!path)
    return -1;

  char tmp[1024];
  strncpy(tmp, path, sizeof(tmp));
  tmp[sizeof(tmp) - 1] = '\0';

  char *last_slash = strrchr(tmp, '/');
  if (!last_slash)
    return 0; // no directory part to create

  *last_slash = '\0'; // truncate to directory

  char partial[1024] = {0};
  char *p = tmp;

  if (*p == '/') {
    strcat(partial, "/");
    p++;
  }

  char *token = strtok(p, "/");
  while (token) {
    if (strlen(partial) > 1)
      strcat(partial, "/");
    strcat(partial, token);

    struct stat st;
    if (stat(partial, &st) != 0) {
      if (mkdir(partial, 0755) != 0) {
        perror("mkdir");
        return -1;
      }
      printf("Created directory: %s\n", partial);
    } else if (!S_ISDIR(st.st_mode)) {
      fprintf(stderr, "%s exists but is not a directory\n", partial);
      return -1;
    }

    token = strtok(NULL, "/");
  }

  return 0;
}

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char **split_whitespace(const char *input) {
  if (!input)
    return NULL;

  size_t capacity = 8; // initial space for 8 strings
  size_t count = 0;
  char **result = malloc(capacity * sizeof(char *));
  if (!result)
    return NULL;

  const char *p = input;
  while (*p) {
    // skip leading whitespace
    while (*p && isspace((unsigned char)*p))
      p++;
    if (!*p)
      break;

    // find end of word
    const char *start = p;
    while (*p && !isspace((unsigned char)*p))
      p++;

    size_t len = p - start;
    char *word = malloc(len + 1);
    if (!word) {
      // free all previous on failure
      for (size_t i = 0; i < count; i++)
        free(result[i]);
      free(result);
      return NULL;
    }

    memcpy(word, start, len);
    word[len] = '\0';

    if (count >= capacity) {
      capacity *= 2;
      char **temp = realloc(result, capacity * sizeof(char *));
      if (!temp) {
        for (size_t i = 0; i < count; i++)
          free(result[i]);
        free(word);
        free(result);
        return NULL;
      }
      result = temp;
    }

    result[count++] = word;
  }

  // NULL-terminate the array
  result = realloc(result, (count + 1) * sizeof(char *));
  result[count] = NULL;
  return result;
}

#ifndef _WIN32
  // Linux/glibc has asprintf
  #define _GNU_SOURCE
  #include <stdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
int asprintf(char **strp, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int size = _vscprintf(fmt, args);
  va_end(args);
  if (size < 0) return -1;

  *strp = malloc(size + 1);
  if (!*strp) return -1;

  va_start(args, fmt);
  int r = vsnprintf(*strp, size + 1, fmt, args);
  va_end(args);

  return r;
}
#endif

