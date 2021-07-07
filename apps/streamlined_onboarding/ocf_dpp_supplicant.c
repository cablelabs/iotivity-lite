#include <stdlib.h>
#include <wpa_ctrl.h>
#include "oc_api.h"
#include "oc_core_res.h"
#include "oc_pstat.h"
#include "oc_base64.h"
#include "oc_streamlined_onboarding.h"

static char *ctrl_iface = NULL;

static int
send_so_info(oc_so_info_t *so_info)
{
  (void) so_info;
  struct wpa_ctrl *ctrl = NULL;
  if (!ctrl_iface) {
    OC_ERR("wpa_supplicant control interface not specified");
    return -1;
  }
  ctrl = wpa_ctrl_open(ctrl_iface);
  if (ctrl == NULL) {
    OC_ERR("Failed to open wpa_supplicant interface");
    return -1;
  }
  OC_DBG("Opened wpa_supplicant control interface");

  // TODO Send actual info
  return 0;
}

/* TODO
 * Currently generates a simple, base64 encoded 16-byte PSK. Should be updated
 * to generate a COSE-encoded symmetric key.
 */
static int
generate_so_psk(char *psk_output)
{
  // Similar to oc_tls generation of random pin
  int psk_len = 16;
  uint8_t psk[psk_len];
  for (int i = 0; i < psk_len; i++) {
    psk[i] = oc_random_value();
  }
  int output_len = oc_base64_encode(psk, psk_len, (uint8_t *)psk_output, OC_SO_MAX_CRED_LEN);
  if (output_len < 0)
    return -1;
  psk_output[output_len] = '\0';
  return 0;
}

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
  if (generate_so_psk(so_info.cred) != 0)
    return -1;
  OC_DBG("Generated streamlined onboarding PSK: %s\n", so_info.cred);

  // TODO: Send to Supplicant
  return send_so_info(&so_info);
}
