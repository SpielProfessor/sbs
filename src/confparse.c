#include "confparse.h"
#include <stdlib.h>
#include <string.h>

// strdup alias
#define aar strdup
SbsConf *sbsConfDefault() {
  SbsConf *returned = malloc(sizeof(SbsConf));
  returned->ccargs_dbg = aar("");
  returned->ccargs_rel = aar("");
  returned->ldargs_dbg = aar("");
  returned->ldargs_rel = aar("");
  returned->ldmode_ar  = false;

  returned->compiler = aar("cc");
  returned->filetypes = strVecNew();
  returned->linker = aar("cc");
  returned->name = NULL;
  returned->postbuild = aar("");
  returned->postprocess = aar("");
  returned->prebuild = aar("");
  returned->preprocess = aar("");
  returned->version = NULL;
  returned->verbose = false;
  returned->libraries = strVecNew();
  return returned;
}

void sbsFreeConf(SbsConf *conf) {
  if (!conf)
    return;
  free(conf->name);
  free(conf->version);
  free(conf->compiler);
  free(conf->ccargs_rel);
  free(conf->ccargs_dbg);
  free(conf->ldargs_rel);
  free(conf->ldargs_dbg);
  delStrVec(conf->filetypes);
  free(conf->linker);
  free(conf->prebuild);
  free(conf->postbuild);
  free(conf->postprocess);
  free(conf->preprocess);
  delStrVec(conf->libraries);

  free(conf);
}

// actual parsing

#include <ctype.h>
#include <ini.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool parse_bool(const char *value) {
  if (!value)
    return false;
  while (*value && isspace((unsigned char)*value))
    value++;

  if (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0)
    return true;
  if (strcasecmp(value, "false") == 0 || strcmp(value, "0") == 0)
    return false;

  return false;
}

static void assign_string(char **target, const char *value) {
  if (*target) {
    free(*target);
  }
  *target = aar(value);
}

static void assign_strvec(StrVec **vec, const char *value) {
  if (*vec) {
    delStrVec(*vec);
    *vec = NULL;
  }
  *vec = strVecNew();
  const char *p = value;
  while (*p) {
    while (*p && (*p == ' ' || *p == '\t'))
      p++;
    if (!*p)
      break;
    const char *start = p;
    while (*p && *p != ' ' && *p != '\t')
      p++;
    size_t len = p - start;
    char *word = malloc(len + 1);
    if (!word)
      continue;
    memcpy(word, start, len);
    word[len] = '\0';
    strVecPush(*vec, word);
  }
}

static int handler(void *user, const char *section, const char *name,
                   const char *value) {
  SbsConf *conf = (SbsConf *)user;

  if (strcmp(section, "info") == 0) {
    if (strcmp(name, "name") == 0) {
      assign_string(&conf->name, value);
    } else if (strcmp(name, "version") == 0) {
      assign_string(&conf->version, value);
    }

  } else if (strcmp(section, "compiler") == 0) {
    if (strcmp(name, "compiler") == 0) {
      assign_string(&conf->compiler, value);
      // compilation args
    } else if (strcmp(name, "ccargs-rel") == 0 ||
               strcmp(name, "ccargs_rel") == 0) {
      assign_string(&conf->ccargs_rel, value);
    } else if (strcmp(name, "ccargs-dbg") == 0 ||
               strcmp(name, "ccargs_dbg") == 0) {
      assign_string(&conf->ccargs_dbg, value);
      // linker args
    } else if (strcmp(name, "ldargs-rel") == 0 ||
               strcmp(name, "ldargs_rel") == 0) {
      puts("reading rlink args");
      assign_string(&conf->ldargs_rel, value);
    } else if (strcmp(name, "ldargs-dbg") == 0 ||
               strcmp(name, "ldargs_dbg") == 0) {
      puts("reading dlink args");
      assign_string(&conf->ldargs_dbg, value);
    } else if (strcmp(name, "filetypes") == 0) {
      assign_strvec(&conf->filetypes, value);
    } else if (strcmp(name, "linker") == 0) {
      assign_string(&conf->linker, value);
    } else if (strcmp(name, "ldmode-ar")==0) {
      conf->ldmode_ar=parse_bool(value);
    }

  } else if (strcmp(section, "scripts") == 0) {
    if (strcmp(name, "prebuild") == 0) {
      assign_string(&conf->prebuild, value);
    } else if (strcmp(name, "postbuild") == 0) {
      assign_string(&conf->postbuild, value);
    } else if (strcmp(name, "postprocess") == 0) {
      assign_string(&conf->postprocess, value);
    } else if (strcmp(name, "preprocess") == 0) {
      assign_string(&conf->preprocess, value);
    }

  } else if (strcmp(section, "sbsconf") == 0) {
    if (strcmp(name, "verbose") == 0) {
      conf->verbose = parse_bool(value);
    }
  } else if (strcmp(section, "libraries") == 0) {
    assign_strvec(&conf->libraries, value);
  }

  return 1; // continue parsing
}

int parse_config(const char *filename, SbsConf **conf_out) {
  if (!conf_out)
    return -1;

  SbsConf *conf = sbsConfDefault();
  if (!conf)
    return -1;

  int r = ini_parse(filename, handler, conf);
  if (r < 0) {
    fprintf(stderr, "Failed to load config file: %s\n", filename);
    sbsFreeConf(conf);
    return -1;
  }

  // Validate required fields:
  if (!conf->name) {
    fprintf(stderr,
            "Config error: missing required field 'name' in [info] section.\n");
    sbsFreeConf(conf);
    exit(EXIT_FAILURE);
  }
  if (!conf->version) {
    fprintf(
        stderr,
        "Config error: missing required field 'version' in [info] section.\n");
    sbsFreeConf(conf);
    exit(EXIT_FAILURE);
  }

  *conf_out = conf;
  return 0;
}

// returns a serialized library list (with -l)
char *strVecLibSerialize(StrVec *inputs) {
  char *returned = NULL;
  int retLength = 0;

  // get required length
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    retLength += strlen(el->content) + 3; // +3 for " -l"
  }
  retLength++; // +1 for \0
  // allocate
  returned = malloc(retLength);
  if (!returned) {
    return NULL;
  }
  strcpy(returned, "");
  // actually insert text
  for (StrVecElement *el = inputs->first; el != NULL; el = el->next) {
    strcat(returned, " -l");
    strcat(returned, el->content);
  }
  return returned;
}
