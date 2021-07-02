#include "oc_api.h"
#include "oc_core_res.h"
#include "oc_pstat.h"
#include "oc_streamlined_onboarding.h"

int
dpp_so_info_init(void)
{
  oc_sec_pstat_t *ps = oc_sec_get_pstat(0);
  if (ps->s != OC_DOS_RFOTM) {
    OC_DBG("Device not in RFOTM; will not generate SO info");
    return 1;
  }

  oc_so_info_t so_info;
  oc_uuid_to_str(oc_core_get_device_id(0), so_info.uuid, sizeof(so_info.uuid));
  OC_DBG("Generating DPP streamlined onboarding info for with UUID: %s", so_info.uuid);
  return 0;
}
