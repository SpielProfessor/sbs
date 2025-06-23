#include "dirdiscover.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// TODO: library

#define BINCMD "gcc"

// Forward declaration
int createParentDirs(const char *path);

// Get latest mtime among input files
time_t latestInputTime(StrVec *inputs) {
  time_t latest = 0;
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    struct stat st;
    if (stat(el->content, &st) != 0) {
      perror(el->content);
      continue;
    }
    if (st.st_mtime > latest)
      latest = st.st_mtime;
  }
  return latest;
}

void linkFiles(StrVec *inputs, const char *outputPath, int forceRebuild,
               SbsConf *config) {
  // Check if rebuild is necessary
  struct stat outStat;
  int needsLinking = forceRebuild;

  if (!needsLinking) {
    if (stat(outputPath, &outStat) != 0) {
      needsLinking = 1; // Output doesn't exist
    } else {
      time_t latest = latestInputTime(inputs);
      if (latest > outStat.st_mtime)
        needsLinking = 1;
    }
  }

  if (!needsLinking) {
    if (VERBOSE) {
      printf("Skipping linking (up-to-date): %s\n", outputPath);
    }
    return;
  }

  if (createParentDirs(outputPath) != 0) {
    fprintf(stderr, "Failed to create output directory for %s\n", outputPath);
    return;
  }

  // Build args array dynamically
  int count = 0;
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next)
    count++;

  // +3: compiler + "-o" + outputPath + NULL
  char **args = malloc((count + 4) * sizeof(char *));
  if (!args) {
    perror("malloc");
    return;
  }

  int i = 0;
  args[i++] = BINCMD;
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    args[i++] = el->content;
  }
  args[i++] = "-o";
  args[i++] = (char *)outputPath;

  args[i] = NULL;

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    free(args);
    return;
  }

  if (pid == 0) {
    execvp(BINCMD, args);
    perror("exec failed");
    exit(1);
  }

  int status;
  if (waitpid(pid, &status, 0) == -1) {
    perror("waitpid");
    free(args);
    return;
  }

  if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
    printf("Linked %d files → %s\n", count, outputPath);
  } else {
    fprintf(stderr, "Linking failed: %s\n", outputPath);
  }

  free(args);
}
