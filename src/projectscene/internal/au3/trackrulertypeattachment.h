/*
* Audacity: A Digital Audio Editor
*/
#pragma once

#include "au3wrap/au3types.h"
#include "trackedit/dom/track.h"
#include "au3-track/Track.h"

class TrackRulerTypeAttachment : public TrackAttachment
{
public:
    static TrackRulerTypeAttachment& Get(const agu::au3::Au3Track* track);
    static TrackRulerTypeAttachment& Get(agu::au3::Au3Track* track);

    TrackRulerTypeAttachment(agu::au3::Au3Track& track);
    void Reparent(const std::shared_ptr<agu::au3::Au3Track>& parent) override;
    void CopyTo(agu::au3::Au3Track& track) const override;
    void WriteXMLAttributes(XMLWriter& writer) const override;
    bool HandleXMLAttribute(const std::string_view& attr, const XMLAttributeValueView& valueView) override;

    agu::trackedit::TrackRulerType GetRulerType() const;
    void SetRulerType(const agu::trackedit::TrackRulerType& rulerType);

private:
    std::weak_ptr<agu::au3::Au3Track> mTrack;
    agu::trackedit::TrackRulerType mRulerType;
};
