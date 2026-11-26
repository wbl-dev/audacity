/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "au3wrap/au3types.h"
#include "trackedit/dom/track.h"

#include "au3-track/Track.h"

namespace agu::au3 {
class TrackHeightAttachment : public TrackAttachment
{
public:
    static TrackHeightAttachment& Get(const Au3Track* track);
    static TrackHeightAttachment& Get(Au3Track* track);

    TrackHeightAttachment(Au3Track& track);
    void Reparent(const std::shared_ptr<Au3Track>& parent) override;
    void CopyTo(Au3Track& track) const override;
    void WriteXMLAttributes(XMLWriter& writer) const override;
    bool HandleXMLAttribute(const std::string_view& attr, const XMLAttributeValueView& valueView) override;

    int GetHeight() const;
    void SetHeight(int height);

    int GetSingleViewHeight() const;
    void SetSingleViewHeight(int height);
    int GetMultiViewHeight() const;
    void SetMultiViewHeight(int height);

    double GetMultiViewHeightRatio() const;
    void SetMultiViewHeightRatio(double ratio);

private:
    std::weak_ptr<Au3Track> mTrack;
    double mMultiViewHeightRatio{ 0.5 };
    int mHeight{ 0 };  // 0 means use default height
    int mSingleViewHeight{ 0 };
    int mMultiViewHeight{ 0 };
};
}
