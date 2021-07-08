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

