// Srand.h

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace ns3 {
namespace ndn {

class Srand {

private :

  int random;

  int UP;

  int LOW;

public :

  Srand (double up, double low);

  Srand (int up   , double low);

  Srand (double up, int low);

  Srand (int up   , int low);

  Srand ();

  int 
  GetValue () const;

  bool
  IfEventHappen (double _probability) const;

};

} // namespace ndn
} // namespace ns3
