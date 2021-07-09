#include <stdio.h>
#include <wpa_ctrl.h>
#include "oc_api.h"
#include "oc_core_res.h"
#include "oc_pstat.h"
#include "oc_streamlined_onboarding.h"

static char ctrl_iface[128];
static struct wpa_ctrl *ctrl = NULL;

static int
send_so_info(oc_so_info_t *so_info)
{
  if (ctrl == NULL) {
    return -1;
  }

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
  return 0;
}

static int
read_config(char *config_path)
{
  FILE *fp = NULL;
  fp = fopen(config_path, "r");
  if (!fp) {
    OC_ERR("Failed to open DPP config file");
    return -1;
  }

  char line[128];
  while (fgets(line, sizeof(line), fp) != NULL) {
    line[strlen(line) - 1] = '\0';
    if (line[0] == '#') {
      continue;
    }
    sscanf(line, "ctrl_iface=%s", ctrl_iface);
  }
  fclose(fp);
  return 0;
}

int
dpp_so_gen_info(void)
{
  oc_sec_pstat_t *ps = oc_sec_get_pstat(0);
  if (ps->s != OC_DOS_RFOTM) {
    OC_DBG("Device not in RFOTM; will not generate SO info");
    return 1;
  }

  oc_so_info_t so_info;
  oc_uuid_to_str(oc_core_get_device_id(0), so_info.uuid, sizeof(so_info.uuid));
  OC_DBG("Generating DPP streamlined onboarding info for with UUID: %s", so_info.uuid);
  if (oc_so_generate_psk(so_info.cred) != 0)
    return -1;
  OC_DBG("Generated streamlined onboarding PSK: %s\n", so_info.cred);

  return send_so_info(&so_info);
}

int
dpp_so_init(char *config_path)
{
  if (read_config(config_path) < 0) {
    return -1;
  }
  ctrl = wpa_ctrl_open(ctrl_iface);
  if (ctrl == NULL) {
    OC_ERR("Failed to open wpa_supplicant interface");
    return -1;
  }
  OC_DBG("Opened wpa_supplicant control interface");
  return wpa_ctrl_attach(ctrl);
}

int
dpp_so_teardown(void)
{
  if (ctrl != NULL) {
    wpa_ctrl_detach(ctrl);
    wpa_ctrl_close(ctrl);
  }
  return 0;
}
