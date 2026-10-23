#include "livekit_track_publication.h"
#include "livekit_ffi_raw.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// LiveKitTrackPublication

void LiveKitTrackPublication::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_sid"), &LiveKitTrackPublication::get_sid);
    ClassDB::bind_method(D_METHOD("get_name"), &LiveKitTrackPublication::get_name);
    ClassDB::bind_method(D_METHOD("get_kind"), &LiveKitTrackPublication::get_kind);
    ClassDB::bind_method(D_METHOD("get_source"), &LiveKitTrackPublication::get_source);
    ClassDB::bind_method(D_METHOD("get_muted"), &LiveKitTrackPublication::get_muted);
    ClassDB::bind_method(D_METHOD("get_mime_type"), &LiveKitTrackPublication::get_mime_type);
    ClassDB::bind_method(D_METHOD("get_simulcasted"), &LiveKitTrackPublication::get_simulcasted);
    ClassDB::bind_method(D_METHOD("get_track"), &LiveKitTrackPublication::get_track);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sid"), "", "get_sid");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "kind"), "", "get_kind");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "source"), "", "get_source");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "muted"), "", "get_muted");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "mime_type"), "", "get_mime_type");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulcasted"), "", "get_simulcasted");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "track", PROPERTY_HINT_RESOURCE_TYPE, "LiveKitTrack"), "", "get_track");
}

LiveKitTrackPublication::LiveKitTrackPublication() {
}

LiveKitTrackPublication::~LiveKitTrackPublication() {
}

void LiveKitTrackPublication::bind_publication(const std::shared_ptr<livekit::TrackPublication> &pub) {
    publication_ = pub;
}

String LiveKitTrackPublication::get_sid() const {
    if (publication_) {
        return String(publication_->sid().c_str());
    }
    return String();
}

String LiveKitTrackPublication::get_name() const {
    if (publication_) {
        return String(publication_->name().c_str());
    }
    return String();
}

int LiveKitTrackPublication::get_kind() const {
    if (publication_) {
        return (int)publication_->kind();
    }
    return 0;
}

int LiveKitTrackPublication::get_source() const {
    if (publication_) {
        return (int)publication_->source();
    }
    return 0;
}

bool LiveKitTrackPublication::get_muted() const {
    if (publication_) {
        return publication_->muted();
    }
    return false;
}

String LiveKitTrackPublication::get_mime_type() const {
    if (publication_) {
        return String(publication_->mimeType().c_str());
    }
    return String();
}

bool LiveKitTrackPublication::get_simulcasted() const {
    if (publication_) {
        return publication_->simulcasted();
    }
    return false;
}

Ref<LiveKitTrack> LiveKitTrackPublication::get_track() const {
    if (publication_) {
        auto native_track = publication_->track();
        if (native_track) {
            Ref<LiveKitTrack> track;
            track.instantiate();
            track->bind_track(native_track);
            return track;
        }
    }
    return Ref<LiveKitTrack>();
}

// LiveKitLocalTrackPublication

void LiveKitLocalTrackPublication::_bind_methods() {
}

LiveKitLocalTrackPublication::LiveKitLocalTrackPublication() {
}

LiveKitLocalTrackPublication::~LiveKitLocalTrackPublication() {
}

// LiveKitRemoteTrackPublication

void LiveKitRemoteTrackPublication::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_subscribed"), &LiveKitRemoteTrackPublication::get_subscribed);
    ClassDB::bind_method(D_METHOD("set_subscribed", "subscribed"), &LiveKitRemoteTrackPublication::set_subscribed);

    ClassDB::bind_method(D_METHOD("set_video_quality", "quality"), &LiveKitRemoteTrackPublication::set_video_quality);
    ClassDB::bind_method(D_METHOD("set_video_dimensions", "size"), &LiveKitRemoteTrackPublication::set_video_dimensions);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "subscribed"), "set_subscribed", "get_subscribed");

    BIND_ENUM_CONSTANT(VIDEO_QUALITY_LOW);
    BIND_ENUM_CONSTANT(VIDEO_QUALITY_MEDIUM);
    BIND_ENUM_CONSTANT(VIDEO_QUALITY_HIGH);
}

LiveKitRemoteTrackPublication::LiveKitRemoteTrackPublication() {
}

LiveKitRemoteTrackPublication::~LiveKitRemoteTrackPublication() {
}

bool LiveKitRemoteTrackPublication::get_subscribed() const {
    if (publication_) {
        auto remote_pub = std::dynamic_pointer_cast<livekit::RemoteTrackPublication>(publication_);
        if (remote_pub) {
            return remote_pub->subscribed();
        }
    }
    return false;
}

void LiveKitRemoteTrackPublication::set_subscribed(bool subscribed) {
    if (publication_) {
        auto remote_pub = std::dynamic_pointer_cast<livekit::RemoteTrackPublication>(publication_);
        if (remote_pub) {
            remote_pub->setSubscribed(subscribed);
        }
    }
}

// Which simulcast layer the server sends this subscriber (LOW is the publisher's smallest -- a tile's worth). Only a
// simulcasted video track has layers; on any other it does nothing.
bool LiveKitRemoteTrackPublication::set_video_quality(int quality) {
    if (!publication_ || quality < VIDEO_QUALITY_LOW || quality > VIDEO_QUALITY_HIGH) {
        return false;
    }
    return godot_livekit_ffi::set_quality(publication_->ffiHandleId(), quality);
}

// The size this video is drawn at, in pixels: the server picks the smallest layer that covers it (and sends nothing
// bigger), and follows it as the tile grows or shrinks.
bool LiveKitRemoteTrackPublication::set_video_dimensions(const Vector2i &size) {
    if (!publication_ || size.x <= 0 || size.y <= 0) {
        return false;
    }
    return godot_livekit_ffi::set_dimension(publication_->ffiHandleId(), (uint32_t)size.x, (uint32_t)size.y);
}
