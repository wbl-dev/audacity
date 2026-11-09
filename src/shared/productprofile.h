/*
 * Audacity Workbench product policy.
 */
#pragma once

namespace au::shared {
constexpr bool isWorkbenchProfile() noexcept
{
#ifdef AU4_WORKBENCH_PROFILE
    return true;
#else
    return false;
#endif
}

constexpr bool nonDiagnosticExternalLinksAvailable() noexcept
{
    return !isWorkbenchProfile();
}
}
