#include <boost/locale/message.hpp>
#include <string>

void PfLocaleInit(const std::string id);

#define TT(s) (boost::locale::translate((s)))