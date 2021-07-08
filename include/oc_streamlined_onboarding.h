#ifndef OC_STREAMLINED_ONBOARDING_H
#define OC_STREAMLINED_ONBOARDING_H

#define OC_SO_MAX_CRED_LEN (128)

typedef struct oc_so_info_t {
  char uuid[OC_UUID_LEN];
  char cred[OC_SO_MAX_CRED_LEN];
  struct oc_so_info_t *next;
} oc_so_info_t;

int oc_so_generate_psk(char *psk_output);

#endif /* OC_STREAMLINED_ONBOARDING_H */
