
// slru-policy.h

#ifndef SLRU_POLICY_H
#define SLRU_POLICY_H

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/options.hpp>

#include <boost/foreach.hpp>

#include "utils/MineTime.h"

#include <iostream>

using namespace std;

namespace ns3 {
namespace ndn {
namespace ndnSIM {

struct slru_policy_traits {

  static std::string
  GetName () {
    return "slru";
  }

  struct policy_hook_type : public boost::intrusive::list_member_hook <> {
  };

  template <class Container>
  struct container_hook {
    typedef boost::intrusive::member_hook<Container, policy_hook_type, &Container::policy_hook_>
      type;
  };

  template <class Base, class Container, class Hook>
  struct policy {
    typedef typename boost::intrusive::list <Container, Hook> policy_container;

    class type : public policy_container {
    public :
      typedef Container parent_trie;

      type (Base &base)
        : base_ (base)
        , max_size_  (100)
        , prePeriod  (0.0)
        , periodTime (1000.0)
        , NUMENTRY   (0.0)
      {
      }

      // @brief never used
      inline void
      update (typename parent_trie::iterator item) {
        policy_container::splice ( policy_container::end ()
                                 , *this
                                 , policy_container::s_iterator_to (*item));
      }

      inline bool
      insert (typename parent_trie::iterator item) {

        if (policy_container::size () >= (max_size_ / 2.0)) {
          double icount = 0.0;
          for ( typename policy_container::const_iterator ientry = policy_container::begin ()
              ; ientry != policy_container::end (); icount += 1.0) {
            if (icount == max_size_ / 2.0) {
              policy_container::erase (ientry++);
              break;
            } else {
              ++ientry;
            }
          }
        }
        policy_container::push_front (*item);
        return true;
      }

      inline void
      lookup (typename parent_trie::iterator item) {
        for ( typename policy_container::const_iterator ientry = policy_container::begin ()
            ; ientry != policy_container::end (); ientry++){
          if (ientry == policy_container::s_iterator_to (*item)) {
            // This Data is hit
            policy_container::splice (ientry, *this, policy_container::end ()); 
            break;
          }
        }
      }

      inline void 
      erase (typename parent_trie::iterator item) {
        policy_container::erase (policy_container::s_iterator_to (*item));
      }

      inline void
      clear () {
        policy_container::clear ();
      }

      inline void
      set_max_size (size_t max_size) {
        max_size_ = max_size;
      }

      inline size_t
      get_max_size () const {
        return max_size_;
      }

    private :

      type ()
        : base_ (*((Base*)0)) 
        , prePeriod (0.0)
        , periodTime (1000.0)
        , NUMENTRY   (0.0) {};

      bool IfAPeriod () {
        if (time.GetNowTimeMS () - prePeriod >= periodTime) {
          prePeriod = time.GetNowTimeMS ();
          return true;
        } else {
          return false;
        }
      }
    private :

      Base& base_;

      size_t max_size_;

      ns3::ndn::MineTime time;

      double prePeriod;

      double periodTime;

      double NUMENTRY;

    };
  };

};

} // namespace ndnSIM
} // namespace ndn
} // namespace ns3

#endif // SLRU_POLICY_H
