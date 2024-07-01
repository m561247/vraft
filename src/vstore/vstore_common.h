#ifndef VSTORE_VSTORE_COMMON_H_
#define VSTORE_VSTORE_COMMON_H_

#include <memory>

namespace vstore {

class Vstore;
using VstoreSPtr = std::shared_ptr<Vstore>;
using VstoreUPtr = std::unique_ptr<Vstore>;
using VstoreWPtr = std::weak_ptr<Vstore>;

}  // namespace vstore

#endif
