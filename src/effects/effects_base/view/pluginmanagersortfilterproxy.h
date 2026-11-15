/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "uicomponents/qml/AGu/UiComponents/tablesortfilterproxymodel.h"

namespace agu::effects {
class PluginManagerTableViewModel;

class PluginManagerSortFilterProxy : public agu::uicomponents::TableSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PluginManagerSortFilterProxy(PluginManagerTableViewModel* view);

protected:
    bool acceptsRow(int sourceRow) const override;
    int compareCells(int column, int leftSourceRow, int rightSourceRow) const override;

private:
    PluginManagerTableViewModel* m_view = nullptr;
};
}
