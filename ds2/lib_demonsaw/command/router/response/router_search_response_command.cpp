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

#include "router_search_response_command.h"
#include "command/router/request/router_ping_request_command.h"
#include "command/router/request/router_search_request_command.h"
#include "component/client/client_option_component.h"
#include "component/group/group_component.h"
#include "component/session_component.h"
#include "component/tunnel_component.h"
#include "data/request/search_request_data.h"
#include "json/json_packet.h"
#include "message/request/search_request_message.h"
#include "message/response/search_response_message.h"
#include "security/base.h"
#include "system/mutex/mutex_queue.h"

namespace eja
{
	http_status router_search_response_command::execute(const entity::ptr entity, const json_message::ptr message, const json_data::ptr data)
	{
		// Option
		const auto option = get_router_option();
		if (!option)
		{
			m_socket->write(http_code::internal_server_error);
			return http_code::internal_server_error;
		}

		if (!option->get_message_router())
		{
			m_socket->write(http_code::not_implemented);
			return http_code::not_implemented;
		}

		// Request
		const auto request_message = search_request_message::create(message);
		if (request_message->invalid())
		{
			m_socket->write(http_code::bad_request);
			return http_code::bad_request;
		}

		// Group
		const auto group_map = get_group_entity_map();
		if (!group_map)
		{
			m_socket->write(http_code::internal_server_error);
			return http_code::internal_server_error;
		}

		const auto group = entity->get<group_component>();
		if (!group)
		{
			m_socket->write(http_code::internal_server_error);
			return http_code::internal_server_error;
		}

		const auto& group_id = group->get_id();
		const auto group_entity = group_map->get(group_id);
		if (!group_entity)
		{
			m_socket->write(http_code::not_found);
			return http_code::not_found;
		}

		// Public group only
		if (group_id.empty())
		{
			// Anti Reverse Engineer
			const auto request_data = get_data(data);
			const auto request_search = search_request_data::create(request_data, json::search);
			if (request_search->invalid())
			{
				m_socket->write(http_code::ok);
				return http_code::ok;
			}

			// Anti Spam
			const auto spam = entity->get<spam_component>();
			if (spam)
			{
				spam->request();
				spam->update();

				if (spam->invalid())
				{
					const auto spam_map = get_spam_entity_map();
					if (spam_map)
					{
						// TODO: Replace with MAC Address
						//
						const auto client = entity->get<client_component>();
						if (client)
						{
							const auto spam_timeout = entity->get<spam_timeout_component>();
							if (spam_timeout)
							{
								spam_timeout->restart();
								spam_map->add(client->get_id(), entity);
							}
						}
					}

					m_socket->write(http_code::ok);
					return http_code::ok;
				}
			}
		}

		const auto spam_map = get_spam_entity_map();
		if (spam_map)
		{
			const auto client = entity->get<client_component>();
			if (client && spam_map->has(client->get_id()))
			{
				m_socket->write(http_code::ok);
				return http_code::ok;
			}
		}

		const auto entity_map = group_entity->get<entity_map_component>();
		if (!entity_map)
		{
			m_socket->write(http_code::internal_server_error);
			return http_code::internal_server_error;
		}

		// Session
		const auto session = entity->get<session_component>();
		if (!session)
		{
			m_socket->write(http_code::internal_server_error);
			return http_code::internal_server_error;
		}

		// Response
		const auto response_id = hex::random();		
		const auto response_packet = json_packet::create();
		const auto response_message = search_response_message::create(response_packet);
		response_message->set_id(response_id);

		const auto json = response_packet->str();
		const auto json_encrypted = session->encrypt(json);
		const auto json_encoded = base64::encode(json_encrypted);

		// Tunnel		
		const auto tunnel = entity->get<tunnel_component>();
		if (!tunnel)
		{
			m_socket->write(http_code::not_found);
			return http_code::not_found;
		}

		// Socket
		const auto socket = tunnel->get_socket();
		if (!socket)
		{
			m_socket->write(http_code::not_found);
			return http_code::not_found;
		}

		// Http
		http_ok_response response;
		response.set_body(json_encoded);
		m_socket->write(response);

		// Limit the max number of threads		
		const auto entity_queue = std::make_shared<mutex_queue<eja::entity>>(entity_map->get_queue());
		const auto max_threads = std::min(entity_queue->size(), option->get_max_threads());

		// Command		
		const auto self = shared_from_this();

		for (size_t i = 0; i < max_threads; ++i)
		{			
			std::thread thread([this, self, data, entity, socket, entity_queue, response_id]()
			{
				entity::ptr item = nullptr;

				while (item = entity_queue->pop())
				{
					// Ignore self
					if (item == entity)
						continue;

					// Ignore ghosts
					const auto option = item->get<client_option_component>();
					if (option && !option->get_search())
						continue;

					// Tunnel			
					const auto item_tunnel = item->get<tunnel_component>();
					if (!item_tunnel)
						continue;

					// Socket
					const auto item_socket = item_tunnel->get_socket();
					if (!item_socket)
						continue;

					try
					{
						// Request
						const auto request_command = router_search_request_command::create(self->get_entity(), item_socket);
						const auto request_status = request_command->execute(item, data);
						const auto response_data = request_command->get_data();

						if (request_status.is_ok() && response_data)
						{
							// Response
							{
								const auto response_command = router_search_response_command::create(self->get_entity(), socket);
								response_command->set_id(response_id);

								const auto response_status = response_command->execute(entity, response_data);
								if (response_status.is_ok())
								{
									const auto timeout = entity->get<timeout_component>();
									if (timeout)
										timeout->update();
								}
							}

							item->update();
						}
						else if (request_status.is_none())
						{
							// CORE DUMP
							item_socket->close();
						}
					}
					catch (std::exception& e)
					{
						// CORE DUMP
						item_socket->close();
						log(e);
					}
					catch (...)
					{
						// CORE DUMP
						item_socket->close();
						log();
					}
				}
			});

			thread.detach();
		}

		return response.get_status();
	}

	http_status router_search_response_command::execute(const entity::ptr entity, const json_data::ptr data)
	{
		// Response
		const auto response_packet = json_packet::create();
		const auto response_message = search_response_message::create(response_packet);
		response_message->set_id(m_id);

		const auto response_data = json_data::create(response_packet);
		response_data->set(data);

		const auto json = response_packet->str();
		const auto session = entity->get<session_component>();
		const auto json_encrypted = session->encrypt(json);
		const auto json_encoded = base64::encode(json_encrypted);

		// Http
		http_request request;
		http_response response;
		request.set_body(json_encoded);

		{
			auto_lock_ptr(m_socket);
			m_socket->write(request);
			m_socket->read(response);
		}

		return response.get_status();
	}
}
