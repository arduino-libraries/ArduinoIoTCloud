#include "CBOR.h"

CommandId toCommandId(CBORCommandTag tag) {
  switch(tag) {
  case CBORCommandTag::CBOROtaBeginUp:
    return CommandId::OtaBeginUpId;
  case CBORCommandTag::CBORThingBeginCmd:
    return CommandId::ThingBeginCmdId;
  case CBORCommandTag::CBORLastValuesBeginCmd:
    return CommandId::LastValuesBeginCmdId;
  case CBORCommandTag::CBORDeviceBeginCmd:
    return CommandId::DeviceBeginCmdId;
  case CBORCommandTag::CBOROtaProgressCmdUp:
    return CommandId::OtaProgressCmdUpId;
  case CBORCommandTag::CBORTimezoneCommandUp:
    return CommandId::TimezoneCommandUpId;
  case CBORCommandTag::CBOROtaUpdateCmdDown:
    return CommandId::OtaUpdateCmdDownId;
  case CBORCommandTag::CBORThingUpdateCmd:
    return CommandId::ThingUpdateCmdId;
  case CBORCommandTag::CBORLastValuesUpdate:
    return CommandId::LastValuesUpdateCmdId;
  case CBORCommandTag::CBORTimezoneCommandDown:
    return CommandId::TimezoneCommandDownId;
  default:
    return CommandId::UnknownCmdId;
  }
}


CBORCommandTag toCBORCommandTag(CommandId id) {
  switch(id) {
  case CommandId::OtaBeginUpId:
    return CBORCommandTag::CBOROtaBeginUp;
  case CommandId::ThingBeginCmdId:
    return CBORCommandTag::CBORThingBeginCmd;
  case CommandId::LastValuesBeginCmdId:
    return CBORCommandTag::CBORLastValuesBeginCmd;
  case CommandId::DeviceBeginCmdId:
    return CBORCommandTag::CBORDeviceBeginCmd;
  case CommandId::OtaProgressCmdUpId:
    return CBORCommandTag::CBOROtaProgressCmdUp;
  case CommandId::TimezoneCommandUpId:
    return CBORCommandTag::CBORTimezoneCommandUp;
  case CommandId::OtaUpdateCmdDownId:
    return CBORCommandTag::CBOROtaUpdateCmdDown;
  case CommandId::ThingUpdateCmdId:
    return CBORCommandTag::CBORThingUpdateCmd;
  case CommandId::LastValuesUpdateCmdId:
    return CBORCommandTag::CBORLastValuesUpdate;
  case CommandId::TimezoneCommandDownId:
    return CBORCommandTag::CBORTimezoneCommandDown;
  default:
    return CBORCommandTag::CBORUnknownCmdTag;
  }
}