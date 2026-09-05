// GeneralsX @feature GeneralsOnline AuthInterface header
#pragma once

#include "GameNetwork/GeneralsOnline/NGMP_types.h"
#include <string>
#include <functional>
#include <cstdint>

class NGMP_OnlineServices_AuthInterface
{
public:
	NGMP_OnlineServices_AuthInterface();

	std::string GetDisplayName();
	std::wstring GetDisplayNameW();
	int64_t GetUserID() const;
	std::string GetAuthToken() const;

	bool IsLoggedIn() const;

	void BeginLogin();
	void LogoutOfMyAccount();

	void RegisterForLoginCallback(std::function<void(ELoginResult)> callback)
	{
		m_cb_LoginPendingCallback = callback;
	}
	void DeregisterForLoginCallback()
	{
		m_cb_LoginPendingCallback = nullptr;
	}

	void InvokeLoginCallback(ELoginResult result)
	{
		if (m_cb_LoginPendingCallback)
			m_cb_LoginPendingCallback(result);
	}

private:
	std::function<void(ELoginResult)> m_cb_LoginPendingCallback = nullptr;
};
