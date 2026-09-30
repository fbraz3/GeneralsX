// GeneralsX @feature GeneralsOnline NGMPGame header
#pragma once

#include "GameNetwork/GameInfo.h"
#include "GameNetwork/GeneralsOnline/NGMP_types.h"
#include <chrono>

struct LobbyEntry;

class NGMPGameSlot : public GameSlot
{
public:
	NGMPGameSlot();
	Int getProfileID(void) const { return m_profileID; }
	void setProfileID(Int id) { m_profileID = id; }
	Int getWins(void) const { return m_wins; }
	Int getLosses(void) const { return m_losses; }
	void setWins(Int wins) { m_wins = wins; }
	void setLosses(Int losses) { m_losses = losses; }

	Int getSlotRankPoints(void) const { return m_rankPoints; }
	Int getFavoriteSide(void) const { return m_favoriteSide; }
	void setSlotRankPoints(Int val) { m_rankPoints = val; }
	void setFavoriteSide(Int val) { m_favoriteSide = val; }

	void setPingString(UnicodeString pingStr) { m_pingStr = pingStr; }
	void setPingString(AsciiString pingStr) { m_pingStr.translate(pingStr); }
	inline UnicodeString getPingString(void) const { return m_pingStr; }
	inline Int getPingAsInt(void) const { return m_pingInt; }

	int64_t m_userID = -1;

	void UpdateLatencyFromConnection(UnicodeString pingStr, int ping)
	{
		m_pingStr = pingStr;
		m_pingInt = ping;
	}

protected:
	Int m_profileID;
	UnicodeString m_pingStr;
	Int m_pingInt;
	Int m_wins, m_losses;
	Int m_rankPoints, m_favoriteSide;
};

class NGMPGame : public GameInfo
{
private:
	NGMPGameSlot m_Slots[MAX_SLOTS];
	UnicodeString m_gameName;
	Int m_id;
	Bool m_requiresPassword;
	Bool m_allowObservers;
	UnsignedInt m_version;
	UnsignedInt m_exeCRC;
	UnsignedInt m_iniCRC;
	Bool m_isQM;

	AsciiString m_ladderIP;
	AsciiString m_pingStr;
	Int m_pingInt;
	UnsignedShort m_ladderPort;

	Int m_reportedNumPlayers;
	Int m_reportedMaxPlayers;
	Int m_reportedNumObservers;

	bool m_bHasCommittedOutcome = false;
	bool m_bCommittingOutcome = false;
	std::chrono::system_clock::time_point matchStartTime;

	bool m_bCountdownStarted = false;
	int64_t m_countdownStartTime = -1;
	int64_t m_countdownLastCheckTime = -1;

public:
	NGMPGame();
	virtual ~NGMPGame();
	virtual void reset(void);

	bool HasCommittedOutcome() const { return m_bHasCommittedOutcome; }
	bool IsCommittingOutcome() const { return m_bCommittingOutcome; }
	void SetCommittingOutcome(bool committing) { m_bCommittingOutcome = committing; }
	void SetHasCommittedOutcome(bool committed = true) { m_bHasCommittedOutcome = committed; m_bCommittingOutcome = false; }

	void StartCountdown();
	void StopCountdown()
	{
		m_bCountdownStarted = false;
		m_countdownStartTime = -1;
		m_countdownLastCheckTime = -1;
	}
	bool IsCountdownStarted() const { return m_bCountdownStarted; }
	int64_t GetCountdownStartTime() const { return m_countdownStartTime; }
	void UpdateCountdownLastCheckTime()
	{
		m_countdownLastCheckTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	int GetTotalCountdownDuration() const { return 5; }
	int64_t GetCountdownLastCheckTime() const { return m_countdownLastCheckTime; }

	void StartMatchTimer() { matchStartTime = std::chrono::system_clock::now(); }
	std::chrono::system_clock::time_point GetStartTime() { return matchStartTime; }

	void SyncWithLobby(LobbyEntry& lobby);
	void UpdateSlotsFromCurrentLobby();

	void cleanUpSlotPointers(void);
	inline void setID(Int id) { m_id = id; }
	inline Int getID(void) const { return m_id; }

	inline void setHasPassword(Bool val) { m_requiresPassword = val; }
	inline Bool getHasPassword(void) const { return m_requiresPassword; }
	inline void setAllowObservers(Bool val) { m_allowObservers = val; }
	inline Bool getAllowObservers(void) const { return m_allowObservers; }
	Bool canKickOnObserversDisabled();

	inline void setVersion(UnsignedInt val) { m_version = val; }
	inline UnsignedInt getVersion(void) const { return m_version; }
	inline void setExeCRC(UnsignedInt val) { m_exeCRC = val; }
	inline UnsignedInt getExeCRC(void) const { return m_exeCRC; }
	inline void setIniCRC(UnsignedInt val) { m_iniCRC = val; }
	inline UnsignedInt getIniCRC(void) const { return m_iniCRC; }

	inline void setReportedNumPlayers(Int val) { m_reportedNumPlayers = val; }
	inline Int getReportedNumPlayers(void) const { return m_reportedNumPlayers; }
	inline void setReportedMaxPlayers(Int val) { m_reportedMaxPlayers = val; }
	inline Int getReportedMaxPlayers(void) const { return m_reportedMaxPlayers; }
	inline void setReportedNumObservers(Int val) { m_reportedNumObservers = val; }
	inline Int getReportedNumObservers(void) const { return m_reportedNumObservers; }

	inline void setLadderIP(AsciiString ladderIP) { m_ladderIP = ladderIP; }
	inline AsciiString getLadderIP(void) const { return m_ladderIP; }
	inline void setLadderPort(UnsignedShort ladderPort) { m_ladderPort = ladderPort; }
	inline UnsignedShort getLadderPort(void) const { return m_ladderPort; }
	void setPingString(AsciiString pingStr);
	inline AsciiString getPingString(void) const { return m_pingStr; }
	inline Int getPingAsInt(void) const { return m_pingInt; }

	virtual Bool amIHost(void) const;

	NGMPGameSlot* getGameSpySlot(Int index);

	AsciiString generateGameSpyGameResultsPacket(void);
	AsciiString generateLadderGameResultsPacket(void);
	void markGameAsQM(void) { m_isQM = TRUE; }
	Bool isQMGame(void) { return m_isQM; }

	virtual void init(void);
	virtual void resetAccepted(void);
	virtual void startGame(Int gameID);
	void launchGame(void);
	virtual Int getLocalSlotNum(void) const override;

	inline void setGameName(UnicodeString name) { m_gameName = name; }
	inline UnicodeString getGameName(void) const { return m_gameName; }
};

extern NGMPGame* TheNGMPGame;
