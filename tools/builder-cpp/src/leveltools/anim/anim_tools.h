// The animation tools of the level conversion as functions. Each takes a level, the offsets of its lists
// (pc_sections() for a PC level) and the bytes per moveable (18 PC, 20 PSX), and returns the changed level,
// or nothing when the level has the change already.
#pragma once
#include <optional>
#include <utility>
#include <vector>

#include "anim_common.h"

namespace anim {

inline constexpr unsigned CRAWL_FLIP_INDEX = 313, CRAWL_FLIP_BASE = 32000;
inline constexpr unsigned CROUCH_TURN_INDEX = 314, CROUCH_TURN_BASE = 32100;
inline constexpr unsigned CRAWL_PICKUP_INDEX = 292;
inline constexpr unsigned CROUCH_ROLL_ANIM = 220;
inline constexpr unsigned CONTROLLED_DROP_INDEX = 316, CONTROLLED_DROP_BASE = 32200;
inline constexpr unsigned LEDGE_JUMPS_INDEX = 318, LEDGE_JUMPS_BASE = 32220;
inline constexpr unsigned CORNER_SHIMMY_INDEX = 322, CORNER_SHIMMY_BASE = 32292;
inline constexpr unsigned CLIMB_CORNERS_INDEX = 330, CLIMB_CORNERS_BASE = 31000;

std::optional<Bytes> add_crawl_flip(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_crouch_turn(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_crawl_pickup(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> fix_crouch_roll_arm(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_controlled_drop(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_ledge_jumps(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_corner_shimmy(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);
std::optional<Bytes> add_climb_corners(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);

// s needs the item list (pc_sections(d, true)). report: the animations freed (or to be freed).
struct StripReport {
    std::vector<unsigned> anims;
};
std::optional<Bytes> strip_dead_anims(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE,
                                      StripReport *report = nullptr);

// report: bytes removed from the frames and the mesh trees, pose blocks shared.
struct CompactReport {
    size_t frames = 0, shared = 0, trees = 0;
};
std::optional<Bytes> compact_frames(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE,
                                    CompactReport *report = nullptr);

std::optional<Bytes> pack_rotations(const Bytes &d, const Sections &s, size_t stride = PC_STRIDE);

// compact_frames.py's read(): the animation heads, the moveables, the start of the frame data, its size in
// bytes and per animation the bytes of its poses (offset, length).
struct FrameLayout {
    std::vector<AnimHead> anims;
    std::vector<Moveable> movs;
    size_t frames = 0, size = 0;
    std::vector<std::pair<size_t, size_t>> spans;
};
FrameLayout read_frames(const Bytes &d, const Sections &s, size_t stride);

}  // namespace anim
