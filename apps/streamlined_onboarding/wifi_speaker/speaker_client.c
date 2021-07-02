#include "oc_api.h"
#include "oc_core_res.h"
#include "port/oc_clock.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#define MAX_URI_LENGTH (30)

#define SCANF(...)                                                             \
  do {                                                                         \
    if (scanf(__VA_ARGS__) <= 0) {                                             \
      PRINT("ERROR Invalid input\n");                                          \
    }                                                                          \
  } while (0)

pthread_mutex_t mutex;
pthread_cond_t cv;
static pthread_t event_loop_thread;
struct timespec ts;

static char speaker_addr[MAX_URI_LENGTH];
static oc_endpoint_t *speaker_server;

/* TODO: These should be set by an initial GET request */
static int currentvol = 0;
static bool currentmute = false;

int quit = 0;

static int
app_init(void)
{
  int ret = oc_init_platform("Linux", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.pc", "Speaker Controller", "ocf.1.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  return ret;
}

static void
signal_event_loop(void)
{
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cv);
  pthread_mutex_unlock(&mutex);
}

void
handle_signal(int signal)
{
  (void)signal;
  quit = 1;
  signal_event_loop();
}

static oc_discovery_flags_t
discovery(const char *anchor, const char *uri, oc_string_array_t types,
    oc_interface_mask_t iface_mask, oc_endpoint_t *endpoint,
    oc_resource_properties_t bm, void *user_data)
{
  (void)anchor;
  (void)user_data;
  (void)iface_mask;
  (void)bm;
  int i;
  int uri_len = strlen(uri);
  uri_len = (uri_len >= MAX_URI_LENGTH) ? MAX_URI_LENGTH - 1 : uri_len;
  for (i = 0; i < (int)oc_string_array_get_allocated_size(types); i++) {
    char *t = oc_string_array_get_item(types, i);
    if (strlen(t) == 11 && strncmp(t, "oic.r.audio", 11) == 0) {
      oc_endpoint_list_copy(&speaker_server, endpoint);
      strncpy(speaker_addr, uri, uri_len);
      speaker_addr[uri_len] = '\0';
      PRINT("Speaker addr: %s\n", speaker_addr);

      PRINT("Resource %s hosted at endpoints:\n", speaker_addr);
      oc_endpoint_t *ep = endpoint;
      while (ep != NULL) {
        PRINTipaddr(*ep);
        PRINT("\n");
        ep = ep->next;
      }

      // oc_do_get(a_light, light_server, NULL, &get_light, LOW_QOS, NULL);

      return OC_STOP_DISCOVERY;
    }
  }
  oc_free_server_endpoints(endpoint);
  return OC_CONTINUE_DISCOVERY;
}

/* TODO */
static void
issue_requests(void)
{
  PRINT("Discovering speaker device\n");
  oc_do_ip_discovery("oic.r.audio", &discovery, NULL);
}

static void
*ocf_event_thread(void *data)
{
  (void)data;
  oc_clock_time_t next_event;
  while (quit != 1) {
    // pthread_mutex_lock(&client_sync_lock);
    next_event = oc_main_poll();
    // pthread_mutex_unlock(&client_sync_lock);

    pthread_mutex_lock(&mutex);
    if (next_event == 0) {
      pthread_cond_wait(&cv, &mutex);
    }
    else {
      ts.tv_sec = (next_event / OC_CLOCK_SECOND);
      ts.tv_nsec = (next_event % OC_CLOCK_SECOND) * 1.e09 / OC_CLOCK_SECOND;
      pthread_cond_timedwait(&cv, &mutex, &ts);
    }
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void
display_menu()
{
  PRINT("#### Basic OCF Speaker Interface ####\n");
  PRINT("Options (select one):\n");
  PRINT("1) Set volume\n");
  PRINT("2) Mute\n");
  PRINT("3) Unmute\n");
  PRINT("0) Exit\n");
}

static void
handle_response(oc_client_response_t *data)
{
  PRINT("POST Response:\n");
  switch(data->code) {
    case OC_STATUS_CHANGED:
      PRINT("POST response: Changed\n");
      break;
    case OC_STATUS_SERVICE_UNAVAILABLE:
      PRINT("POST response: Service unavailable\n");
      break;
    case OC_STATUS_UNAUTHORIZED:
      PRINT("POST response: Unauthorized\n");
      break;
    default:
      PRINT("POST response code %d\n", data->code);
      break;
  }
}

static void
send_speaker_request(int newvol, bool mutestate)
{
  if (!speaker_server) {
    PRINT("Speaker Server has not been discovered!\n");
    return;
  }
  if (oc_init_post(speaker_addr, speaker_server, NULL, &handle_response, LOW_QOS, NULL)) {
    oc_rep_start_root_object();
    oc_rep_set_boolean(root, mute, mutestate);
    oc_rep_set_int(root, volume, newvol);
    oc_rep_end_root_object();
    if (oc_do_post())
      PRINT("Sent POST request\n");
    else
      PRINT("Could not send POST request\n");
  } else
    PRINT("Could not init POST request\n");
  currentvol = newvol;
  currentmute = mutestate;
}

static void
set_volume()
{
  int newvol;
  PRINT("Volume level to set (0-100): ");
  SCANF("%d", &newvol);
  PRINT("\n");
  if (newvol < 0 || newvol > 100) {
    PRINT("Invalid volume level: %d\n", newvol);
    return;
  }
  PRINT("Setting volume to %d\n", newvol);
  send_speaker_request(newvol, false);
}

/* TODO */
static void
set_mute()
{
  send_speaker_request(currentvol, false);
}

static void
set_unmute()
{
  send_speaker_request(currentvol, true);
}

static void
display_device_uuid(void)
{
  char buffer[OC_UUID_LEN];
  oc_uuid_to_str(oc_core_get_device_id(0), buffer, sizeof(buffer));
  PRINT("Device UUID: %s\n", buffer);
}

int
main(void)
{
  int init;
  struct sigaction sa;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);

  static const oc_handler_t handler = {.init = app_init,
                                       .signal_event_loop = signal_event_loop,
                                       .requests_entry = issue_requests };

#ifdef OC_SECURITY
  oc_storage_config("./speaker_client_creds");
#endif               /* OC_SECURITY */

  init = oc_main_init(&handler);
  if (init < 0)
    return init;
  display_device_uuid();
  if (pthread_create(&event_loop_thread, NULL, &ocf_event_thread, NULL) != 0) {
    return -1;
  }

  int c;
  while (quit != 1) {
    display_menu();
    SCANF("%d", &c);

    switch (c) {
      case 1:
        set_volume();
        break;
      case 2:
        set_mute();
        break;
      case 3:
        set_unmute();
        break;
      case 0:
        handle_signal(0);
        break;
    }
  }

  pthread_join(event_loop_thread, NULL);

  if (speaker_server != NULL) {
    oc_free_server_endpoints(speaker_server);
  }
  oc_main_shutdown();
  return 0;
}
