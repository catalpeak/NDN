
// m-content-store-impl.cc

#include "content-store-change.h"
#include "slru-policy.h"

#include "ns3/ndnSIM/utils/trie/multi-policy.hpp"
#include "ns3/ndnSIM/utils/trie/aggregate-stats-policy.hpp"
#include "lru-policy.hpp"

#define NS_OBJECT_ENSURE_REGISTERED_TEMPL(type, templ)     \
  static struct X##type##templ##RegistrationClass {        \
    X##type##templ##RegistrationClass()                    \
    {                                                      \
      ns3::TypeId tid = type<templ>::GetTypeId();          \
      tid.GetParent();                                     \
    }                                                      \
  } x_##type##templ##RegistrationVariable

namespace ns3 {
namespace ndn {

using namespace ndnSIM;

namespace cs {

template class MContentStoreImpl <mlru_policy_traits>;

NS_OBJECT_ENSURE_REGISTERED_TEMPL (MContentStoreImpl, mlru_policy_traits);

typedef multi_policy_traits<boost::mpl::vector2<mlru_policy_traits, aggregate_stats_policy_traits>>
  MlruWithCountsTraits;

template class MContentStoreImpl <MlruWithCountsTraits>;
NS_OBJECT_ENSURE_REGISTERED_TEMPL (MContentStoreImpl, MlruWithCountsTraits);

#ifdef DOXYGEN

class mlru : public MContentStoreImpl <mlru_policy_traits> {

};

#endif

} // namespace cs
} // namespace ndn
} // namespace ns3

