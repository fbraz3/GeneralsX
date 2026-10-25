// GeneralsX @feature GeneralsOnline NGMP types definition
#pragma once

#include <cstdint>
#include <string>

class NetworkMemberBase
{
public:
	int64_t user_id = -1;
	std::string display_name;

	bool m_bIsHost = false;
	bool m_bIsReady = false;
	bool m_bIsAdmin = false;

	std::string sort_key;
};

enum class ELoginResult
{
    Success,
    Failed,
    UserCancelled
};
