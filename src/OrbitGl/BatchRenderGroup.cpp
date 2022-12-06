#include "OrbitGl/BatchRenderGroup.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

namespace BatchRenderGroupManager {
absl::flat_hash_map<BatchRenderGroupId, uint64_t> creation_order_index;
absl::flat_hash_map<BatchRenderGroupId, BatchRenderGroupState> group_states;
uint64_t current_creation_order_index = 0;
uint64_t frame_start_index = 0;

inline bool HasValidCreationIndex(const BatchRenderGroupId& id) {
  return creation_order_index.contains(id) && (creation_order_index[id] >= frame_start_index);
}

inline uint64_t GetCreationOrderIndex(const BatchRenderGroupId& id) {
  uint64_t index = BatchRenderGroupManager::creation_order_index.contains(id)
                       ? BatchRenderGroupManager::creation_order_index.at(id)
                       : BatchRenderGroupManager::frame_start_index;
  if (index < frame_start_index) {
    index = frame_start_index;
  }

  return index;
}

void ResetOrdering() { frame_start_index = ++current_creation_order_index; }

void TouchId(const BatchRenderGroupId& id) {
  if (!HasValidCreationIndex(id)) {
    creation_order_index[id] = ++current_creation_order_index;
  }
}

[[nodiscard]] BatchRenderGroupState GetGroupState(const BatchRenderGroupId& id) {
  return group_states[id];
}

void SetGroupState(const BatchRenderGroupId& id, BatchRenderGroupState state) {
  group_states[id] = std::move(state);
}
}  // namespace BatchRenderGroupManager

BatchRenderGroupId::BatchRenderGroupId(const std::string& name, float layer)
    : name(name), layer(layer) {}

const std::string BatchRenderGroupId::kGlobalGroup = "global";

bool operator==(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return lhs.layer == rhs.layer && lhs.name == rhs.name;
}

bool operator!=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs == rhs);
}

bool operator<(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  if (lhs.layer != rhs.layer) return lhs.layer < rhs.layer;

  uint64_t lhs_creation = BatchRenderGroupManager::GetCreationOrderIndex(lhs);
  uint64_t rhs_creation = BatchRenderGroupManager::GetCreationOrderIndex(rhs);
  return lhs_creation < rhs_creation;
}

bool operator<=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  if (lhs < rhs) return true;

  uint64_t lhs_creation = BatchRenderGroupManager::GetCreationOrderIndex(lhs);
  uint64_t rhs_creation = BatchRenderGroupManager::GetCreationOrderIndex(rhs);
  return (lhs.layer == rhs.layer && lhs_creation == rhs_creation);
}

bool operator>(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs <= rhs);
}

bool operator>=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs < rhs);
}

}  // namespace orbit_gl