/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Antonin Bas (antonin@barefootnetworks.com)
 *
 */

#include "web_server.h"
#include "simple_router_mgr.h"

#include <sstream>
#include <vector>
#include <fstream>

#include <microhttpd.h>

#define POSTBUFFERSIZE  16384
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   8096
#define GET             0
#define POST            1

namespace {

struct connection_info_struct {
	int connectiontype;
	WebServer *web_server;
	
	std::string addRuleTemplate;
	std::string delRuleTemplate;
	std::string addResponse;
	std::string delResponse;
	
	std::string prefix;
	std::string destIp;
	std::size_t prefixLen;
	std::size_t destPort;
	std::size_t delPrefixLen;
	std::string delPrefix;
	
	std::string update_error;
	struct MHD_PostProcessor *postprocessor;
};

const char *monitor_page_template = "<html><head>\
<style type=\"text/css\">td {vertical-align: top;} .list td {border-bottom: 1px solid black; text-align: center;} </style>\
</head><body>\
<h1>L3 Controller monitor page</h1>\
<table><tr><td> \
<h2>Insert rule to LPM table</h2>\
%s\
<h2>Delete rule from LPM table</h2>\
%s\
<td>\
<h2> LPM table overview </h2>\
%s \
</td></tr></table>\
</body></html>";
const char *errorpage = "<html><body>This doesn’t seem to be right.</body></html>";

bool LoadTemplate(const std::string &filename, std::string &dest) {
	std::ifstream load(filename);
	
	try {
		load.seekg (0, load.end);
		int length = load.tellg();
		load.seekg (0, load.beg);
		
		char *buf = new char[length];
		if (buf == NULL) return false;
		
		load.read(buf, length);
		dest = std::string(buf, length);
		
		delete[] buf;
		load.close();
		load.clear();
	}
	catch (...) {
		return false;
	}
	
	return true;
}

std::string IntToIp(const unsigned char *num) {
	std::string result;
	
	for (int i = 3; i >= 0; i--) {
		if (!result.empty()) result += ".";
		result += std::to_string((int)(num[i]));
	}
	
	return result;
}

char *generate_page(WebServer *web_server, connection_info_struct *con_info = NULL) {
	std::cerr << "WS_generate_page\n";
	char *answerstring = new char[MAXANSWERSIZE];
	if (!answerstring) return NULL;
	char *auxstring = new char[MAXANSWERSIZE];
	if (!answerstring) return NULL;
	
	const char *addRuleTemplate = "";
	const char *delRuleTemplate = "";
	const char *listRuleTemplate = "";
	const char *addResponse = "";
	const char *delResponse = "";
	
	std::string addTemplate;
	std::string delTemplate;
	
	if (not LoadTemplate("templates/addrule.html", addTemplate)) {
		std::cerr << "Could not load addrule.html\n";
		return NULL;
	}
	if (not LoadTemplate("templates/delrule.html", delTemplate)) {
		std::cerr << "Could not load delrule.html\n";
		return NULL;
	}
	
	std::string listTemplate = "<table class=\"list\"><tr><th>Prefix</th><th>Prefix Len</th><th>-></th><th>Next Hop</th><th>Port</th></tr>";
	const NextHopMap &nhm = web_server->get_next_hop_map();
	for (auto rule : web_server->get_lpm_rule_map()) {
		uint32_t ip = rule.first;
		listTemplate += "<tr><td>";
		listTemplate += IntToIp((const unsigned char*)(&ip));
		listTemplate += "</td><td>";
		listTemplate += std::to_string(rule.second);
		listTemplate += "</td><td>-&gt;</td><td>";
		listTemplate += IntToIp((const unsigned char*)(&(nhm.at(rule.first).first)));
		listTemplate += "</td><td>";
		listTemplate += std::to_string(nhm.at(rule.first).second);
		listTemplate += "</td></tr>";
	}
	listTemplate += "</table>";
	
	addRuleTemplate = addTemplate.c_str();
	delRuleTemplate = delTemplate.c_str();
	listRuleTemplate = listTemplate.c_str();
	
	if (con_info) {
		assert(web_server == con_info->web_server);
		
		addResponse = con_info->addResponse.c_str();
		delResponse = con_info->delResponse.c_str();
	}
	
	snprintf(auxstring, MAXANSWERSIZE, monitor_page_template, addRuleTemplate, delRuleTemplate, listRuleTemplate);
	snprintf(answerstring, MAXANSWERSIZE, auxstring, addResponse, delResponse);
	
	return answerstring;
}

int send_page(struct MHD_Connection *connection, const char *page) {
	int ret;
	struct MHD_Response *response;
	response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
	if (!response) return MHD_NO;
	
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	
	return ret;
}

int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
	connection_info_struct *con_info = static_cast<connection_info_struct *>(coninfo_cls);
	
	std::vector<std::string> stringInputNames = {"prefix", "dest_ip", "del_prefix"};
	std::vector<std::string*> strings = {&(con_info->prefix), &(con_info->destIp), &(con_info->delPrefix)};
	
	std::vector<std::string> numberInputNames {"prefix_len", "dest_port", "del_prefix_len"};
	std::vector<std::size_t*> numbers = {&con_info->prefixLen, &con_info->destPort, &con_info->delPrefixLen};
	
