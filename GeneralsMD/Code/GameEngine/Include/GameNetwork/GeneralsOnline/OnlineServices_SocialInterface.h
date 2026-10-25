// GeneralsX @feature GeneralsOnline SocialInterface header
#pragma once

#include "Common/GameDefines.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>

class NGMP_OnlineServices_SocialInterface
{
public:
	NGMP_OnlineServices_SocialInterface();

	bool IsUserFriend(int64_t target_user_id) const;
	bool IsUserIgnored(int64_t target_user_id) const;

	void AddFriend(int64_t target_user_id);
	void RemoveFriend(int64_t target_user_id);

	void IgnoreUser(int64_t target_user_id);
	void UnignoreUser(int64_t target_user_id);

private:
	std::unordered_set<int64_t> m_friends;
	std::unordered_set<int64_t> m_ignored;
};
