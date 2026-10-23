#ifndef GODOT_LIVEKIT_FFI_RAW_H
#define GODOT_LIVEKIT_FFI_RAW_H

// Requests the LiveKit FFI (liblivekit_ffi, the Rust core under the C++ SDK) understands but the C++ SDK does not
// wrap yet: the subscriber's simulcast layer and the size a video is drawn at. Each is one small protobuf message,
// encoded here by hand (two varint fields inside one length-delimited field of FfiRequest) and sent straight to
// livekit_ffi_request -- the same entry point the C++ SDK's FfiClient uses, in the same process.
//
// Field numbers from livekit/rust-sdks livekit-ffi/protocol: ffi.proto (FfiRequest) and track_publication.proto.

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" {
uint64_t livekit_ffi_request(const uint8_t *data, size_t len, const uint8_t **res_ptr, size_t *res_len);
bool livekit_ffi_drop_handle(uint64_t handle);
}

namespace godot_livekit_ffi {

// FfiRequest fields
constexpr uint32_t REQ_UPDATE_REMOTE_TRACK_PUBLICATION_DIMENSION = 43;
constexpr uint32_t REQ_SET_REMOTE_TRACK_PUBLICATION_QUALITY = 68;

inline void put_varint(std::vector<uint8_t> &out, uint64_t v) {
    while (v >= 0x80) {
        out.push_back(static_cast<uint8_t>(v | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<uint8_t>(v));
}

inline void put_varint_field(std::vector<uint8_t> &out, uint32_t field, uint64_t v) {
    put_varint(out, (static_cast<uint64_t>(field) << 3) | 0); // wire type 0: varint
    put_varint(out, v);
}

// Send FfiRequest{ <field>: <inner> } and drop the reply. Returns false if the FFI gave no reply.
inline bool send(uint32_t field, const std::vector<uint8_t> &inner) {
    std::vector<uint8_t> msg;
    put_varint(msg, (static_cast<uint64_t>(field) << 3) | 2); // wire type 2: length-delimited
    put_varint(msg, inner.size());
    msg.insert(msg.end(), inner.begin(), inner.end());
    const uint8_t *res = nullptr;
    size_t res_len = 0;
    uint64_t h = livekit_ffi_request(msg.data(), msg.size(), &res, &res_len);
    if (h != 0) {
        livekit_ffi_drop_handle(h);
    }
    return h != 0;
}

// SetRemoteTrackPublicationQualityRequest { track_publication_handle = 1; quality = 2 (0 low, 1 medium, 2 high) }
inline bool set_quality(uint64_t publication_handle, int quality) {
    std::vector<uint8_t> inner;
    put_varint_field(inner, 1, publication_handle);
    put_varint_field(inner, 2, static_cast<uint64_t>(quality));
    return send(REQ_SET_REMOTE_TRACK_PUBLICATION_QUALITY, inner);
}

// UpdateRemoteTrackPublicationDimensionRequest { track_publication_handle = 1; width = 2; height = 3 }
inline bool set_dimension(uint64_t publication_handle, uint32_t width, uint32_t height) {
    std::vector<uint8_t> inner;
    put_varint_field(inner, 1, publication_handle);
    put_varint_field(inner, 2, width);
    put_varint_field(inner, 3, height);
    return send(REQ_UPDATE_REMOTE_TRACK_PUBLICATION_DIMENSION, inner);
}

} // namespace godot_livekit_ffi

#endif // GODOT_LIVEKIT_FFI_RAW_H
