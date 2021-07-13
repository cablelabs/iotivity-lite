#ifndef OCF_SUPPLICANT_UTILS_H
#define OCF_SUPPLICANT_UTILS_H

#include "oc_streamlined_onboarding.h"

int dpp_so_init(char *config_path);
int dpp_send_so_info(void);
int dpp_so_teardown(void);
oc_so_info_t *dpp_so_info_poll(void);
#endif /* OCF_SUPPLICANT_UTILS_H */
