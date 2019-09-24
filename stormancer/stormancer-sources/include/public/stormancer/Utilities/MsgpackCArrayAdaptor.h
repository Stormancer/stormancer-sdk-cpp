#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/msgpack_define.h"

/// \file
/// MsgPAck adaptors to support packing and unpacking of C-style arrays (non-dynamic arrays which size is known at compile time).
/// These adaptors were taken from https://github.com/msgpack/msgpack-c/pull/466/files
/// There is a specialization for char[], which will be packed as STR and not ARRAY. However, it can be unpacked as both BIN and STR.

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {

			template<typename T, std::size_t N>
			struct convert<T[N]> {
				msgpack::object const& operator()(msgpack::object const& o, T* v) const {
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size > N) throw msgpack::type_error();
					msgpack::object* p = o.via.array.ptr;
					msgpack::object* const pend = o.via.array.ptr + o.via.array.size;
					do {
						p->convert(*v);
						++p;
						++v;
					} while (p < pend);
				}
			};

			template<typename T, std::size_t N>
			struct pack<T[N]> {
				template <typename Stream>
				packer<Stream>& operator()(msgpack::packer<Stream>& o, const T(&v)[N]) const {
					o.pack_array(N);
					const T* ptr = v;
					for (; ptr != &v[N]; ++ptr) o.pack(*ptr);
					return o;
				}
			};

			template <std::size_t N>
			struct convert<char[N]> {
				msgpack::object const& operator()(msgpack::object const& o, char(&v)[N]) const {
					switch (o.type) {
					case msgpack::type::BIN:
						if (o.via.bin.size > N) { throw msgpack::type_error(); }
						std::memcpy(v, o.via.bin.ptr, o.via.bin.size);
						break;
					case msgpack::type::STR:
						if (o.via.str.size + 1 > N) { throw msgpack::type_error(); }
						std::memcpy(v, o.via.str.ptr, o.via.str.size);
						v[o.via.str.size] = '\0';
						break;
					default:
						throw msgpack::type_error();
						break;
					}
					return o;
				}
			};

		} // namespace adaptor
	} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
