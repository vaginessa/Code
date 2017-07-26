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

#ifndef _EJA_OBJECT_
#define _EJA_OBJECT_

#include <memory>
#include <string>

#include "security/hex.h"
#include "system/mutex/mutex_list.h"
#include "system/mutex/mutex_map.h"
#include "system/mutex/mutex_queue.h"
#include "system/mutex/mutex_vector.h"
#include "system/type.h"

namespace eja
{
	class object : public type
	{
	protected:
		std::string m_id;

	public:
		using ptr = std::shared_ptr<object>;

	protected:
		object() { }
		object(const eja::object& object) : m_id(object.m_id) { }
		object(const size_t size) { m_id = hex::random(size); }
		object(const byte* id, const size_t size) : m_id(reinterpret_cast<const char*>(id), size) { }
		object(const std::string& id) : m_id(id) { }
		object(const char* id) : m_id(id) { }
		virtual ~object() override { }

	public:
		// Operator
		object& operator=(const object& object);
		object& operator=(const std::string& id);
		object& operator=(const char* id);

		bool operator==(const object& object) { return m_id == object.m_id; }
		bool operator!=(const object& object) { return m_id != object.m_id; }
		bool operator<(const object& object) { return m_id < object.m_id; }
		bool operator>(const object& object) { return m_id > object.m_id; }
		bool operator<=(const object& object) { return m_id <= object.m_id; }
		bool operator>=(const object& object) { return m_id >= object.m_id; }

		// Interface
		virtual void clear() { m_id.clear(); }
		virtual std::string str() const override { return m_id; }

		// Utility
		bool has_id() const { return !m_id.empty(); }

		// Mutator
		virtual void set_id() { m_id = hex::random(); }
		virtual void set_id(const std::nullptr_t null) { m_id.clear(); } 
		virtual void set_id(const byte* id, const size_t size) { m_id.assign(reinterpret_cast<const char*>(id), size); }
		virtual void set_id(const std::string& id) { m_id = id; }
		virtual void set_id(const char* id) { m_id = id; }

		// Accessor
		const std::string& get_id() const { return m_id; }
		std::string& get_id() { return m_id; }
	};

	// Container
	derived_type(object_list, mutex_list<object>);
	derived_type(object_map, mutex_map<std::string, object>);
	derived_type(object_queue, mutex_queue<object>);
	derived_type(object_vector, mutex_vector<object>);
}

#endif
