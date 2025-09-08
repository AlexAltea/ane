// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <string_view>

std::string hex(uint64_t value);

template <typename T>
std::string hex(T value)
{
	return hex(static_cast<uint64_t>(value));
}

template <typename Field>
void print_td_field(std::string_view field_name, Field value, std::string_view fmt)
{
	const auto numeric = static_cast<uint32_t>(value);
	const auto formatted = std::vformat(fmt, std::make_format_args(numeric));
	std::print("      {} = {}\n", field_name, formatted);
}

template <typename BaseField, typename EnableField>
void print_td_enable_field(std::string_view field_name, BaseField base, EnableField enable)
{
	const auto numeric_base = static_cast<uint32_t>(base);
	const bool is_enabled = static_cast<uint32_t>(enable) != 0;
	if (is_enabled) {
		std::print("      {} = Enabled (base={})\n", field_name, numeric_base);
	} else {
		std::print("      {} = Disabled\n", field_name);
	}
}

#define PRINT(scope, field, fmt) ::print_td_field(#field, (scope).field, fmt)
