// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2023 1BitSquared <info@1bitsquared.com>
// SPDX-FileContributor: Written by Rachel Mant <git@dragonmux.network>
#include <substrate/console>
#include "windows/serialInterface.hxx"
#include "bmp.hxx"

using substrate::console;

[[nodiscard]] std::string serialForDevice(const usbDevice_t &device)
{
	const auto serialIndex{device.serialNumberIndex()};
	if (serialIndex == 0)
		return {};
	const auto handle{device.open()};
	return handle.readStringDescriptor(serialIndex);
}

void displayError(const LSTATUS error, const std::string_view operation, const std::string_view path)
{
	char *message{nullptr};
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
		error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char *>(&message), 0, nullptr);
	console.error("Failed to "sv, operation, ' ', path, ", got error "sv, asHex_t<8, '0'>{uint32_t(error)},
		": "sv, message);
	LocalFree(message);
}

struct hklmRegistryKey_t
{
private:
	HKEY handle{static_cast<HKEY>(INVALID_HANDLE_VALUE)};

public:
	hklmRegistryKey_t() noexcept = default;
	hklmRegistryKey_t(const hklmRegistryKey_t &) = delete;
	hklmRegistryKey_t operator =(const hklmRegistryKey_t &) = delete;

	hklmRegistryKey_t(const std::string &path, const REGSAM permissions) :
		handle
		{
			[&]()
			{
				auto keyHandle{static_cast<HKEY>(INVALID_HANDLE_VALUE)};
				if (const auto result{RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, permissions, &keyHandle)};
					result != ERROR_SUCCESS)
				{
					displayError(result, "open registry key"sv, path);
					return static_cast<HKEY>(INVALID_HANDLE_VALUE);
				}
				return keyHandle;
			}()
		} { }

	~hklmRegistryKey_t() noexcept
	{
		if (handle != INVALID_HANDLE_VALUE)
			RegCloseKey(handle);
	}

	[[nodiscard]] bool valid() const noexcept { return handle != INVALID_HANDLE_VALUE; }

	[[nodiscard]] std::string readStringKey(std::string_view keyName) const
	{
		DWORD valueLength{0U};
		if (const auto result{RegGetValue(handle, nullptr, keyName.data(), RRF_RT_REG_SZ, nullptr, nullptr, &valueLength)};
			result != ERROR_SUCCESS && result != ERROR_MORE_DATA)
		{
			displayError(result, "retrieve value for key"sv, keyName);
			return {};
		}

		std::string value(valueLength, '\0');
		if (const auto result{RegGetValue(handle, nullptr, keyName.data(), RRF_RT_REG_SZ, nullptr, value.data(),
			&valueLength)}; result != ERROR_SUCCESS)
		{
			displayError(result, "retrieve value for key"sv, keyName);
			return {};
		}
		return value;
	}
};

serialInterface_t::serialInterface_t(const usbDevice_t &usbDevice)
{
}

serialInterface_t::~serialInterface_t() noexcept
{
}

void serialInterface_t::swap(serialInterface_t &interface) noexcept
{
	std::swap(device, interface.device);
}

void serialInterface_t::writePacket(const std::string_view &packet) const
{
}

std::string serialInterface_t::readPacket() const
{
	return "";
}
