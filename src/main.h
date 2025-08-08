#include "confparse.h"
#include <stdbool.h>
#ifndef SBS_MAIN_HEADER
#define FILENAME "Buildconf"
#define SOURCEFOLDER "src"
#define BUILDDIR "build"
#define OBJFILE ".o"
#define BINDIR "bin"
#define SBS_MAIN_HEADER
#define VERSION "0.1a"
extern bool VERBOSE;

//
// WINDOWS COMPATABILITY
// 
#ifdef _WIN32
  #include <io.h>
  #define mkdir(path, mode) mkdir(path)
#else
  #include <sys/stat.h>
  #include<unistd.h>
#endif

#ifndef _WIN32
  // Linux/glibc has asprintf
  #define _GNU_SOURCE
  #include <stdio.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  int asprintf(char **strp, const char *fmt, ...);
#endif
#include <sys/stat.h>

#ifdef _WIN32
  #include <windows.h>
  #define IS_DIR(path) is_dir_windows(path)
  static int is_dir_windows(const char *path) {
      DWORD attr = GetFileAttributesA(path);
      return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
  }
#else
  #define IS_DIR(path) is_dir_unix(path)
  static int is_dir_unix(const char *path) {
      struct stat s;
      return (stat(path, &s) == 0) && S_ISDIR(s.st_mode);
  }
#endif


//
// PROTOTYPES
//

// make a new buildfile
void makebuildfile(char *path, char *name);
// build the application
void build(char *builddir, bool mode, SbsConf *config);

#endif
