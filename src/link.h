#include "confparse.h"
#include "dirdiscover.h"
#ifndef SBS_LINK_FN
// TODO: self-made implementation
int linkFiles(StrVec *inputs, const char *outputPath, int forceRebuild,
              SbsConf *config, bool dbgmode);
#define SBS_LINK_FN
#endif
