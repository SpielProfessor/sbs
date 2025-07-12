/************************************
 * SIMPLE BUILD SYSTEM              *
 ************************************/

// TODO: MAKE PORTABLE (not just POSIX/GCC systems)

#include "main.h"
#include "compile.h"
#include "confparse.h"
#include "dirdiscover.h"
#include "link.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

bool VERBOSE = true;

//
// MAIN ENTRY POINT
//
int main(int argc, char **argv) {
  // too few arguments supplied
  if (argc < 2) {
    puts("Invalid amount of arguments supplied; Try 'sbs -h' for help");
    return 0;
  }
  // help message
  if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    puts("SBS  -  THE SIMPLE BUILD SYSTEM                             \n"
         "-------------------------------                             \n"
         "                                                            \n"
         "USAGE:                                                      \n"
         "    sbs    --new   / -n    <project-name>                   \n"
         "    sbs    --init  / -i    <path> <project-name>            \n"
         "    sbs    --build / -b    [release-mode]                   \n"
         "    sbs    --run   / -R    [release-mode]                   \n"
         "    sbs    --help  / -h                                     \n"
         "  WHERE:                                                    \n"
         "    project-name: single-word project name                  \n"
         "    release-mode: either `--debug` (-d) or `--release` (-r) \n");
    printf("You are running SBS v.%s, compiled on %s at %s\n", VERSION,
           __DATE__, __TIME__);
    return 0;
  }

  // create a new sbs project
  if (strcmp(argv[1], "--new") == 0 || strcmp(argv[1], "-n") == 0) {
    if (argc < 3) {
      puts("ERROR: Usage: sbs --new <project-name>");
      return 1;
    }
    mkdir(argv[2],
          S_IRUSR | S_IWUSR | S_IXUSR); // create a new directory
    makebuildfile(argv[2], argv[2]);
    return 0;
  }
  if (strcmp(argv[1], "--init") == 0 || strcmp(argv[1], "-i") == 0) {
    if (argc < 4) {
      puts("Usage: sbs --init <path> <name>");
      return 1;
    }
    makebuildfile(argv[2], argv[3]);
    return 0;
  }
  // ALL THAT FOLLOWS REQUIRES A CONFIGURATION FILE
  SbsConf *config = {0};
  if (parse_config("./Buildconf", &config) != 0) {
    return -1;
  }

  if (strcmp(argv[1], "--build") == 0 || strcmp(argv[1], "-b") == 0) {
    bool releasemode = false;
    if (argc > 2) {
      if (strcmp(argv[2], "--release") == 0 || strcmp(argv[2], "-R") == 0) {
        releasemode = true;
      }
    }

    if (releasemode) {
      puts("Building project in release mode.");
    } else {
      puts("Building project in debug mode.");
    }
    build(".", releasemode, config);
  }
  if (strcmp(argv[1], "--run") == 0 || strcmp(argv[1], "-r") == 0) {
    bool releasemode = false;
    if (argc > 2) {
      if (strcmp(argv[2], "--release") == 0 || strcmp(argv[2], "-R") == 0) {
        releasemode = true;
      }
    }
    if (releasemode) {
      puts("Building and running project in release mode.");
    } else {
      puts("Building and running project in debug mode.");
    }
    build(".", releasemode, config);
    // execute
    char *executable = BUILDDIR "/" BINDIR "/";
    executable = strcatr(executable, releasemode ? "release" : "debug");
    system(executable);
    free(executable);
  }
  sbsFreeConf(config);
}

// create the build directory
void mkbuilddir(char *directory) {
  char *out = pathPush(directory, BUILDDIR);
  mkdir(out,
        S_IRUSR | S_IWUSR | S_IXUSR); // create a new directory
  pathPushR(&out, BINDIR);
  mkdir(out, S_IRUSR | S_IWUSR | S_IXUSR);
  free(out);
}

// build the application
void build(char *builddir, bool releaseMode, SbsConf *config) {
  printf("Building %s v.%s in %s mode\n", config->name, config->version,
         releaseMode ? "release" : "debug");

  // TODO: prebuild

  // initialize build directory structure
  mkbuilddir(builddir);
  // get path to source directory; get source files
  char *sourcedir = pathPush(builddir, SOURCEFOLDER);
  // TODO: use config
  StrVec *sourcefiles = srcDiscover(
      sourcedir, (const char *[]){".c", ".cpp", ".cc", ".cxx", NULL});

  StrVecElement *c = sourcefiles->first;

  // compile files to the build directory
  while (c != NULL) {
    char *nextpath = pathPush(builddir, BUILDDIR);
    if (releaseMode) {
      pathPushR(&nextpath, "release");
    } else {
      pathPushR(&nextpath, "debug");
    }
    pathPushR(&nextpath, c->content);
    char *tmp = strcatr(nextpath, OBJFILE);
    free(nextpath);
    nextpath = tmp;
    // TODO: preprocess

    // TODO: maybe implement force-compile option
    compileFile(c->content, nextpath, 0, config, releaseMode);

    // TODO: post-process
    c = c->next;
    free(nextpath);
  }
  // link files
  char *bd = pathPush(builddir, BUILDDIR);
  if (releaseMode) {
    pathPushR(&bd, "release");
  } else {
    pathPushR(&bd, "debug");
  }

  StrVec *built = srcDiscover(bd, (const char *[]){".o", NULL});
  linkFiles(built, releaseMode ? "build/bin/release" : "build/bin/debug", 0,
            config, releaseMode);
  free(bd);
  delStrVec(built);

  // TODO: Postbuild
  // clean up
  free(sourcedir);
  delStrVec(sourcefiles);
}

// create a new Buildfile at the given path
void makebuildfile(char *dirpath, char *name) {
  // allocate path
  char *path = malloc(strlen(dirpath) + strlen("/" FILENAME));
  if (!path) {
    puts("Malloc failed! Execution aborted!");
    exit(-1);
  }
  // create path
  strcpy(path, dirpath);
  strcat(path, "/" FILENAME);

  if (access(path, F_OK) == 0) { // check if Buildfile exists, abort if so
    printf("file already exists at %s - aborting\n", path);
    exit(1);
  } else {
    if (VERBOSE) {
      printf("INFO: new buildfile -> %s\n", path);
    }
  }

  // initialize buildfile
  FILE *fd = fopen(path, "w+");
  if (fd != NULL) {
    fprintf(fd,
            ";\n"
            "; Build configuration for %s\n"
            "; This file is the build configuration for use with `SBS`, the "
            "simple build system\n"
            ";\n"
            "[info]\n"
            "name = %s\n"
            "version = 1.0\n",
            name, name);
  } else {
    printf("ERROR: couldn't open %s for writing! Execution aborted!\n", path);
    exit(-1);
  }

  // clean up
  fclose(fd);
  free(path);
}
