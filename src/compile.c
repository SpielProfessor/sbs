#include "main.h"
#include "utils.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define BINCMD "gcc" // or "clang", etc.

#define COMPILE_S "%s "
#define OUTPUT_S "%s "
#define INPUT_S "%s "
#define EXTRAPRAMS_S "%s "
int compileFile(const char *inputPath, const char *outputPath, int forceRebuild,
                SbsConf *config, bool dbgmode) {
  int returnval = 0;
  int compileCommandReturn = 0;
  char *normalizedOutput = pathNormalize(outputPath);
  char *normalizedInput = pathNormalize(inputPath);
  char *command = NULL;
  char *compiler = NULL;
  if (!config->compiler) {
    compiler = BINCMD;
  } else {
    compiler = config->compiler;
  }
  int r = asprintf(&command, COMPILE_S INPUT_S "-o" OUTPUT_S "-c" EXTRAPRAMS_S,
                   compiler, normalizedInput, normalizedOutput,
                   dbgmode ? config->ccargs_rel : config->ccargs_dbg);

  //
  // ERROR HANDLING
  //
  if (r < -1) {
    puts("Couldn't allocate path!");
    returnval = -1;
    goto compileFileReturn;
  }

  if (!normalizedOutput || !normalizedInput) {
    puts("Couldn't normalize input or output path, or the command");
    returnval = -1;
    goto compileFileReturn;
  }
  createParentDirs(normalizedOutput);
  //
  // FORCE REBUILD
  //
  if (forceRebuild) {
    goto compileAndReturn;
  }

  //
  // NON-FORCED REBUILD
  //

  // output up-to-date
  long outdate = getFileModTime(outputPath);
  long indate = getFileModTime(inputPath);
  // error getting input time - output time may be -1 as it may not exist
  if (indate == -1) {
    printf("Couldn't get time of `%s`: returned value `%ld`\n", inputPath,
           indate);
    returnval = -1;
    goto compileFileReturn;
  }
  // needs no recompile
  if (indate <= outdate) {
    if (config->verbose) {
      printf("Didn't compile %s -> %s (up-to-date)\n", normalizedInput,
             normalizedOutput);
    }
    goto compileFileReturn;
  }

  // needs a recompile - continue

  //
  // COMPILE AND EXIT
  //
compileAndReturn:
  printf("compiling %s -> %s with %s\n", normalizedInput, normalizedOutput,
         command);
  compileCommandReturn = system(command);
  if (compileCommandReturn == 0) {
    if (config->verbose) {
      printf("Compiled %s -> %s\n", normalizedInput, normalizedOutput);
    }
  } else {
    printf("Error %d while compiling %s -> %s\n", compileCommandReturn,
           normalizedInput, normalizedOutput);
  }

  //
  // RETURN
  //
compileFileReturn:
  free(normalizedInput);
  free(normalizedOutput);
  free(command);
  return returnval;
}
