#include "dirdiscover.h"
#include "main.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define LINKCMD "ld"
#define COMPILE_S "%s"
#define OUTPUT_S "%s"
#define INPUT_S "%s"
#define EXTRAPRAMS_S "%s"
#define LIBRARIES "%s"

// Forward declaration
int createParentDirs(const char *path);

// Get latest mtime among input files
long latestInputTime(StrVec *inputs) {
  long latest = 0;
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    int modtime = getFileModTime(el->content);
    if (modtime > latest)
      latest = modtime;
  }
  return latest;
}

int linkFiles(StrVec *inputs, const char *outputPath, int forceRebuild,
              SbsConf *config, bool releaseMode) {
  // INITIALIZE VARIABLES
  int retval = 0;
  int cmdReturned = -1;
  char *command = NULL;
  char *normalizedOutput = pathNormalize(outputPath);
  char *linker = NULL;
  char *libraries = strVecLibSerialize(config->libraries);
  if (!libraries) {
    puts("ERROR: couldn't parse or allocate library string!");
    goto sbsLinkFilesExit;
  }
  // switch to preferred linker
  if (!config->compiler) {
    linker = LINKCMD;
  } else {
    linker = config->compiler;
  }
  // serialized input files
  char *inputFiles = NULL;
  int inputLength = 0;
  if (inputs->first == NULL) {
    retval = -1;
    puts("INFO: No files were found to link");
    goto sbsLinkFilesExit;
  }
  // get required length
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    inputLength += strlen(el->content) + 1; // +1 for following space
  }
  inputLength++; // +1 for \0
  // allocate
  inputFiles = malloc(inputLength);
  if (!inputFiles) {
    puts("Couldn't allocate input variables string!");
    retval = -1;
    goto sbsLinkFilesExit;
  }
  strcpy(inputFiles, "");
  // actually insert text
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    strcat(inputFiles, el->content);
    strcat(inputFiles, " ");
  }

  // build command string
  int r = asprintf(
      &command,
      COMPILE_S
      " " INPUT_S /*INPUT_S should have a trailing space*/ "-o " OUTPUT_S
      " " EXTRAPRAMS_S " " LIBRARIES,
      linker, inputFiles, normalizedOutput,
      releaseMode ? config->ldargs_rel : config->ldargs_dbg, libraries);
  free(inputFiles); // serialized input files not needed
  free(libraries);  // libraries neither
  if (r < 0) {
    puts("Couldn't allocate command string!");
    retval = -1;
    goto sbsLinkFilesExit;
  }

  // force-link files
  if (forceRebuild) {
    goto sbsLinkFiles;
  }

  // file date check
  if (getFileModTime(outputPath) >= latestInputTime(inputs)) {
    puts("Didn't link (up-to-date)");
    goto sbsLinkFilesExit;
  }

  //
  // CALL THE LINKER
  //
sbsLinkFiles:
  puts("Linking files");
  if (config->verbose) {
    printf("calling %s\n", command);
  }
  cmdReturned = system(command);
  if (cmdReturned != 0) {
    puts("Failed to link files!");
    goto sbsLinkFilesExit;
  }
  // CLEAN UP
sbsLinkFilesExit:
  free(command);
  free(normalizedOutput);
  return retval;
}
