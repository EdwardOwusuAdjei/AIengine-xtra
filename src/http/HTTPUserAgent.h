/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013  Luis Campo Giralte
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
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
 *
 */
#ifndef _HTTPUserAgent_H_
#define _HTTPUserAgent_H_

#include <iostream>

class HTTPUserAgent 
{
public:
    	explicit HTTPUserAgent(const std::string& name):ua_name_(name) {};
    	explicit HTTPUserAgent():ua_name_("") {};
    	virtual ~HTTPUserAgent() {};

        void setName(const std::string& name) { ua_name_ = name;};
	std::string &getName() { return ua_name_; };

private:
	std::string ua_name_;
};

typedef std::shared_ptr<HTTPUserAgent> HTTPUserAgentPtr;
typedef std::weak_ptr<HTTPUserAgent> HTTPUserAgentPtrWeak;

#endif
