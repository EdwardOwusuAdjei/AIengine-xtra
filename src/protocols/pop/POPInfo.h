/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2016  Luis Campo Giralte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 
 *
 */
#ifndef SRC_PROTOCOLS_POP_POPINFO_H_
#define SRC_PROTOCOLS_POP_POPINFO_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include "StringCache.h"

namespace aiengine {

class POPInfo 
{
public:
    	explicit POPInfo() { reset(); }
    	virtual ~POPInfo() {}

	void reset() { 
		client_commands_ = 0;
		server_commands_ = 0;
		user_name.reset();
		is_banned_ = false;
	}

	void resetStrings() { user_name.reset(); }

	void incClientCommands() { ++client_commands_; }
	void incServerCommands() { ++server_commands_; }

        void setIsBanned(bool value) { is_banned_ = value; }
        bool getIsBanned() const { return is_banned_; }

	WeakPointer<StringCache> user_name;
	
#if defined(PYTHON_BINDING) || defined(RUBY_BINDING)

	friend std::ostream& operator<< (std::ostream& out, const POPInfo& iinfo) {
	
		out << "Client cmds:" << iinfo.client_commands_ << " Server cmds:" << iinfo.server_commands_;
        	return out;
	}

	const char *getUserName() const { return (!user_name.expired() ? user_name.lock()->getName() : ""); }
#endif

private:
	int16_t client_commands_;	
	int16_t server_commands_;	
	bool is_banned_;
};

} // namespace aiengine

#endif  // SRC_PROTOCOLS_POP_POPINFO_H_
