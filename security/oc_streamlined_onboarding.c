#include "oc_base64.h"
#include "oc_api.h"
#include "oc_streamlined_onboarding.h"

/* TODO
 * Currently generates a simple, base64 encoded 16-byte PSK. Should be updated
 * to generate a COSE-encoded symmetric key.
 */
int
oc_so_generate_psk(char *psk_output)
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

void
oc_so_append_info(oc_so_info_t *head, oc_so_info_t *new_info)
{
  if (head == NULL) {
    return;
  }
  while (head->next != NULL) {
    head = head->next;
  }
  head->next = new_info;
}

oc_so_info_t *
oc_so_parse_rep_array(oc_rep_t *so_info_rep_array)
{
  oc_so_info_t *head = NULL, *cur = NULL;
  size_t str_len;
  char *uuid = NULL, *cred = NULL;
  while (so_info_rep_array != NULL) {
    cur = malloc(sizeof(oc_so_info_t));
    oc_rep_get_string(so_info_rep_array->value.object, "uuid", &uuid, &str_len);
    uuid[str_len] = '\0';
    oc_rep_get_string(so_info_rep_array->value.object, "cred", &cred, &str_len);
    cred[str_len] = '\0';

    PRINT("Parsed UUID: %s\n", uuid);
    PRINT("Parsed CRED: %s with length %d\n", cred, str_len);
    strncpy(cur->uuid, uuid, OC_UUID_LEN - 1);
    strncpy(cur->cred, cred, OC_SO_MAX_CRED_LEN - 1);

    if (head == NULL) {
      head = cur;
    }
    else {
      oc_so_append_info(head, cur);
    }
    so_info_rep_array = so_info_rep_array->next;
  }
  return head;
}

void
oc_so_info_free(oc_so_info_t *head)
{
  oc_so_info_t *cur;
  while (head != NULL) {
    cur = head;
    head = head->next;
    free(cur);
  }
}
