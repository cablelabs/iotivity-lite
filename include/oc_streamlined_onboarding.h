#ifndef OC_STREAMLINED_ONBOARDING_H
#define OC_STREAMLINED_ONBOARDING_H

#define OC_SO_MAX_CRED_LEN (128)

typedef struct
{
  char uuid[OC_UUID_LEN];
  char cred[OC_SO_MAX_CRED_LEN];
} oc_so_info_t;

#endif /* OC_STREAMLINED_ONBOARDING_H */
