#include "oc_api.h"
#include "oc_diplomat.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

static pthread_mutex_t mutex;
static pthread_cond_t cv;
static pthread_t event_loop_thread;
static struct timespec ts;
static oc_resource_t *res = NULL;

static int quit = 0;

static void
poll_for_so_info(void)
{
  OC_DBG("Polling for Streamlined Onboarding info via CLI\n");
  while (quit != 1) {
    PRINT("Enter Base64-encoded Streamlined Onboarding information: ");
    char *info_input = NULL;
    int input_len;
    if (scanf("%ms%n", &info_input, &input_len) < 1) {
      OC_ERR("Failed to read input\n");
      continue;
    }
    OC_DBG("Read %d characters\n", input_len);
    OC_DBG("Read input: %s\n", info_input);
    if (info_input)
      free(info_input);
  }
}

static void
*ocf_event_thread(void *data)
{
  (void)data;
  oc_clock_time_t next_event;
  while (quit != 1) {
    next_event = oc_main_poll();

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

static void
register_resources(void)
{
  res = oc_new_resource(NULL, "/diplomat", 1, 0);
  oc_resource_bind_resource_type(res, "oic.r.diplomat");
  oc_resource_bind_resource_interface(res, OC_IF_R);
  oc_resource_set_default_interface(res, OC_IF_R);
  oc_resource_set_discoverable(res, true);
  oc_resource_set_observable(res, true);
  oc_resource_set_request_handler(res, OC_GET, oc_diplomat_get, NULL);
  oc_add_resource(res);
}

static int
app_init(void)
{
  int ret = oc_init_platform("Linux", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.diplomat", "CLI Diplomat", "ocf.1.0.0",
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

int
main(void)
{
  int init;
  struct sigaction sa;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handle_signal;
  sigaction(SIGINT, &sa, NULL);

  static const oc_handler_t handler = { .init = app_init,
                                        .signal_event_loop = signal_event_loop,
                                        .register_resources = register_resources };

#ifdef OC_STORAGE
  oc_storage_config("./cli_diplomat_creds");
#endif /* OC_STORAGE */

  init = oc_main_init(&handler);
  if (init < 0)
    return init;

  if (pthread_create(&event_loop_thread, NULL, &ocf_event_thread, NULL) != 0) {
    return -1;
  }

  poll_for_so_info();

  pthread_join(event_loop_thread, NULL);
  oc_main_shutdown();
  oc_diplomat_teardown();
  return 0;
}
