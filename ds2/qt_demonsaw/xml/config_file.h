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

#ifndef _EJA_CONFIG_FILE_
#define _EJA_CONFIG_FILE_

#include <boost/property_tree/ptree.hpp>

#include "xml/xml_config_file.h"

namespace eja
{
	class config_file final : public xml_config_file
	{
	private:
		bool m_option = default_view::option;
		bool m_toolbar = default_view::toolbar;
		bool m_menubar = default_view::menubar;
		bool m_statusbar = default_view::statusbar;
		bool m_detail = default_view::detail;

	private:
		// Accessor
		virtual void get_client_group(const boost::property_tree::iptree& tree, const entity::ptr& entity) const override;

	public:
		config_file() { }
		virtual ~config_file() override { }

		// Using
		using xml_config_file::read;
		using xml_config_file::write;

		// Interface
		virtual bool read(const char* path) override;
		virtual bool write(const char* path) override;

		// Mutator
		void set_option(const bool value) { m_option = value; }
		void set_toolbar(const bool value) { m_toolbar = value; }
		void set_menubar(const bool value) { m_menubar = value; }
		void set_statusbar(const bool value) { m_statusbar = value; }
		void set_detail(const bool value) { m_detail = value; }

		bool get_option() const { return m_option; }
		bool get_toolbar() const { return m_toolbar; }
		bool get_menubar() const { return m_menubar; }
		bool get_statusbar() const { return m_statusbar; }
		bool get_detail() const { return m_detail; }
	};
}

#endif
