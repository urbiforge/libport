/**
 ** Test code for libport/shared-ptr.hh features.
 */

#include <libport/ref-counted.hh>
#include <libport/shared-ptr.hh>

#define INSTANCES(N) assert(Counted::instances == N)

using libport::shared_ptr;

struct Counted : libport::RefCounted
{
  Counted () : libport::RefCounted() { ++instances; }
  virtual ~Counted () { assert (0 < instances); --instances; }
  static unsigned instances;
};

struct SubCounted : Counted
{
};

unsigned Counted::instances;

typedef shared_ptr<Counted> rCounted;
typedef shared_ptr<SubCounted> rSubCounted;

int
main ()
{
  {
    SubCounted* subcounted = new SubCounted;
    rSubCounted r1 (subcounted);
    {
      INSTANCES(1);
      rCounted r2;
      INSTANCES(1);
      r2 = r1;
      INSTANCES(1);
      rSubCounted r3 = r2.cast<SubCounted>();
      INSTANCES(1);
      rCounted r4 = new Counted;
      INSTANCES(2);
      r4 = r3;
      INSTANCES(1);
    }
    INSTANCES(1);
  }
  INSTANCES(0);

  {
    rSubCounted p = new SubCounted();
    p = p;
    INSTANCES(1);
  }
  INSTANCES(0);
}
