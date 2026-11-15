/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

namespace agu::effects {
struct DynamicsSample
{
    double time = 0;
    double inputDb = 0;
    double outputDb = 0;
    double compressionDb = 0;
};
}
