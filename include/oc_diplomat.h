#ifndef OC_DIPLOMAT_H
#define OC_DIPLOMAT_H

#include "oc_streamlined_onboarding.h"

extern oc_so_info_t *so_info_list;

void oc_diplomat_process_encoded_so_info(char *new_so_info);
void oc_diplomat_get(oc_request_t *request, oc_interface_mask_t iface_mask, void *user_data);
void oc_diplomat_teardown(void);

#endif /* OC_DIPLOMAT_H */
