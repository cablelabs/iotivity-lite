#ifndef OC_STREAMLINED_ONBOARDING_H
#define OC_STREAMLINED_ONBOARDING_H

#define OC_SO_MAX_CRED_LEN (128)

typedef struct oc_so_info_t {
  char uuid[OC_UUID_LEN];
  char cred[OC_SO_MAX_CRED_LEN];
  struct oc_so_info_t *next;
} oc_so_info_t;

int oc_so_generate_psk(char *psk_output);
void oc_so_append_info(oc_so_info_t *head, oc_so_info_t *new_info);
void oc_so_info_free(oc_so_info_t *head);

#endif /* OC_STREAMLINED_ONBOARDING_H */
