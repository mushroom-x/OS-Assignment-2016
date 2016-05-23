#include "FDT.h"

FDT * initFdt(FDT* fdtPtr){
	fdtPtr->filePtr = 0;
	fdtPtr->isUpate = FALSE;

	return fdtPtr;
}
