#ifndef NODECPP_H
#define NODECPP_H

namespace nodecpp {

template<class T>
class unique_ptr {
	T* ptr;
public:
	unique_ptr(T* ptr = nullptr) :ptr(ptr) {}
	unique_ptr& operator=(const unique_ptr&) = delete;
	void reset(T* ptr) {}

 	T* operator->() { return ptr; }
	T& operator*() { return *ptr; }
};


template <class T>
class naked_ptr {
	T* ptr;
public:
	naked_ptr(T* ptr = nullptr) :ptr(ptr) {}

	naked_ptr(const naked_ptr&) = default;
	naked_ptr(naked_ptr&&) = default;
	naked_ptr& operator=(const naked_ptr&) = default;
	naked_ptr& operator=(naked_ptr&&) = default;

 	T* operator->() { return ptr; }
	T& operator*() { return *ptr; }
};
}




#endif NODECPP_H