	// Test text based inputs
	for (std::size_t i = 0; i < stringInputNames.size(); i++) {
		std::string &name = stringInputNames[i];
		
		if (!strcmp(key, name.c_str())) {
			if ((size > 0) && (size <= MAXNAMESIZE)) {
				*(strings[i]) = std::string(data, size);
				return MHD_YES;
			}
			
			return MHD_NO;
		}
	}
	
	// Test number based inputs
	for (std::size_t i = 0; i < numberInputNames.size(); i++) {
		std::string &name = numberInputNames[i];
		
		if (!strcmp(key, name.c_str())) {
			try {
				*(numbers[i]) = std::stoi(std::string(data));
			}
			catch (...) {
				return MHD_NO;
			}
			
			return MHD_YES;
		}
	}
	
	return MHD_NO;
}

void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
	connection_info_struct *con_info = static_cast<connection_info_struct *>(*con_cls);
	
	if (!con_info) return;
	if (con_info->connectiontype == POST) {
		MHD_destroy_post_processor(con_info->postprocessor);
	}
	
	delete con_info;
	*con_cls = NULL;
}

uint32_t IpToInt(const std::string &ip) {
	std::istringstream ss(ip);
	std::string token;

	uint32_t result = 0;
	int offset = 3;
	
	while(std::getline(ss, token, '.')) {
		*((uint8_t*)(&result) + offset) = uint8_t(std::stoi(token));
		offset--;
		if (offset == -1) break;
	}
	
	return result;
}

int perform_requested_ops_and_respond(struct MHD_Connection *connection, connection_info_struct *con_info) {
	WebServer *server = con_info->web_server;
	if (con_info->prefix != "") {
		uint32_t prefix = IpToInt(con_info->prefix);
		uint32_t nhop = IpToInt(con_info->destIp);
		
		int rtn = server->add_route(prefix, con_info->prefixLen, nhop, con_info->destPort);
		if (rtn) {
			con_info->addResponse = "Error while adding rule.";
		}
		else {
			con_info->addResponse = "Rule added.";
		}
	}
	
	if (con_info->delPrefix != "") {
		uint32_t prefix = IpToInt(con_info->delPrefix);
		int rtn = server->del_route(prefix, con_info->delPrefixLen);
		if (rtn) {
			con_info->delResponse = "Error while removing the rule.";
		}
		else {
			con_info->delResponse = "Rule removed.";
		}
	}
	
	return send_page(connection, generate_page(con_info->web_server, con_info));
}

int answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
	WebServer *server = static_cast<WebServer *>(cls);
	
	if (!*con_cls) {
		struct connection_info_struct *con_info;
		con_info = new connection_info_struct;
	
		if (!con_info) return MHD_NO;
		
		con_info->prefix = "";
		con_info->destIp = "";
		con_info->prefixLen = 0;
		con_info->destPort = 0;
		con_info->delPrefixLen = 0;
		con_info->delPrefix = "";
		con_info->addRuleTemplate = "";
		con_info->delRuleTemplate = "";
		con_info->web_server = server;
	
		if (!strncmp (method, "POST", sizeof "POST")) {
			con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, iterate_post, static_cast<void *>(con_info));
			
			if (!con_info->postprocessor) {
				delete con_info;
				return MHD_NO;
			}
		
			con_info->connectiontype = POST;
		}
		else {
			con_info->connectiontype = GET;
		}
		
		*con_cls = static_cast<void *>(con_info);
		return MHD_YES;
	}
	
	if (!strncmp(method, "GET", sizeof "GET")) {
		return send_page(connection, generate_page(server));
	}
	
	if (!strncmp(method, "POST", sizeof "POST")) {
		connection_info_struct *con_info = static_cast<connection_info_struct *>(*con_cls);
	
		if (*upload_data_size != 0) {
			MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
			*upload_data_size = 0;
			return MHD_YES;
		}
		else {
			return perform_requested_ops_and_respond(connection, con_info);
		}
	}
	
	return send_page(connection, errorpage);
}

}  // namespace

WebServer::WebServer(SimpleRouterMgr *simple_router_mgr, int port) : simple_router_mgr(simple_router_mgr), port(port) { }

WebServer::~WebServer() {
	MHD_stop_daemon(daemon);
}

int WebServer::start() {
	daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL, &answer_to_connection, static_cast<void *>(this), MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);
	
	return (daemon != NULL);
}

void WebServer::set_json_name(const std::string &json_name) {
	std::unique_lock<std::mutex> lock(mutex);
	current_json = json_name;
}

std::string WebServer::get_json_name() const {
	std::unique_lock<std::mutex> lock(mutex);
	return current_json;
}

int WebServer::query_counter(const std::string &counter_name, size_t index, uint64_t *packets, uint64_t *bytes) {
	//return simple_router_mgr->query_counter(counter_name, index, packets, bytes);
	return 0;
}

int WebServer::update_json_config(const std::string &config_buffer, const std::string *p4info_buffer) {
	//return simple_router_mgr->update_config(config_buffer, p4info_buffer);
	return 0;
}

int WebServer::add_route(uint32_t prefix, int pLen, uint32_t nhop, uint16_t port) {
	return simple_router_mgr->add_route(prefix, pLen, nhop, port);
}

int WebServer::del_route(uint32_t prefix, int pLen) {
	return simple_router_mgr->del_route(prefix, pLen);
}

const LpmRuleMap &WebServer::get_lpm_rule_map() const {
	return simple_router_mgr->get_lpm_rule_map();
}

const NextHopMap &WebServer::get_next_hop_map() const {
	return simple_router_mgr->get_next_hop_map();
}