#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/msgpack_define.h"
#include "cpprest/json.h"

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {

			template <>
			struct convert<web::json::value> {
				msgpack::object const& operator()(msgpack::object const& o, web::json::value& v) const {
					utility::string_t str;
					switch (o.type) {
					case msgpack::type::STR:
#ifdef _UTF16_STRINGS
						str.assign(o.via.str.ptr, (o.via.str.ptr + o.via.str.size));
#else
						str.assign(o.via.str.ptr, o.via.str.size);
#endif
						v = web::json::value::parse(str);
						break;
					case msgpack::type::NIL:
						v = web::json::value::Null;
						break;
					default:
						throw msgpack::type_error();
						break;
					}
					return o;
				}
			};

			template <>
			struct pack<web::json::value> {
				template <typename Stream>
				msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, web::json::value const& v) const {
					auto wstr = v.serialize();
					auto str = utility::conversions::to_utf8string(wstr);
					uint32_t size = checked_get_container_size(str.size());
					o.pack_str(size);
					o.pack_str_body(str.data(), size);
					return o;
				}
			};

			template <>
			struct object<web::json::value> {
				void operator()(msgpack::object& o, const web::json::value& v) const {
					auto wstr = v.serialize();
					auto str = utility::conversions::to_utf8string(wstr);
					uint32_t size = checked_get_container_size(str.size());
					o.type = msgpack::type::STR;
					o.via.str.ptr = str.data();
					o.via.str.size = size;
				}
			};

			template <>
			struct object_with_zone<web::json::value> {
				void operator()(msgpack::object::with_zone& o, const web::json::value& v) const {
					auto wstr = v.serialize();
					auto str = utility::conversions::to_utf8string(wstr);
					uint32_t size = checked_get_container_size(str.size());
					o.type = msgpack::type::STR;
					char* ptr = static_cast<char*>(o.zone.allocate_align(size));
					o.via.str.ptr = ptr;
					o.via.str.size = size;
					std::memcpy(ptr, str.data(), str.size());
				}
			};
		} // namespace adaptor
	} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack


