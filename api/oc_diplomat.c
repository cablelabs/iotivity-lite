#include "oc_api.h"
#include "oc_streamlined_onboarding.h"
#include "oc_diplomat.h"
#include "oc_base64.h"

oc_so_info_t *so_info_list;

void
oc_diplomat_process_encoded_so_info(char *new_so_info)
{
  int buf_len = oc_base64_decode((uint8_t *)new_so_info, strlen(new_so_info));
  OC_DBG("Buf len is %d\nBuffer is %s\n", buf_len, new_so_info);
  if (buf_len < 0) {
    OC_ERR("Failed to decode new SO info\n");
    return;
  }

  oc_rep_t *rep = NULL;
  struct oc_memb rep_objects = { sizeof(oc_rep_t), 0, 0, 0, 0 };
  oc_rep_set_pool(&rep_objects);
  int err = oc_parse_rep((uint8_t *)new_so_info, buf_len, &rep);
  if (err != 0) {
    OC_ERR("Failed to parse new SO info\n");
    return;
  }

#ifdef OC_DEBUG
  char * json;
  size_t json_size;
  json_size = oc_rep_to_json(rep, NULL, 0, true);
  json = (char *)malloc(json_size + 1);
  oc_rep_to_json(rep, json, json_size + 1, true);
  OC_DBG("Decoded SO info:\n%s\n", json);
  free(json);
#endif /* OC_DEBUG */

  if (rep)
    oc_free_rep(rep);
}

void
oc_diplomat_get(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  (void) user_data;
  OC_DBG("GET /diplomat\n");
  if (so_info_list == NULL) {
    OC_DBG("No updated information to send\n");
    oc_send_response(request, OC_STATUS_OK);
    return;
  }

  oc_rep_start_root_object();
  switch (iface_mask) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
    /* fall through */
  case OC_IF_R:
    oc_rep_open_array(root, soinfo);
    oc_so_info_t *cur = so_info_list;

    while (cur != NULL) {
      OC_DBG("Setting SO info values with uuid: %s and cred: %s\n", cur->uuid, cur->cred);
      oc_rep_object_array_begin_item(soinfo);
      oc_rep_set_text_string(soinfo, uuid, cur->uuid);
      oc_rep_set_text_string(soinfo, cred, cur->cred);
      oc_rep_object_array_end_item(soinfo);
      cur = cur->next;
    }
    oc_rep_close_array(root, soinfo);
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}


void
oc_diplomat_teardown(void)
{
  oc_so_info_free(so_info_list);
}
