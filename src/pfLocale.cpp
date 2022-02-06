/**
 * Copyright © 2021 Zjl37 <2693911885@qq.com>
 *
 * This file is part of Zjl37/planeFight2.
 *
 * PlaneFight2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "boost/locale.hpp"
#include "pfLocale.hpp"

void PfLocaleInit(const std::string id) {
	boost::locale::generator gen;

	gen.add_messages_path("./locale");
	gen.add_messages_domain("messages");
	std::locale::global(gen(id));
	std::cout.imbue(std::locale());
}
