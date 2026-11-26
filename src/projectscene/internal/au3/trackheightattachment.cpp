/*
 * Audacity: A Digital Audio Editor
 */
#include "trackheightattachment.h"

#include <algorithm>
#include <cmath>

namespace agu::au3 {
static const AttachedTrackObjects::RegisteredFactory keyTrackHeight{
    [](Au3Track& track) -> std::shared_ptr<TrackHeightAttachment> { return std::make_shared<TrackHeightAttachment>(track); }
};

static constexpr auto HeightAttr = "height";
static constexpr auto SingleViewHeightAttr = "singleviewheight";
static constexpr auto MultiViewHeightAttr = "multiviewheight";
static constexpr auto MultiViewRatioAttr = "multiviewratio";

TrackHeightAttachment& TrackHeightAttachment::Get(Au3Track* track)
{
    return track->AttachedTrackObjects::Get<TrackHeightAttachment>(keyTrackHeight);
}

TrackHeightAttachment& TrackHeightAttachment::Get(const Au3Track* track)
{
    return Get(const_cast<Au3Track*>(track));
}

TrackHeightAttachment::TrackHeightAttachment(Au3Track& track)
    : mTrack{track.shared_from_this()}
{
}

void TrackHeightAttachment::Reparent(const std::shared_ptr<Au3Track>& parent)
{
    mTrack = parent;
}

void TrackHeightAttachment::CopyTo(Au3Track& track) const
{
    auto& attachment = Get(&track);
    attachment.SetHeight(mHeight);
    attachment.SetSingleViewHeight(mSingleViewHeight);
    attachment.SetMultiViewHeight(mMultiViewHeight);
    attachment.SetMultiViewHeightRatio(mMultiViewHeightRatio);
}

void TrackHeightAttachment::WriteXMLAttributes(XMLWriter& writer) const
{
    writer.WriteAttr(HeightAttr, mHeight);
    writer.WriteAttr(SingleViewHeightAttr, mSingleViewHeight);
    writer.WriteAttr(MultiViewHeightAttr, mMultiViewHeight);
    writer.WriteAttr(MultiViewRatioAttr, mMultiViewHeightRatio);
}

bool TrackHeightAttachment::HandleXMLAttribute(const std::string_view& attr, const XMLAttributeValueView& valueView)
{
    double ratio;
    if (attr == MultiViewRatioAttr && valueView.TryGet(ratio)) {
        SetMultiViewHeightRatio(ratio);
        return true;
    }
    long nValue;
    if (attr == HeightAttr && valueView.TryGet(nValue)) {
        mHeight = static_cast<int>(nValue);
        return true;
    }
    if (attr == SingleViewHeightAttr && valueView.TryGet(nValue)) {
        SetSingleViewHeight(static_cast<int>(nValue));
        return true;
    }
    if (attr == MultiViewHeightAttr && valueView.TryGet(nValue)) {
        SetMultiViewHeight(static_cast<int>(nValue));
        return true;
    }
    return false;
}

int TrackHeightAttachment::GetHeight() const
{
    return mHeight;
}

void TrackHeightAttachment::SetHeight(int height)
{
    mHeight = height;
}

int TrackHeightAttachment::GetSingleViewHeight() const
{
    return mSingleViewHeight;
}

void TrackHeightAttachment::SetSingleViewHeight(int height)
{
    mSingleViewHeight = std::max(0, height);
}

int TrackHeightAttachment::GetMultiViewHeight() const
{
    return mMultiViewHeight;
}

void TrackHeightAttachment::SetMultiViewHeight(int height)
{
    mMultiViewHeight = std::max(0, height);
}

double TrackHeightAttachment::GetMultiViewHeightRatio() const
{
    return mMultiViewHeightRatio;
}

void TrackHeightAttachment::SetMultiViewHeightRatio(double ratio)
{
    if (std::isfinite(ratio)) {
        mMultiViewHeightRatio = std::clamp(ratio, 0.1, 0.9);
    }
}
}
