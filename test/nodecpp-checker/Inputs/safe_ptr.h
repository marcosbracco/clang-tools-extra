#ifndef SAFE_PTR_H
#define SAFE_PTR_H

namespace nodecpp {

template<class T>
class owning_ptr {
	T* ptr;
public:
	owning_ptr(T* ptr = nullptr) :ptr(ptr) {}
	owning_ptr& operator=(const owning_ptr&) = delete;
	void reset(T* ptr) {}

	T* get() { return ptr; }
	const T* get() const { return ptr; }
 	T* operator->() { return ptr; }
 	const T* operator->() const { return ptr; }
	T& operator*() { return *ptr; }
	const T& operator*() const { return *ptr; }
};

template<class T>
class safe_ptr {
	T* ptr;
public:
	safe_ptr(T* ptr = nullptr) :ptr(ptr) {}
	safe_ptr& operator=(const safe_ptr&) = delete;
	void reset(T* ptr) {}

	T* get() { return ptr; }
	const T* get() const { return ptr; }
 	T* operator->() { return ptr; }
 	const T* operator->() const { return ptr; }
	T& operator*() { return *ptr; }
	const T& operator*() const { return *ptr; }
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

	T* get() { return ptr; }
	const T* get() const { return ptr; }
 	T* operator->() { return ptr; }
 	const T* operator->() const { return ptr; }
	T& operator*() { return *ptr; }
	const T& operator*() const { return *ptr; }
};
}


#endif //SAFE_PTR_H
