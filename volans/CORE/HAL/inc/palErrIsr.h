
#include "halTypes.h"

eHalStatus isrMasterTrdyErr(tHddHandle hHdd);
eHalStatus isrMasterRetryErr(tHddHandle hHdd);
eHalStatus isrMasterTarAbort(tHddHandle hHdd);
eHalStatus isrMasterMasAbort(tHddHandle hHdd);
eHalStatus isrMasterDetSerr(tHddHandle hHdd);
eHalStatus isrMasterDetPerr(tHddHandle hHdd);
eHalStatus isrTargetAhbAddrErr(tHddHandle hHdd);
eHalStatus isrMisalignAddrErr(tHddHandle hHdd);
eHalStatus isrTarget1DiscardErr(tHddHandle hHdd);
eHalStatus isrWriteByteEnableErr(tHddHandle hHdd);
eHalStatus isrXmitDisableChange(tHddHandle hHdd);
eHalStatus isrTargetBlocked(tHddHandle hHdd);
