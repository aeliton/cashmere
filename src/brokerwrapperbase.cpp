#include "cashmere/brokerwrapper.h"

namespace Cashmere {

WrapperStoreBase::~WrapperStoreBase() = default;

WrapperBase::WrapperBase(const std::string& url)
  : _url(url)
{
}
WrapperBase::~WrapperBase() {}

std::string WrapperBase::url() const
{
  return _url;
}

}
