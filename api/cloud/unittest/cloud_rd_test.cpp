/******************************************************************
 *
 * Copyright 2019 Jozef Kralik All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <gtest/gtest.h>

#include "oc_api.h"
#include "oc_cloud_internal.h"
#include "oc_collection.h"

class TestCloudRD : public testing::Test {
public:
  static oc_handler_t s_handler;
  static pthread_mutex_t s_mutex;
  static pthread_cond_t s_cv;

  static void onPostResponse(oc_client_response_t *) {}

  static int appInit(void)
  {
    int result = oc_init_platform("OCFCloud", nullptr, nullptr);
    result |= oc_add_device("/oic/d", "oic.d.light", "Lamp", "ocf.1.0.0",
                            "ocf.res.1.0.0", nullptr, nullptr);
    return result;
  }

  static void signalEventLoop(void) { pthread_cond_signal(&s_cv); }

  static oc_event_callback_retval_t quitEvent(void *data)
  {
    bool *quit = static_cast<bool *>(data);
    *quit = true;
    return OC_EVENT_DONE;
  }

  static void poolEvents(uint16_t seconds)
  {
    bool quit = false;
    oc_set_delayed_callback(&quit, quitEvent, seconds);

    while (true) {
      pthread_mutex_lock(&s_mutex);
      oc_clock_time_t next_event = oc_main_poll();
      if (quit) {
        pthread_mutex_unlock(&s_mutex);
        break;
      }
      if (next_event == 0) {
        pthread_cond_wait(&s_cv, &s_mutex);
      } else {
        struct timespec ts;
        ts.tv_sec = (next_event / OC_CLOCK_SECOND);
        ts.tv_nsec = (next_event % OC_CLOCK_SECOND) * 1.e09 / OC_CLOCK_SECOND;
        pthread_cond_timedwait(&s_cv, &s_mutex, &ts);
      }
      pthread_mutex_unlock(&s_mutex);
    }
  }

protected:
  static void SetUpTestCase()
  {
    s_handler.init = &appInit;
    s_handler.signal_event_loop = &signalEventLoop;
    int ret = oc_main_init(&s_handler);
    ASSERT_EQ(0, ret);
  }

  static oc_resource_t *findResource(oc_link_t *head, oc_resource_t *res)
  {
    for (oc_link_t *l = head; l; l = l->next) {
      if (l->resource == res) {
        return l->resource;
      }
    }
    return nullptr;
  }

  static void TearDownTestCase() { oc_main_shutdown(); }
};

oc_handler_t TestCloudRD::s_handler;
pthread_mutex_t TestCloudRD::s_mutex;
pthread_cond_t TestCloudRD::s_cv;

TEST_F(TestCloudRD, cloud_publish_f)
{
  // When
  int ret = oc_cloud_add_resource(nullptr);

  // Then
  ASSERT_EQ(-1, ret);
}

TEST_F(TestCloudRD, cloud_publish_p)
{
  // When
  oc_resource_t *res1 = oc_new_resource(nullptr, "/light/1", 1, 0);
  oc_resource_bind_resource_type(res1, "test");
  int ret = oc_cloud_add_resource(res1);

  // Then
  ASSERT_EQ(0, ret);
  oc_cloud_context_t *ctx = oc_cloud_get_context(0);
  ASSERT_NE(nullptr, ctx);
  ASSERT_NE(nullptr, ctx->rd_publish_resources);
  EXPECT_EQ(res1, findResource(ctx->rd_publish_resources, res1));

  // Clean-up
  oc_cloud_delete_resource(res1);
  EXPECT_TRUE(oc_delete_resource(res1));
}

TEST_F(TestCloudRD, cloud_delete)
{
  // When
  oc_resource_t *res1 = oc_new_resource(nullptr, "/light/1", 1, 0);
  oc_resource_bind_resource_type(res1, "test");
  int ret = oc_cloud_add_resource(res1);
  ASSERT_EQ(0, ret);
  oc_cloud_delete_resource(res1);

  // Then
  oc_cloud_context_t *ctx = oc_cloud_get_context(0);
  ASSERT_NE(nullptr, ctx);
  EXPECT_EQ(nullptr, findResource(ctx->rd_publish_resources, res1));

  // Clean-up
  EXPECT_TRUE(oc_delete_resource(res1));
}
