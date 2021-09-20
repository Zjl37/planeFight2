#include "boost/locale.hpp"
#include "pfLocale.hpp"

void PfLocaleInit(const std::string id) {
	boost::locale::generator gen;

	gen.add_messages_path("./lang");
	gen.add_messages_domain("messages");
	std::locale::global(gen(id));
	std::cout.imbue(std::locale());
}