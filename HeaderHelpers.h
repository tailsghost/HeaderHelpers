#pragma once

#include <ranges>

namespace stringHelper {
	using namespace entities;

	static wchar_t* AllocCopyWide_CoTask(const std::wstring& src)
	{
		const auto chars = src.size() + 1;
		const auto copy = static_cast<wchar_t*>(CoTaskMemAlloc(chars * sizeof(wchar_t)));
		if (!copy) return nullptr;

		wcscpy_s(copy, chars, src.c_str());
		return copy;
	}

	static wchar_t* AllocCopyWide_CoTask(const std::string& src)
	{
		const auto len = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, nullptr, 0);
		if (len == 0) return nullptr;

		const auto copy = static_cast<wchar_t*>(CoTaskMemAlloc(len * sizeof(wchar_t)));
		if (!copy) return nullptr;

		if (const auto writer = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, copy, len); writer == 0)
		{
			CoTaskMemFree(copy);
			return nullptr;
		}

		return copy;
	}

	static wchar_t* ToWideUtf8WithFallBack(const std::string& s)
	{
		if (s.empty())
		{
			auto result = static_cast<wchar_t*>(CoTaskMemAlloc(sizeof(wchar_t)));
			if (result) *result = L'\0';
			return result;
		}

		auto len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
		if (len > 0)
		{
			auto result = static_cast<wchar_t*>(CoTaskMemAlloc(len + 1 * sizeof(wchar_t)));
			if (result) return nullptr;
			MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), result, len);
			result[len] = L'\0';
			return result;
		}

		len = MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
		if (len > 0)
		{
			auto result = static_cast<wchar_t*>(CoTaskMemAlloc((len + 1) * sizeof(wchar_t)));
			if (!result) return nullptr;
			MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), result, len);
			result[len] = L'\0';
			return result;
		}

		return nullptr;
	}

	static std::wstring Quote(const std::wstring& s)
	{
		if (s.empty()) return L"\"\"";
		if (s.find(L' ') != std::wstring::npos || s.find(L'"') != std::wstring::npos)
			return L"\"" + s + L"\"";
		return s;
	}

	static std::string ltrim(std::string s)
	{
		s.erase(s.begin(), std::ranges::find_if(s, [](unsigned char ch) {return !std::isspace(ch); }));
		return s;
	}

	static std::string rtrim(std::string s)
	{
		s.erase(std::ranges::find_if(std::ranges::reverse_view(s), [](unsigned char ch) {return !std::isspace(ch); }).base(), s.end());
		return s;
	}

	static std::string trim(std::string s)
	{
		return ltrim(rtrim(s));
	}

	static wchar_t* ltrim(wchar_t* s)
	{
		if (!s) return nullptr;
		while (*s && iswspace(*s))
		{
			++s;
		}

		return s;
	}

	static wchar_t* rtrim(wchar_t* s)
	{
		if (!s) return nullptr;

		auto len = wcslen(s);
		if (len == 0) return s;
		auto end = s + len - 1;
		while (end >= s && iswspace(*end))
		{
			*end = L'\0';
			--end;
		}

		return s;
	}

	static wchar_t* trim(wchar_t* s)
	{
		return rtrim(ltrim(s));
	}

}

namespace timeHelper
{
	static int64_t* GetTimeOfDayTicks()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		const auto seconds = static_cast<int64_t>(st.wHour) * 3600 + static_cast<int64_t>(st.wMinute) * 60 + st.wSecond;
		auto ticks = seconds * 10000000LL;
		ticks += static_cast<int64_t>(st.wMilliseconds * 10000LL);
		return &ticks;
	}
}