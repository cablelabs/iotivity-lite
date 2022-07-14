#include "oc_api.h"
#include "oc_streamlined_onboarding.h"
#include "oc_diplomat.h"

oc_so_info_t *so_info_list;

void
oc_diplomat_get(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data)
{
  (void) user_data;
  OC_DBG("GET /diplomat\n");
  if (so_info_list == NULL) {
    PRINT("No updated information to send\n");
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
