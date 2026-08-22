// GeneralsX @feature GeneralsOnline SocialInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_SocialInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"

NGMP_OnlineServices_SocialInterface::NGMP_OnlineServices_SocialInterface()
{
}

bool NGMP_OnlineServices_SocialInterface::IsUserFriend(int64_t target_user_id) const
{
	return m_friends.find(target_user_id) != m_friends.end();
}

bool NGMP_OnlineServices_SocialInterface::IsUserIgnored(int64_t target_user_id) const
{
	return m_ignored.find(target_user_id) != m_ignored.end();
}

void NGMP_OnlineServices_SocialInterface::AddFriend(int64_t target_user_id)
{
	m_friends.insert(target_user_id);
}

void NGMP_OnlineServices_SocialInterface::RemoveFriend(int64_t target_user_id)
{
	m_friends.erase(target_user_id);
}

void NGMP_OnlineServices_SocialInterface::IgnoreUser(int64_t target_user_id)
{
	m_ignored.insert(target_user_id);
}

void NGMP_OnlineServices_SocialInterface::UnignoreUser(int64_t target_user_id)
{
	m_ignored.erase(target_user_id);
}
