#include "confparse.h"
#include "dirdiscover.h"
#ifndef SBS_LINK_FN
// TODO: self-made implementation
void linkFiles(StrVec *inputs, const char *outputPath, int forceRebuild,
               SbsConf *config);
#define SBS_LINK_FN
#endif
