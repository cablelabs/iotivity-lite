#include <stdio.h>
#include <wpa_ctrl.h>
#include "oc_api.h"
#include "oc_core_res.h"
#include "oc_pstat.h"
#include "oc_base64.h"
#include "oc_streamlined_onboarding.h"

static char ctrl_iface[128];

static int
send_so_info(oc_so_info_t *so_info)
{
  (void) so_info;
  struct wpa_ctrl *ctrl = NULL;

  ctrl = wpa_ctrl_open(ctrl_iface);
  if (ctrl == NULL) {
    OC_ERR("Failed to open wpa_supplicant interface");
    return -1;
  }
  OC_DBG("Opened wpa_supplicant control interface");

  char wpa_command[29 + strlen(so_info->uuid) + strlen(so_info->cred)];
  sprintf(wpa_command, "DPP_OCF_INFO_ADD uuid=%s cred=%s", so_info->uuid, so_info->cred);
  OC_DBG("WPA_CTRL command to send: %s\n", wpa_command);

  size_t reply_len = 128;
  char reply_buf[reply_len];
  int ret = wpa_ctrl_request(ctrl, wpa_command, strlen(wpa_command), reply_buf, &reply_len, NULL);
  if (ret == -2) {
    OC_ERR("'%s' command timed out.", wpa_command);
    return -2;
  } else if (ret < 0) {
    OC_ERR("'%s' command failed.", wpa_command);
    return -1;
  }
  OC_DBG("wpa_ctrl response: %s", reply_buf);
  wpa_ctrl_close(ctrl);
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

static void
read_config(char *config_path)
{
  FILE *fp = NULL;
  fp = fopen(config_path, "r");
  if (!fp) {
    OC_ERR("Failed to open DPP config file");
    return;
  }

  char line[128];
  while (fgets(line, sizeof(line), fp) != NULL) {
    line[strlen(line) - 1] = '\0';
    sscanf(line, "ctrl_iface=%s", ctrl_iface);
  }
}

int
dpp_so_info_init(char *config_path)
{
  oc_sec_pstat_t *ps = oc_sec_get_pstat(0);
  if (ps->s != OC_DOS_RFOTM) {
    OC_DBG("Device not in RFOTM; will not generate SO info");
    return 1;
  }

  read_config(config_path);

  oc_so_info_t so_info;
  oc_uuid_to_str(oc_core_get_device_id(0), so_info.uuid, sizeof(so_info.uuid));
  OC_DBG("Generating DPP streamlined onboarding info for with UUID: %s", so_info.uuid);
  if (generate_so_psk(so_info.cred) != 0)
    return -1;
  OC_DBG("Generated streamlined onboarding PSK: %s\n", so_info.cred);

  // TODO: Send to Supplicant
  return send_so_info(&so_info);
}
