/*
 * Audacity: A Digital Audio Editor
 */

#include <gtest/gtest.h>

#include <QUrl>

#include "appfactory.h"
#include "shared/productprofile.h"
#include "framework/stubs/network/networkmanagerstub.h"

namespace au::app {
namespace {

TEST(workbench_module_policy, disables_every_non_diagnostic_service)
{
    const WorkbenchModulePolicy policy = modulePolicy();

    if constexpr (shared::isWorkbenchProfile()) {
        EXPECT_TRUE(policy.diagnostics);
        EXPECT_FALSE(policy.genericNetwork);
        EXPECT_FALSE(policy.cloud);
        EXPECT_FALSE(policy.museHub);
        EXPECT_FALSE(policy.usageInfo);
        EXPECT_FALSE(policy.update);
    } else {
#ifdef MUSE_MODULE_DIAGNOSTICS
        EXPECT_TRUE(policy.diagnostics);
#else
        EXPECT_FALSE(policy.diagnostics);
#endif
#ifdef MUSE_MODULE_NETWORK
        EXPECT_TRUE(policy.genericNetwork);
#else
        EXPECT_FALSE(policy.genericNetwork);
#endif
#ifdef MUSE_MODULE_CLOUD
        EXPECT_TRUE(policy.cloud);
#else
        EXPECT_FALSE(policy.cloud);
#endif
#ifdef AU_BUILD_MUSEHUB_MODULE
        EXPECT_TRUE(policy.museHub);
#else
        EXPECT_FALSE(policy.museHub);
#endif
#ifdef AU_BUILD_USAGEINFO_MODULE
        EXPECT_TRUE(policy.usageInfo);
#else
        EXPECT_FALSE(policy.usageInfo);
#endif
#ifdef MUSE_MODULE_UPDATE
        EXPECT_TRUE(policy.update);
#else
        EXPECT_FALSE(policy.update);
#endif
    }
}

TEST(workbench_network_stub, rejects_all_ordinary_http_requests)
{
    muse::network::NetworkManagerStub network;
    const QUrl url("https://example.invalid/workbench-network-must-be-stubbed");
    const muse::network::OutgoingDeviceVar noOutgoingData = muse::network::NoOutgoingDevice {};
    const int notSupported = static_cast<int>(muse::Ret::Code::NotSupported);

    EXPECT_EQ(network.get(url, nullptr).ret.code(), notSupported);
    EXPECT_EQ(network.post(url, noOutgoingData, nullptr).ret.code(), notSupported);
    EXPECT_EQ(network.put(url, noOutgoingData, nullptr).ret.code(), notSupported);
    EXPECT_EQ(network.patch(url, noOutgoingData, nullptr).ret.code(), notSupported);
    EXPECT_EQ(network.del(url, nullptr).ret.code(), notSupported);
    EXPECT_TRUE(modulePolicy().diagnostics);
}

} // namespace
} // namespace au::app
