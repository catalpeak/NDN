// Srand.cc

#include "Srand.h"

namespace ns3 {
namespace ndn {

Srand::Srand (double up, double low) {
  if (up < low) {
    cout << "A Wrong Srand Create !!!" << endl;
  }
  UP  = (int) up;
  LOW = (int) low;
  srand ((unsigned) time (NULL));
  random = ( rand () % (int)(up - low + 1.0) ) + low;
}

Srand::Srand (int up   , double low) {
  if (up < low) {
    cout << "A Wrong Srand Create !!!" << endl;
  }
  UP  = (int) up;
  LOW = (int) low;
  srand ((unsigned) time (NULL));
  random = ( rand () % (up - (int)low + 1) ) + (int)low;
}

Srand::Srand (double up, int low)    {
  if (up < low) {
    cout << "A Wrong Srand Create !!!" << endl;
  }
  UP  = (int) up;
  LOW = (int) low;
  srand ((unsigned) time (NULL));
  random = ( rand () % ((int)up - low + 1) ) + low;
}

Srand::Srand (int up   , int low)    {
  if (up < low) {
    cout << "A Wrong Srand Create !!!" << endl;
  }
  UP  = (int) up;
  LOW = (int) low;
  srand ((unsigned) time (NULL));
  random = ( rand () % (up - low + 1) ) + low;
}

Srand::Srand () {
//  srand ((unsigned) time (NULL));
  UP    = 0;
  LOW   = 0;
  random = 0;
}

int 
Srand::GetValue () const {
  return random;
}

bool 
Srand::IfEventHappen (double _probability) const {
  if (_probability <= 1.0 && _probability >= -1e-6) {
    double probability = _probability * 100;
    double SRAND       = rand () % 100;
    if (SRAND <= probability) {
      return true;
    } else {
      return false;
    }
  } else {
    cout << "In Function \"IfEventHappen\" Input A Wrang Parameter" << endl;
    return false;
  }
}

} // namespace ndn
} // namespace ns3
