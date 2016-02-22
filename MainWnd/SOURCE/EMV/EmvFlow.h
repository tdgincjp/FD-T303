#ifndef EMVFLOW_H
#define EMVFLOW_H

#include "..\\defs\\Constant.h"
#include "..\\defs\\struct.h"
#include "..\\transactions\\Finan.h"

BOOL InitEmv();
BOOL InitDevice();
BOOL InitData();
void InitCallBack();
void CloseDevice();

#endif
