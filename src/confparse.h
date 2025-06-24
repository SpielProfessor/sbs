#include "dirdiscover.h"
#include <stdbool.h>
#ifndef SBS_CONFIG_PARSE
typedef struct SbsConf {
  //[info]
  char *name;
  char *version;

  // [libraries]
  StrVec *libraries;
  // [compiler]
  char *compiler;
  char *ccargs_rel;
  char *ccargs_dbg;
  char *ldargs_dbg;
  char *ldargs_rel;

  StrVec *filetypes;
  char *linker;

  // [scripts]
  StrVec *prebuild;
  StrVec *postbuild;

  StrVec *postprocess;
  StrVec *preprocess;
  bool verbose;
} SbsConf;
SbsConf *sbsConfDefault();
int parse_config(const char *filename, SbsConf **conf_out);
void sbsFreeConf(SbsConf *conf);
char *strVecLibSerialize(StrVec *vector);
#define SBS_CONFIG_PARSE
#endif
