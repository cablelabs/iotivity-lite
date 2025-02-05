/******************************************************************
 *
 * Copyright 2019 Jozef Kralik All Rights Reserved.
 * Copyright 2018 Samsung Electronics All Rights Reserved.
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

#ifndef OC_SECURITY

#include <gtest/gtest.h>

#include "oc_api.h"
#include "oc_cloud_internal.h"
#include "oc_endpoint.h"

class TestCloudAccess : public testing::Test {
public:
  static oc_handler_t s_handler;
  static oc_endpoint_t s_endpoint;

  static void onPostResponse(oc_client_response_t *) {}

  static int appInit(void)
  {
    int result = oc_init_platform("OCFCloud", nullptr, nullptr);
    result |= oc_add_device("/oic/d", "oic.d.light", "Cloud's Light",
                            "ocf.1.0.0", "ocf.res.1.0.0", nullptr, nullptr);
    return result;
  }

  static void signalEventLoop(void) {}

protected:
  static void SetUpTestCase()
  {
    s_handler.init = &appInit;
    s_handler.signal_event_loop = &signalEventLoop;
    int ret = oc_main_init(&s_handler);
    ASSERT_EQ(0, ret);

    oc_string_t ep_str;
    oc_new_string(&ep_str, "coap://224.0.1.187:5683", 23);
    oc_string_to_endpoint(&ep_str, &s_endpoint, nullptr);
    oc_free_string(&ep_str);
  }

  static void TearDownTestCase() { oc_main_shutdown(); }
};
oc_handler_t TestCloudAccess::s_handler;
oc_endpoint_t TestCloudAccess::s_endpoint;

TEST_F(TestCloudAccess, cloud_access_register_p)
{
  // When
  bool ret =
    cloud_access_register(&s_endpoint, "auth_provider", "auth_code", "uid",
                          "access_token", 0, -1, onPostResponse, nullptr);

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(TestCloudAccess, cloud_access_register_f)
{
  // Given
  oc_endpoint_t *ep = nullptr;

  // When
  bool ret = cloud_access_register(ep, nullptr, nullptr, nullptr, nullptr, 0,
                                   -1, nullptr, nullptr);

  // Then
  EXPECT_FALSE(ret);
}

TEST_F(TestCloudAccess, cloud_access_login_p)
{
  // When
  bool ret = cloud_access_login(&s_endpoint, "uid", "access_token", 0, -1,
                                onPostResponse, nullptr);

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(TestCloudAccess, cloud_access_login_f)
{
  // Given
  oc_endpoint_t *ep = nullptr;

  // When
  bool ret = cloud_access_login(ep, nullptr, nullptr, 0, -1, nullptr, nullptr);

  // Then
  EXPECT_FALSE(ret);
}

TEST_F(TestCloudAccess, cloud_access_logout_p)
{
  // When
  bool ret = cloud_access_logout(&s_endpoint, "uid", "access_token", 0, -1,
                                 onPostResponse, nullptr);

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(TestCloudAccess, cloud_access_logout_f)
{
  // Given
  oc_endpoint_t *ep = nullptr;

  // When
  bool ret = cloud_access_logout(ep, nullptr, nullptr, 0, -1, nullptr, nullptr);

  // Then
  EXPECT_FALSE(ret);
}

TEST_F(TestCloudAccess, cloud_access_refresh_access_token_p)
{
  // When
  bool ret = cloud_access_refresh_access_token(
    &s_endpoint, "uid", "refresh_token", 0, -1, onPostResponse, nullptr);

  // Then
  EXPECT_TRUE(ret);
}

TEST_F(TestCloudAccess, cloud_access_refresh_access_token_f)
{
  // Given
  oc_endpoint_t *ep = nullptr;

  // When
  bool ret = cloud_access_refresh_access_token(ep, nullptr, nullptr, 0, -1,
                                               nullptr, nullptr);

  // Then
  EXPECT_FALSE(ret);
}

#endif /* !OC_SECURITY */
