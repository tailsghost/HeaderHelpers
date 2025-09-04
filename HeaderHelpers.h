#pragma once

#include <windows.h>
#include <ranges>
#include <string>
#include <vector>

namespace stringHelper {
	using namespace entities;


	static std::wstring utf8ToWstring(const std::string& s) {
		std::wstring out;
		out.reserve(s.size());
		size_t i = 0;
		while (i < s.size()) {
			unsigned char c = static_cast<unsigned char>(s[i]);
			uint32_t codepoint = 0;
			if (c < 0x80) {
				codepoint = c;
				++i;
			}
			else if ((c >> 5) == 0x6) {
				if (i + 1 >= s.size()) break;
				codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
				i += 2;
			}
			else if ((c >> 4) == 0xE) {
				if (i + 2 >= s.size()) break;
				codepoint = ((c & 0x0F) << 12)
					| ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6)
					| (static_cast<unsigned char>(s[i + 2]) & 0x3F);
				i += 3;
			}
			else if ((c >> 3) == 0x1E) {
				if (i + 3 >= s.size()) break;
				codepoint = ((c & 0x07) << 18)
					| ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12)
					| ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6)
					| (static_cast<unsigned char>(s[i + 3]) & 0x3F);
				i += 4;
			}
			else {
				++i;
				continue;
			}

			if (sizeof(wchar_t) == 2) {
				if (codepoint <= 0xFFFF) {
					out.push_back(static_cast<wchar_t>(codepoint));
				}
				else {
					codepoint -= 0x10000;
					wchar_t high = static_cast<wchar_t>((codepoint >> 10) + 0xD800);
					wchar_t low = static_cast<wchar_t>((codepoint & 0x3FF) + 0xDC00);
					out.push_back(high);
					out.push_back(low);
				}
			}
			else {
				out.push_back(static_cast<wchar_t>(codepoint));
			}
		}
		return out;
	};

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

	static std::wstring ToWideBestEffort(const std::string& s) {
		if (s.empty()) return L"";
		auto try_cp = [&](UINT cp)->std::wstring {
			int n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), nullptr, 0);
			if (n <= 0) return L"";
			std::wstring w(n, L'\0');
			n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s.data(), (int)s.size(), &w[0], n);
			if (n <= 0) return L"";
			w.resize(n);
			return w;
			};
		if (auto w = try_cp(CP_UTF8); !w.empty()) return w;
		if (auto w = try_cp(CP_OEMCP); !w.empty()) return w;
		if (auto w = try_cp(CP_ACP); !w.empty()) return w;
		std::wstring w;
		w.reserve(s.size());
		for (unsigned char c : s) w.push_back((wchar_t)c);
		return w;
	}

	static std::string ToStringBestEffort(const std::wstring& line) {
		auto n = WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.size(), nullptr, 0, nullptr, nullptr);
		std::string out;
		if (n > 0) {
			out.resize(n);
			WideCharToMultiByte(CP_UTF8, 0, line.c_str(), (int)line.size(), &out[0], n, nullptr, nullptr);
		}
		out.push_back('\n');
		return out;
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

	static std::wstring vectorUint8ToString(const std::vector<uint8_t>& v) {
		std::string s(v.begin(), v.end());
		return utf8ToWstring(s);
	}

}

namespace timeHelper
{
	static int64_t GetTimeOfDayTicks()
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		const auto seconds = static_cast<int64_t>(st.wHour) * 3600 + static_cast<int64_t>(st.wMinute) * 60 + st.wSecond;
		auto ticks = seconds * 10000000LL;
		ticks += static_cast<int64_t>(st.wMilliseconds * 10000LL);
		return ticks;
	}
}