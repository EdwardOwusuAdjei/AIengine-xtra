/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2014  Luis Campo Giralte
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
#include "HTTPProtocol.h"
#include <iomanip> // setw

namespace aiengine {

#ifdef HAVE_LIBLOG4CXX
log4cxx::LoggerPtr HTTPProtocol::logger(log4cxx::Logger::getLogger("aiengine.http"));
#endif

// List of support request methods rfc 2616
std::vector<HttpMethodType> HTTPProtocol::methods_ {
        std::make_tuple("GET"      	,3,     "gets"     	,0),
        std::make_tuple("POST"        	,4,     "posts"       	,0),
        std::make_tuple("HEAD"          ,4,     "heads"		,0),
        std::make_tuple("CONNECT"       ,7,     "connects"      ,0),
        std::make_tuple("OPTIONS"       ,7,     "options"       ,0),
        std::make_tuple("PUT"          	,3,     "puts"         	,0),
        std::make_tuple("DELETE"        ,6,     "deletes"       ,0),
        std::make_tuple("TRACE"         ,5,     "traces"        ,0)
};

// List of parameters supported
//std::unordered_map<std::string,HttpParameterHandler> HTTPProtocol::parameters_ {
//	{ std::make_pair<std::string,HttpParameterHandler>("Host",std::bind(&HTTPProtocol::process_host_parameter,this,std::placeholders::_1)) }
//};


void HTTPProtocol::release_http_info_cache(HTTPInfo *info) {

        SharedPointer<StringCache> ua_ptr = info->ua.lock();

        if (ua_ptr) { // There is no user agent attached
                UAMapType::iterator it = ua_map_.find(ua_ptr->getName());
		if (it != ua_map_.end()) {
			int *counter = &std::get<1>(it->second);
			--(*counter);

			if ((*counter) <= 0) {
				ua_map_.erase(it);
			}
		}
	}

        SharedPointer<StringCache> uri_ptr = info->uri.lock();

        if (uri_ptr) { // There is a Uri attached
                UriMapType::iterator it = uri_map_.find(uri_ptr->getName());
                if (it != uri_map_.end()) {
                        int *counter = &std::get<1>(it->second);
                        --(*counter);
                        
                        if ((*counter) <= 0) {
                                uri_map_.erase(it);
                        }
                }
        }

	release_http_info(info);
}


int32_t HTTPProtocol::release_http_info(HTTPInfo *info) {

	int32_t bytes_released = 0;

	SharedPointer<StringCache> host = info->host.lock();

        if (host) { // The flow have a host attatched and uri and uas
        	bytes_released += host->getName().size();
                host_cache_->release(host);
	}

        SharedPointer<StringCache> ua = info->ua.lock();
        if (ua) {
		bytes_released += ua->getName().size();
                ua_cache_->release(ua);
	}

        SharedPointer<StringCache> uri = info->uri.lock();
        if (uri) {
        	bytes_released += uri->getName().size();
                uri_cache_->release(uri);
        }
        info->resetStrings();

	return bytes_released;
}


void HTTPProtocol::releaseCache() {

        FlowManagerPtr fm = flow_mng_.lock();

        if (fm) {
                auto ft = fm->getFlowTable();

                std::ostringstream msg;
                msg << "Releasing " << getName() << " cache";

                infoMessage(msg.str());

		int32_t total_bytes_released = 0;
		int32_t total_bytes_released_by_flows = 0;
                int32_t release_flows = 0;
                int32_t release_hosts = host_map_.size();
                int32_t release_uris = uri_map_.size();
                int32_t release_uas = ua_map_.size();

                // Compute the size of the strings used as keys on the map
                std::for_each (host_map_.begin(), host_map_.end(), [&total_bytes_released] (std::pair<std::string,StringCacheHits> const &ht) {
                        total_bytes_released += ht.first.size();
                });
                std::for_each (ua_map_.begin(), ua_map_.end(), [&total_bytes_released] (std::pair<std::string,StringCacheHits> const &ht) {
                        total_bytes_released += ht.first.size();
                });
                std::for_each (uri_map_.begin(), uri_map_.end(), [&total_bytes_released] (std::pair<std::string,StringCacheHits> const &ht) {
                        total_bytes_released += ht.first.size();
                });

                for (auto &flow: ft) {
			SharedPointer<HTTPInfo> info = flow->http_info.lock();
			if (info) {

				total_bytes_released_by_flows += release_http_info(info.get());
				total_bytes_released_by_flows += sizeof(info);
				info.reset();
				flow->http_info.reset();	
                        }
                } 
                host_map_.clear();
		uri_map_.clear();
		ua_map_.clear();

                double cache_compression_rate = 0;

                if (total_bytes_released > 0 ) {
                        cache_compression_rate = 100 - ((total_bytes_released*100)/total_bytes_released_by_flows);
                }

                msg.str("");
                msg << "Release " << release_hosts << " hosts, " << release_uas;
		msg << " useragents, " << release_uris << " uris, " << release_flows << " flows";
		msg << ", " << total_bytes_released << " bytes, compression rate " << cache_compression_rate << "%";
                infoMessage(msg.str());
        }
}


void HTTPProtocol::attach_host(HTTPInfo *info, std::string &host) {

	SharedPointer<StringCache> host_ptr = info->host.lock();

	if (!host_ptr) { // There is no Host object attached to the flow
		HostMapType::iterator it = host_map_.find(host);
		if (it == host_map_.end()) {
			host_ptr = host_cache_->acquire().lock();
			if (host_ptr) {
				host_ptr->setName(host);
				info->host = host_ptr;
				host_map_.insert(std::make_pair(host,std::make_pair(host_ptr,1)));
			}
		} else {
			int *counter = &std::get<1>(it->second);
			++(*counter);
			info->host = std::get<0>(it->second);
		}
	}
}

bool HTTPProtocol::process_host_parameter(HTTPInfo *info,const char *parameter) {

	std::string host(parameter);

	DomainNameManagerPtr ban_hosts = ban_host_mng_.lock();
        if (ban_hosts) {
        	SharedPointer<DomainName> host_candidate = ban_hosts->getDomainName(host);
                if (host_candidate) {
#ifdef HAVE_LIBLOG4CXX
                	LOG4CXX_INFO (logger, "Flow:" << *flow << " matchs with ban host " << host_candidate->getName());
#endif
                        ++total_ban_hosts_;
                        return false;
                }
	}
        ++total_allow_hosts_;
	attach_host(info,host);
	return true;
}

bool HTTPProtocol::process_ua_parameter(HTTPInfo *info, const char *parameter) {

	std::string ua(parameter);

	attach_useragent(info,ua);
	return true;
}

bool HTTPProtocol::process_content_length_parameter(HTTPInfo *info, const char *parameter) {

	std::string value(parameter);
	int32_t length = std::stoi(value);

	info->setContentLength(length);
	info->setHaveData(true);
	// std::cout << "Content-length:" << length << std::endl;
	return true;
}

void HTTPProtocol::attach_useragent(HTTPInfo *info, std::string &ua) {

	SharedPointer<StringCache> ua_ptr = info->ua.lock();

	if (!ua_ptr) { // There is no user agent attached
		UAMapType::iterator it = ua_map_.find(ua);
		if (it == ua_map_.end()) {
			ua_ptr = ua_cache_->acquire().lock();
			if (ua_ptr) {
				ua_ptr->setName(ua);
				info->ua = ua_ptr;
				ua_map_.insert(std::make_pair(ua,std::make_pair(ua_ptr,1)));
			}	
		} else {
			int *counter = &std::get<1>(it->second);
			++(*counter);
			info->ua = std::get<0>(it->second);	
		}
	}
}

// The URI should be updated on every request
void HTTPProtocol::attach_uri(HTTPInfo *info, std::string &uri) {

	UriMapType::iterator it = uri_map_.find(uri);
        if (it == uri_map_.end()) {
        	SharedPointer<StringCache> uri_ptr = uri_cache_->acquire().lock();
                if (uri_ptr) {
                	uri_ptr->setName(uri);
                        info->uri = uri_ptr;
                        uri_map_.insert(std::make_pair(uri,std::make_pair(uri_ptr,1)));
			++total_requests_;
                } 
        } else {
		// Update the URI of the flow
		info->uri = std::get<0>(it->second);
		++total_requests_;
	}
}


int HTTPProtocol::extract_uri(HTTPInfo *info, const char *header) {

        int offset = 0;
        bool found = false;
        std::string http_header(header);
	int method_size = 0;

        // Check if is a response
        if (std::memcmp("HTTP/1.",&header[0],6) == 0) {
                ++total_responses_;
		int end = http_header.find("\r\n");
		if (end > 0) {
			method_size = end + 2;
		}
                // No uri to extract
                return method_size;
        }

	// Is not a response so check what request type is
        for (auto &method: methods_) {
                const char *m = std::get<0>(method);
                offset = std::get<1>(method);

                if (std::memcmp(m,&header[0],offset) == 0) {
                        int32_t *hits = &std::get<3>(method);

                        found = true;
                        ++offset;
                        ++(*hits);
                        break;
                }
        }

        if ((found)and(offset > 0)) {
                int end = http_header.find("HTTP/1.");
                if (end > 0) {
                        std::string uri(http_header,offset,(end-offset) -1);

                        ++total_requests_;
                        attach_uri(info,uri);
			method_size = end + 10;
                }
        }else{
                ++total_http_others_;
        }
	return method_size;
}

void HTTPProtocol::parse_header(HTTPInfo *info, const char *parameters) {

	std::string http_header(parameters);
        std::string token, parameter, auxtoken;
        bool have_token = false;
        int i = 0;
        
	// Process the HTTP header
        const char *ptr = parameters;

	for (i = 0; i< http_header.length(); ++i) {
       		// Check if is end off line
                if (std::memcmp(&ptr[i],"\r\n",2) == 0 ) {

                	if(token.length()) {
                        	auto it = parameters_.find(token);
                                if (it != parameters_.end()) {
                                	auto callback = (*it).second;

                                        bool sw = callback(info,parameter.c_str());
					if (!sw) { // The flow have been marked as banned
                                                info->setIsBanned(true);
						release_http_info_cache(info); 
						break;
					}
                                }
                                // std::cout <<"FULL:(" << token << "):(" << parameter << ")" << std::endl;
                                token.clear();
                                parameter.clear();
                                auxtoken.clear();
			}

			if(std::memcmp(&ptr[i+2],"\r\n",2) == 0) {
				// end of the header
				http_header_size_ += 4;
				break;
			}
                       	have_token = false;
                       	++i;
		} else {
			if ((ptr[i] == ':')and(have_token == false)) {
                        	parameter.clear();
                                token = auxtoken;
                                auxtoken.clear();
                                have_token = true;
                                ++i;
			} else {
                                auxtoken += ptr[i];
                                parameter += ptr[i];
			}
		}
	}
	http_header_size_ += i;
}


void HTTPProtocol::processFlow(Flow *flow) {

	http_header_size_ = 0;
	++total_packets_;	
	total_bytes_ += flow->packet->getLength();
	++flow->total_packets_l7;

	SharedPointer<HTTPInfo> info = flow->http_info.lock();

	if(!info) {
		info = info_cache_->acquire().lock();
                if (!info) {
			return;
		}
		flow->http_info = info;
	} 

	if (info->getIsBanned() == true) {
		// std::cout << "Flow:" << flow << " have been banned" << std::endl; 
		return;
	}

	const char *header = reinterpret_cast <const char*> (flow->packet->getPayload());
	// Requests
	if (flow->getFlowDirection() == FlowDirection::FORWARD) {

		int offset = extract_uri(info.get(),header);
		if ((offset > 0)and(flow->total_packets_l7 == 1)) {

			http_header_size_ = offset;
			parse_header(info.get(),&header[offset]);
		}

                DomainNameManagerPtr host_mng = host_mng_.lock();
                if (host_mng) {
			SharedPointer<StringCache> host_name = info->host.lock();

			if (host_name) {
                		SharedPointer<DomainName> host_candidate = host_mng->getDomainName(host_name->getName());
				if (host_candidate) {
#ifdef PYTHON_BINDING
#ifdef HAVE_LIBLOG4CXX
					LOG4CXX_INFO (logger, "Flow:" << *flow << " matchs with " << host_candidate->getName());
#endif	
					if(host_candidate->haveCallback()) {
						host_candidate->executeCallback(flow);
                                	}
#endif
                        	}
			}
		}
	} else {

		int offset = extract_uri(info.get(),header);
		if (offset > 0) {
			
			http_header_size_ = offset;
			parse_header(info.get(),&header[offset]);

		}
	}
}


void HTTPProtocol::createHTTPInfos(int number) {

	info_cache_->create(number);
	uri_cache_->create(number);
	host_cache_->create(number);
	ua_cache_->create(number);
}

void HTTPProtocol::destroyHTTPInfos(int number) {

	info_cache_->destroy(number);
	uri_cache_->destroy(number);
	host_cache_->destroy(number);
	ua_cache_->destroy(number);
}

void HTTPProtocol::statistics(std::basic_ostream<char>& out) {

	if (stats_level_ > 0) {
		out << getName() << "(" << this << ") statistics" << std::dec <<  std::endl;
		out << "\t" << "Total packets:          " << std::setw(10) << total_packets_ <<std::endl;
		out << "\t" << "Total bytes:        " << std::setw(14) << total_bytes_ <<std::endl;
		if (stats_level_ > 1) {
        		out << "\t" << "Total validated packets:" << std::setw(10) << total_validated_packets_ <<std::endl;
        		out << "\t" << "Total malformed packets:" << std::setw(10) << total_malformed_packets_ <<std::endl;
			if(stats_level_ > 3) { 

				out << "\t" << "Total allow hosts:      " << std::setw(10) << total_allow_hosts_ <<std::endl;
				out << "\t" << "Total banned hosts:     " << std::setw(10) << total_ban_hosts_ <<std::endl;
                                out << "\t" << "Total requests:         " << std::setw(10) << total_requests_ <<std::endl;
                                out << "\t" << "Total responses:        " << std::setw(10) << total_responses_ <<std::endl;
                                for (auto &method: methods_) {
                                        const char *label = std::get<2>(method);
                                        int32_t hits = std::get<3>(method);
                                        out << "\t" << "Total " << label << ":" << std::right << std::setfill(' ') << std::setw(27 - strlen(label)) << hits <<std::endl;

                                }
                                out << "\t" << "Total others:           " << std::setw(10) << total_http_others_ <<std::endl;
			}
			if (stats_level_ > 2) {
				if (flow_forwarder_.lock())
					flow_forwarder_.lock()->statistics(out);
				if (stats_level_ > 3) {
					info_cache_->statistics(out);
					uri_cache_->statistics(out);
					host_cache_->statistics(out);
					ua_cache_->statistics(out);
					if(stats_level_ > 4) {
						showCacheMap(out,uri_map_,"HTTP Uris","Uri");
						showCacheMap(out,host_map_,"HTTP Hosts","Host");
						showCacheMap(out,ua_map_,"HTTP UserAgents","UserAgent");
					}
				}
			}
		}
	}
}

} // namespace aiengine 
