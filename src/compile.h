#include "confparse.h"
#ifndef SBS_COMPILE_FN
int compileFile(const char *inputPath, const char *outputPath, int forceRebuild,
                SbsConf *config, bool dbgmode);
#define SBS_COMPILE_FN
#endif
