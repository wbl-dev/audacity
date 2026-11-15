/*
 * Audacity: A Digital Audio Editor
 */
#include "pluginmanagertableviewverticalheader.h"

namespace agu::effects {
PluginManagerTableViewVerticalHeader::PluginManagerTableViewVerticalHeader(QObject* parent)
    : muse::uicomponents::TableViewHeader(parent)
{
}

const EffectId& PluginManagerTableViewVerticalHeader::effectId() const
{
    return m_effectId;
}

void PluginManagerTableViewVerticalHeader::setEffectId(const EffectId& effectId)
{
    m_effectId = effectId;
}
}
