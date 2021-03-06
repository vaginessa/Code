//
// The MIT License(MIT)
//
// Copyright(c) 2014 Demonsaw LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _EJA_JOIN_REQUEST_MESSAGE_
#define _EJA_JOIN_REQUEST_MESSAGE_

#include <memory>

#include "json/json.h"
#include "json/json_message.h"
#include "json/json_value.h"
#include "json/value/json_client.h"
#include "json/value/json_entity.h"
#include "json/value/json_group.h"
#include "json/value/json_option.h"

namespace eja
{
	class join_request_message final : public json_message
	{
	public:
		using ptr = std::shared_ptr<join_request_message>;

	private:
		join_request_message() : json_message(json_type::join, json_action::request) { }
		join_request_message(const json_message::ptr message) : json_message(message, json_type::join, json_action::request) { }
		join_request_message(const json_value::ptr value) : json_message(value, json_type::join, json_action::request) { }

	public:		
		virtual ~join_request_message() override { }

		// Utility
		virtual bool valid() const override { return json_message::valid() && has_entity() && has_client() && has_group(); }
		bool has_entity() const { return get_value().isMember(json::entity) && !get_value()[json::entity].empty(); }
		bool has_client() const { return get_value().isMember(json::client) && !get_value()[json::client].empty(); }
		bool has_group() const { return get_value().isMember(json::group) && !get_value()[json::group].empty(); }

		// Accessor
		json_entity::ptr get_entity() { return json_entity::create(shared_from_this(), json::entity); }
		json_client::ptr get_client() { return json_client::create(shared_from_this(), json::client); }
		json_group::ptr get_group() { return json_group::create(shared_from_this(), json::group); }
		json_option::ptr get_option() { return json_option::create(shared_from_this(), json::option); }

		// Static
		static ptr create() { return ptr(new join_request_message()); }
		static ptr create(const json_message::ptr message) { return ptr(new join_request_message(message)); }
		static ptr create(const json_value::ptr value) { return ptr(new join_request_message(value)); }
	};
}

#endif
