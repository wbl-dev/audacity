/*
* Audacity: A Digital Audio Editor
*/

#include "au3labelsimporter.h"

#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

#include "au3-track/Track.h"
#include "au3-label-track/LabelTrack.h"

#include "au3wrap/au3types.h"
#include "au3wrap/internal/wxtypes_convert.h"
#include "au3wrap/internal/domconverter.h"

using namespace au::au3;
using namespace au::importexport;

muse::Ret Au3LabelsImporter::importData(const muse::io::path_t& filePath)
{
    const QFileInfo info(filePath.toQString());
    const auto profile = m_registry.parserForSuffix(info.suffix());
    if (!profile) {
        return muse::make_ret(muse::Ret::Code::NotSupported, "No label parser for " + filePath.toStdString());
    }
    QFile file(filePath.toQString());
    if (!file.open(QIODevice::ReadOnly)) {
        return muse::make_ret(muse::Ret::Code::BadData, "Could not open " + filePath.toStdString());
    }
    if (file.size() > 4 * 1024 * 1024) {
        return muse::make_ret(muse::Ret::Code::BadData, "File exceeds 4 MiB limit: " + filePath.toStdString());
    }
    const QByteArray bytes = file.read(4 * 1024 * 1024 + 1);
    if (bytes.size() > 4 * 1024 * 1024 || !file.atEnd()) {
        return muse::make_ret(muse::Ret::Code::BadData, "File exceeds 4 MiB limit: " + filePath.toStdString());
    }
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString text = decoder.decode(bytes);
    if (decoder.hasError()) {
        return muse::make_ret(muse::Ret::Code::BadData, "File is not UTF-8: " + filePath.toStdString());
    }
    const LabelScriptInput input { info.fileName(), info.suffix().toLower(), text };
    const auto parsed = m_engine.parse(profile->source, input);
    if (!parsed.ret) {
        return parsed.ret;
    }

    const auto currentProject = globalContext()->currentProject();
    if (!currentProject) {
        return muse::make_ret(muse::Ret::Code::InternalError);
    }
    Au3Project* project = reinterpret_cast<Au3Project*>(currentProject->au3ProjectPtr());
    IF_ASSERT_FAILED(project) {
        return muse::make_ret(muse::Ret::Code::InternalError);
    }

    auto& tracks = Au3TrackList::Get(*project);
    Au3LabelTrack* labelTrack = ::LabelTrack::Create(tracks, wxFromString(muse::String::fromQString(parsed.val.trackName)));
    for (const auto& label : parsed.val.labels) {
        SelectedRegion region(label.start, label.end);
        region.setFrequencies(label.lowFrequency.value_or(-1.0), label.highFrequency.value_or(-1.0));
        ::LabelStruct value(region, wxFromString(muse::String::fromQString(label.title)));
        value.SetKind(label.kind == ScriptLabelKind::Point ? ::LabelKind::Point : ::LabelKind::Range);
        labelTrack->AddLabel(value);
    }

    // Notify project about the new track
    const auto prj = globalContext()->currentTrackeditProject();
    if (prj) {
        prj->notifyAboutTrackAdded(DomConverter::labelTrack(labelTrack));

        // Notify about each imported label
        const auto& labels = labelTrack->GetLabels();
        for (size_t i = 0; i < labels.size(); ++i) {
            prj->notifyAboutLabelAdded(DomConverter::label(labelTrack, &labels[i]));
        }
    }

    return muse::make_ret(muse::Ret::Code::Ok);
}

std::vector<std::string> Au3LabelsImporter::supportedExtensions() const
{
    return m_registry.supportedExtensions();
}
