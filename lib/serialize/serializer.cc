#include <serialize/serializer.hh>

namespace libport
{
  namespace serialize
  {
    Serializer::Serializer(bool input)
      : input_(input)
    {}

    Serializer::~Serializer()
    {}
  }
}