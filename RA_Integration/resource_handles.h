#ifndef RESOURCE_HANDLES_H
#define RESOURCE_HANDLES_H
#pragma once

#include "RA_Defs.h"

// don't care, plus it won't work otherwise


namespace ra {




// Shoot we'll just use char for now it's too much of a pain.
// ..a
// This was made so we don't have to deallocate, call release() if you don't
// want it automatically erased
// ..a
// For comparisons, at least one side has to have the type
// std::string (can be an rvalue)
//template<typename CharT = TCHAR, class = std::enable_if_t<is_char_v<CharT>>>
class cstring_h
{
public:
	using pointer       = char*; // not too big a deal here, but pointer type is required usually
	using const_pointer = const char*;

	inline constexpr cstring_h(pointer chars) noexcept : chars_{ chars } {}
	~cstring_h() noexcept
	{
		chars_ = "";
		chars_ = nullptr;
	}
	inline constexpr auto get() const noexcept
		-> typename const_pointer {
		return chars_;
	}
	inline constexpr void set(const_pointer chars) noexcept { chars_ = chars; }

	auto release() noexcept -> typename const_pointer
	{
		const_pointer _Ans = get();
		my_ptr() = pointer();
		return (_Ans);
	}

	// non-const version of get



	inline auto my_ptr()  noexcept
		-> typename const_pointer& {
		return chars_;
	}
private:
	const_pointer chars_{ nullptr };
};

// to my knowlege there's only ascii for c files, but fstream more or less does this
class file_h
{
	using fhandle      = std::unique_ptr<FILE, decltype(&_CSTD fclose)>;
	using deleter_type = fhandle::deleter_type;
	using element_type = typename std::pointer_traits<fhandle>::element_type;
	using pointer      = typename std::pointer_traits<fhandle>::pointer;
public:
	// A FILE* only uses const char*, but could use others
	// Will throw if it cannot be opened
	inline file_h(cstring_h& filename, cstring_h& open_mode) :
		filename_{ filename },
		fp_{ fopen(filename.get(), open_mode.get()), fclose }
	{
		if (!fp_)
			throw std::runtime_error{ "File was invalid!" };
	}

	// this is just incase we want to make it more generic later
	inline constexpr auto filename() const noexcept
		->typename cstring_h::const_pointer {
		return filename_.get();
	}
private:
	fhandle fp_;
	cstring_h filename_;
};


// This is shouldn't be used for modal dialogs
class window_h
{
	using pointer       = _CSTD HWND;
	using deleter_type  = decltype(&_CSTD DestroyWindow);
	using element_type  = _STD remove_pointer_t<_CSTD HWND>;
	using window_handle = _STD unique_ptr<element_type, deleter_type>;



public:
	window_h(HWND hwnd) { wnd_._Myptr() = hwnd; }
	auto get() noexcept -> typename pointer { return wnd_.get(); }
	void set(HWND hwnd) noexcept {
		window_handle tmp{ hwnd, DestroyWindow };
		wnd_.reset();
		wnd_ = std::move(tmp);
	}
	auto release() noexcept -> typename pointer { return wnd_.release(); }
private:
	// we'll make it better later, we'll just let the api handle it for now
	window_handle wnd_{ CreateWindowEx(0, null, null, 0, 0,0,0,0, null, null,null,null), DestroyWindow };
};


} // namespace ra


#endif // !RESOURCE_HANDLES_H
