#ifndef ORBIT_CLIENT_DATA_API_SCOPE_ID_H_
#define ORBIT_CLIENT_DATA_API_SCOPE_ID_H_

#include "OrbitBase/Typedef.h"

namespace orbit_client_data {

struct ScopeIdTag {};

using ScopeId = orbit_base::Typedef<ScopeIdTag, const uint64_t>;

static_assert(sizeof(ScopeId) == sizeof(uint64_t), "orbit_base::Typedef is not zero-cost");

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_API_SCOPE_ID_H_