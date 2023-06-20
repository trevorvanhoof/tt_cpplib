#pragma once

#include <vector>
#include <iostream>
#include <functional>
#include <unordered_map>

namespace TT {
	struct Lifetime {
		size_t handle;

		Lifetime();
		~Lifetime();
		Lifetime(const Lifetime& other);
		Lifetime(Lifetime&& other) noexcept;
		Lifetime& operator=(const Lifetime& other);
		Lifetime& operator=(Lifetime && other) noexcept;

		static size_t nextHandle;
		static std::unordered_map<size_t, size_t> refCount;
	};

	template<typename... Args>
	struct Signal {
		size_t nextHandle = 0;
		typedef std::function<void(Args... args)> Callback;

		struct Listener {
			Callback listener;
			size_t lifetimeHandle;
		};

		std::unordered_map<size_t, Listener> listeners;

		size_t connect(const Callback& callable, Lifetime* lifetime = nullptr) {
			size_t handle = nextHandle++;
			listeners.insert_or_assign(handle, Listener{ callable, lifetime == nullptr ? -1 : lifetime->handle });
			return handle;
		}

		void disconnect(size_t handle) {
			listeners.erase(handle);
		}

		void emit(Args... args) {
			for (const auto& pair : listeners) {
				const auto& it = Lifetime::refCount.find(pair.second.lifetimeHandle);
				if (it == Lifetime::refCount.end() || it->second > 0)
					pair.second.listener(args...);
			}
		}
	};
}

#define TT_CONNECT(signal, object, callback, ...) signal.connect(std::bind(&callback, object, __VA_ARGS__), &object->lifetime);
#define TT_CONNECT_L(signal, object, callback, ...) signal.connect(std::bind(&callback, object, __VA_ARGS__));

#define TT_CONNECT0(signal, object, callback) TT_CONNECT(signal, object, callback)
#define TT_CONNECT1(signal, object, callback) TT_CONNECT(signal, object, callback, std::placeholders::_1)
#define TT_CONNECT2(signal, object, callback) TT_CONNECT(signal, object, callback, std::placeholders::_1, std::placeholders::_2)
#define TT_CONNECT3(signal, object, callback) TT_CONNECT(signal, object, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define TT_CONNECT4(signal, object, callback) TT_CONNECT(signal, object, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#define TT_CONNECT0_L(signal, object, callback) TT_CONNECT_L(signal, object, callback)
#define TT_CONNECT1_L(signal, object, callback) TT_CONNECT_L(signal, object, callback, std::placeholders::_1)
#define TT_CONNECT2_L(signal, object, callback) TT_CONNECT_L(signal, object, callback, std::placeholders::_1, std::placeholders::_2)
#define TT_CONNECT3_L(signal, object, callback) TT_CONNECT_L(signal, object, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define TT_CONNECT4_L(signal, object, callback) TT_CONNECT_L(signal, object, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#define TT_CONNECT0_SELF(signal, callback) TT_CONNECT(signal, this, callback)
#define TT_CONNECT1_SELF(signal, callback) TT_CONNECT(signal, this, callback, std::placeholders::_1)
#define TT_CONNECT2_SELF(signal, callback) TT_CONNECT(signal, this, callback, std::placeholders::_1, std::placeholders::_2)
#define TT_CONNECT3_SELF(signal, callback) TT_CONNECT(signal, this, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define TT_CONNECT4_SELF(signal, callback) TT_CONNECT(signal, this, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#define TT_CONNECT0_SELF_L(signal, callback) TT_CONNECT_L(signal, this, callback)
#define TT_CONNECT1_SELF_L(signal, callback) TT_CONNECT_L(signal, this, callback, std::placeholders::_1)
#define TT_CONNECT2_SELF_L(signal, callback) TT_CONNECT_L(signal, this, callback, std::placeholders::_1, std::placeholders::_2)
#define TT_CONNECT3_SELF_L(signal, callback) TT_CONNECT_L(signal, this, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define TT_CONNECT4_SELF_L(signal, callback) TT_CONNECT_L(signal, this, callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#if 0
struct Emitter {
	Signal<> test;
};

struct Observer {
	Lifetime lifetime;

	Observer(Emitter& e) {
		TT_CONNECT0(e.test, this, Observer::cb);
	}

	void cb() {
		std::cout << " test\n";
	}
};

void test() {
	Emitter e;
	{
		Observer a(e);
	}
	Observer b(e);
	e.test.emit();
	e.test.emit();
}
#endif
