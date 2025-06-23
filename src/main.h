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
// PROTOTYPES
//

// make a new buildfile
void makebuildfile(char *path, char *name);
// build the application
void build(char *builddir, bool mode, SbsConf *config);

#endif
