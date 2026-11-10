#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "importexport/labels/ilabelsconfiguration.h"

namespace au::appshell {
class LabelParsersPreferencesModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    muse::GlobalInject<importexport::ILabelsConfiguration> configuration;

public:
    explicit LabelParsersPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList profiles() const;
    Q_INVOKABLE QVariantMap save(const QString& id, const QString& name, const QString& suffixes, bool enabled, const QString& script,
                                 bool allowBuiltinOverride = false);
    Q_INVOKABLE QVariantMap remove(const QString& id);
    Q_INVOKABLE QVariantMap test(const QString& script, const QString& samplePath) const;
};
}
