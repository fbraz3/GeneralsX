/*
**	Command & Conquer Generals(tm)
**	Copyright 2026 Stephan Vedder
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// FILE: MiniAudioManager.cpp
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  MiniAudioManager.cpp                                          */
/* Created:    Stephan Vedder (feliwir), June 2026                           */
/*             Adapted for GeneralsX Core architecture                       */
/* Desc:       Implementation for the MiniAudioManager, interfacing with    */
/*             the miniaudio Sound System.                                   */
/* Reference:  https://github.com/feliwir/CnC_Generals_Zero_Hour            */
/*---------------------------------------------------------------------------*/

#include "Lib/BaseType.h"
#define MINIAUDIO_IMPLEMENTATION
#include "MiniAudioDevice/MiniAudioManager.h"
#include "MiniAudioDevice/MiniAudioStream.h"

#include "Common/AudioAffect.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/AudioRequest.h"
#include "Common/AudioSettings.h"
#include "Common/AsciiString.h"
#include "Common/AudioEventInfo.h"
#include "Common/FileSystem.h"
#include "Common/GameCommon.h"
#include "Common/GameSounds.h"
#include "Common/CRCDebug.h"
#include "Common/GlobalData.h"

#include "GameClient/DebugDisplay.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/View.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/TerrainLogic.h"

#include "Common/File.h"

#ifdef _INTERNAL
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

enum { INFINITE_LOOP_COUNT = 1000000 };

// Callback functions for miniaudio's virtual file system
static ma_result vfsFileOpen(ma_vfs *pVFS, const char *filename, ma_uint32 mode, ma_vfs_file *pFile);
static ma_result vfsFileClose(ma_vfs *pVFS, ma_vfs_file file);
static ma_result vfsFileInfo(ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo);
static ma_result vfsFileRead(ma_vfs *pVFS, ma_vfs_file file, void *pBuffer, size_t bytesToRead, size_t *pBytesRead);
static ma_result vfsFileTell(ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor);
static ma_result vfsFileSeek(ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin);

//-------------------------------------------------------------------------------------------------
MiniAudioManager::MiniAudioManager() :
	m_playbackDeviceCount(0),
	m_selectedPlaybackDevice(PROVIDER_ERROR),
	m_selectedSpeakerType(0),
	m_lastSelectedPlaybackDevice(PROVIDER_ERROR),
	m_binkHandle(NULL),
	m_pref3DProvider(AsciiString::TheEmptyString),
	m_prefSpeaker(AsciiString::TheEmptyString)
{
}

//-------------------------------------------------------------------------------------------------
MiniAudioManager::~MiniAudioManager()
{
	DEBUG_ASSERTCRASH(m_binkHandle == NULL, ("Leaked a Bink handle. Chuybregts"));
	releaseHandleForBink();
	closeDevice();

	DEBUG_ASSERTCRASH(this == TheAudio, ("Umm...\n"));
	TheAudio = NULL;
}

//-------------------------------------------------------------------------------------------------
#if defined(_DEBUG) || defined(_INTERNAL)
AudioHandle MiniAudioManager::addAudioEvent(const AudioEventRTS *eventToAdd)
{
	if (TheGlobalData->m_preloadReport) {
		if (!eventToAdd->getEventName().isEmpty()) {
			m_allEventsLoaded.insert(eventToAdd->getEventName());
		}
	}

	return AudioManager::addAudioEvent(eventToAdd);
}
#endif

#if defined(_DEBUG) || defined(_INTERNAL)
//-------------------------------------------------------------------------------------------------
void MiniAudioManager::audioDebugDisplay(DebugDisplayInterface *dd, void *, FILE *fp)
{
	std::list<PlayingAudio *>::iterator it;

	Coord3D lookPos;
	TheTacticalView->getPosition(&lookPos);
	lookPos.z = TheTerrainLogic->getGroundHeight(lookPos.x, lookPos.y);
	const Coord3D *mikePos = TheAudio->getListenerPosition();
	Coord3D distanceVector = TheTacticalView->get3DCameraPosition();
	distanceVector.sub(mikePos);

	Int now = TheGameLogic->getFrame();
	static Int lastCheck = now;
	const Int frames = 60;

	if (dd)
	{
		dd->printf("MiniAudio version: %s    ", ma_version_string());
		dd->printf("Sound: %s    ", (isOn(AudioAffect_Sound) ? "Yes" : "No"));
		dd->printf("3DSound: %s    ", (isOn(AudioAffect_Sound3D) ? "Yes" : "No"));
		dd->printf("Speech: %s    ", (isOn(AudioAffect_Speech) ? "Yes" : "No"));
		dd->printf("Music: %s\n", (isOn(AudioAffect_Music) ? "Yes" : "No"));
		dd->printf("Channels Available: ");
		dd->printf("%d Sounds    ", m_sound->getAvailableSamples());

		dd->printf("Volume: ");
		dd->printf("Sound: %d    ", REAL_TO_INT(m_soundVolume * 100.0f));
		dd->printf("3DSound: %d    ", REAL_TO_INT(m_sound3DVolume * 100.0f));
		dd->printf("Speech: %d    ", REAL_TO_INT(m_speechVolume * 100.0f));
		dd->printf("Music: %d\n", REAL_TO_INT(m_musicVolume * 100.0f));
		dd->printf("Current 3D Provider: %s    ",
			TheAudio->getProviderName(getSelectedProvider()).str());
		dd->printf("Current Speaker Type: %s\n", TheAudio->translateUnsignedIntToSpeakerType(TheAudio->getSpeakerType()).str());

		dd->printf("Looking at: (%d,%d,%d) -- Microphone at: (%d,%d,%d)\n",
			(Int)lookPos.x, (Int)lookPos.y, (Int)lookPos.z, (Int)mikePos->x, (Int)mikePos->y, (Int)mikePos->z);
		dd->printf("Camera distance from microphone: %d -- Zoom Volume: %d%%\n",
			(Int)distanceVector.length(), (Int)(TheAudio->getZoomVolume() * 100.0f));

		dd->printf("-----------------------------------------------------------\n");
		dd->printf("Playing Audio\n");
	}
	if (fp)
	{
		fprintf(fp, "MiniAudio version: %s    ", ma_version_string());
		fprintf(fp, "Sound: %s    ", (isOn(AudioAffect_Sound) ? "Yes" : "No"));
		fprintf(fp, "3DSound: %s    ", (isOn(AudioAffect_Sound3D) ? "Yes" : "No"));
		fprintf(fp, "Speech: %s    ", (isOn(AudioAffect_Speech) ? "Yes" : "No"));
		fprintf(fp, "Music: %s\n", (isOn(AudioAffect_Music) ? "Yes" : "No"));
		fprintf(fp, "Channels Available: ");
		fprintf(fp, "%d Sounds    ", m_sound->getAvailableSamples());
		fprintf(fp, "Volume: ");
		fprintf(fp, "Sound: %d    ", REAL_TO_INT(m_soundVolume * 100.0f));
		fprintf(fp, "3DSound: %d    ", REAL_TO_INT(m_sound3DVolume * 100.0f));
		fprintf(fp, "Speech: %d    ", REAL_TO_INT(m_speechVolume * 100.0f));
		fprintf(fp, "Music: %d\n", REAL_TO_INT(m_musicVolume * 100.0f));
		fprintf(fp, "Current 3D Provider: %s    ", TheAudio->getProviderName(getSelectedProvider()).str());
		fprintf(fp, "Current Speaker Type: %s\n", TheAudio->translateUnsignedIntToSpeakerType(TheAudio->getSpeakerType()).str());

		fprintf(fp, "Looking at: (%d,%d,%d) -- Microphone at: (%d,%d,%d)\n",
			(Int)lookPos.x, (Int)lookPos.y, (Int)lookPos.z, (Int)mikePos->x, (Int)mikePos->y, (Int)mikePos->z);
		fprintf(fp, "Camera distance from microphone: %d -- Zoom Volume: %d%%\n",
			(Int)distanceVector.length(), (Int)(TheAudio->getZoomVolume() * 100.0f));

		fprintf(fp, "-----------------------------------------------------------\n");
		fprintf(fp, "Playing Audio\n");
	}

	PlayingAudio *playing = NULL;
	Real volume = 0.0f;
	AsciiString filenameNoSlashes;

	// 2-D & 3-D Sounds
	if (dd)
	{
		dd->printf("-----------------------------------------------------Sounds\n");
		int i = 0;
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			playing = *it;
			if (!playing) {
				dd->printf("%d: Silence\n", i);
				continue;
			}

			filenameNoSlashes = playing->m_audioEventRTS->getFilename();
			filenameNoSlashes = filenameNoSlashes.reverseFind('\\') + 1;

			// Calculate Sample volume
			volume = 100.0f;
			volume *= getEffectiveVolume(playing->m_audioEventRTS);

			dd->printf("%2d: %-20s - (%s) Volume: %d (2D)\n", i++, playing->m_audioEventRTS->getEventName().str(), filenameNoSlashes.str(), REAL_TO_INT(volume));
		}
	}
	if (fp)
	{
		fprintf(fp, "-----------------------------------------------------Sounds\n");
		int i = 0;
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			playing = *it;
			if (!playing)
			{
				continue;
			}
			filenameNoSlashes = playing->m_audioEventRTS->getFilename();
			filenameNoSlashes = filenameNoSlashes.reverseFind('\\') + 1;

			// Calculate Sample volume
			volume = 100.0f;
			volume *= getEffectiveVolume(playing->m_audioEventRTS);

			fprintf(fp, "%2d: %-20s - (%s) Volume: %d (2D)\n", i++, playing->m_audioEventRTS->getEventName().str(), filenameNoSlashes.str(), REAL_TO_INT(volume));
		}
	}

	const Coord3D *microphonePos = TheAudio->getListenerPosition();
}
#endif

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::init()
{
	AudioManager::init();
#ifdef INTENSE_DEBUG
	DEBUG_LOG(("Sound has temporarily been disabled in debug builds only. jkmcd\n"));
	// for now, _DEBUG builds only should have no sound. ask jkmcd or srj about this.
	return;
#endif

	// We should now know how many samples we want to load
	openDevice();
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::postProcessLoad()
{
	AudioManager::postProcessLoad();
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::reset()
{
#if defined(_DEBUG) || defined(_INTERNAL)
	dumpAllAssetsUsed();
	m_allEventsLoaded.clear();
#endif

	AudioManager::reset();
	stopAllAudioImmediately();
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::update()
{
	AudioManager::update();
	setDeviceListenerPosition();
	processRequestList();
	processPlayingList();
	processFadingList();
	processStoppedList();
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAudio(AudioAffect which)
{
	if (BitIsSet(which, AudioAffect_Sound)) {
		ma_sound_group_stop(&m_soundGroup);
	}

	if (BitIsSet(which, AudioAffect_Sound3D)) {
		ma_sound_group_stop(&m_sound3DGroup);
	}

	if (BitIsSet(which, AudioAffect_Speech)) {
		ma_sound_group_stop(&m_speechGroup);
	}

	if (BitIsSet(which, AudioAffect_Music)) {
		ma_sound_group_stop(&m_musicGroup);
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::pauseAudio(AudioAffect which)
{
	if (BitIsSet(which, AudioAffect_Sound)) {
		ma_sound_group_stop(&m_soundGroup);
	}

	if (BitIsSet(which, AudioAffect_Sound3D)) {
		ma_sound_group_stop(&m_sound3DGroup);
	}

	if (BitIsSet(which, AudioAffect_Speech)) {
		ma_sound_group_stop(&m_speechGroup);
	}

	if (BitIsSet(which, AudioAffect_Music)) {
		ma_sound_group_stop(&m_musicGroup);
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::resumeAudio(AudioAffect which)
{
	if (BitIsSet(which, AudioAffect_Sound)) {
		ma_sound_group_start(&m_soundGroup);
	}

	if (BitIsSet(which, AudioAffect_Sound3D)) {
		ma_sound_group_start(&m_sound3DGroup);
	}

	if (BitIsSet(which, AudioAffect_Speech)) {
		ma_sound_group_start(&m_speechGroup);
	}

	if (BitIsSet(which, AudioAffect_Music)) {
		ma_sound_group_start(&m_musicGroup);
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::pauseAmbient(Bool shouldPause)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAllAmbientsBy(Object *obj)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAllAmbientsBy(Drawable *draw)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::playAudioEvent(AudioEventRTS *event)
{
	const AudioEventInfo *info = event->getAudioEventInfo();
	if (!info) {
		return;
	}

	AsciiString fileToPlay = event->getFilename();
	fprintf(stderr, "AUDIO: playAudioEvent '%s' type=%d file='%s'\n",
		event->getEventName().str(), info->m_soundType, fileToPlay.str());

#ifdef INTENSIVE_AUDIO_DEBUG

	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing = NULL;

	AudioHandle handleToKill = event->getHandleToKill();
	AsciiString fileToPlay = event->getFilename();
	PlayingAudio *audio = allocatePlayingAudio();
	audio->m_audioEventRTS = event;

	Bool foundSoundToReplace = false;
	if (handleToKill) {
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			playing = (*it);
			if (!playing) {
				continue;
			}

			if (playing->m_audioEventRTS && playing->m_audioEventRTS->getPlayingHandle() == handleToKill)
			{
				// Release this streaming channel immediately because we are going to play another sound in its place.
				releasePlayingAudio(playing);
				m_playingSounds.erase(it);
				foundSoundToReplace = true;
				break;
			}
		}
	}

	ma_sound_group *groupToUse = NULL;
	ma_uint32 flags = 0;
	switch (info->m_soundType)
	{
	case AT_Music:
		groupToUse = &m_musicGroup;
		flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH;
		break;
	case AT_Streaming:
		groupToUse = &m_speechGroup;
		flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH;
		break;
	case AT_SoundEffect:
		if (event->isPositionalAudio()) {
			groupToUse = &m_sound3DGroup;
		}
		else {
			groupToUse = &m_soundGroup;
			flags = MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH;
		}
		break;
	}

	ma_sound *sound = NULL;
	if (!handleToKill || foundSoundToReplace) {
		sound = (ma_sound *)malloc(sizeof(ma_sound));
		ma_result result = ma_sound_init_from_file(&m_engine, fileToPlay.str(), flags, groupToUse, NULL, sound);
		if (result != MA_SUCCESS) {
			DEBUG_LOG(("Failed to initialize sound from file: %s (error: %d)", fileToPlay.str(), result));
			releasePlayingAudio(audio);
			return;
		}
	}
	else {
		DEBUG_LOG(("Skipping sound!"));
		releasePlayingAudio(audio);
		return;
	}

	switch (info->m_soundType)
	{
	case AT_Music:
	case AT_Streaming:
	{
#ifdef INTENSIVE_AUDIO_DEBUG
		DEBUG_LOG(("- Stream\n"));
#endif

		if ((info->m_soundType == AT_Streaming) && event->getUninterruptible()) {
			stopAllSpeech();
		}

		// Put this on here, so that the audio event RTS will be cleaned up regardless.
		audio->m_sound = sound;
		audio->m_type = PAT_Stream;

		if ((info->m_soundType == AT_Streaming) && event->getUninterruptible()) {
			setDisallowSpeech(TRUE);
		}
		break;
	}

	case AT_SoundEffect:
	{
#ifdef INTENSIVE_AUDIO_DEBUG
		DEBUG_LOG(("- Sound"));
#endif

		if (event->isPositionalAudio()) {
			// Sounds that are non-global are positional 3-D sounds. Deal with them accordingly
#ifdef INTENSIVE_AUDIO_DEBUG
			DEBUG_LOG((" Positional"));
#endif
			// Push it onto the list of playing things
			audio->m_sound = sound;
			audio->m_type = PAT_3DSample;

			// Set the position values of the sample here
			if (event->getAudioEventInfo()->m_type & ST_GLOBAL) {
				ma_sound_set_min_distance(sound, TheAudio->getAudioSettings()->m_globalMinRange);
				ma_sound_set_max_distance(sound, TheAudio->getAudioSettings()->m_globalMaxRange);
			}
			else {
				ma_sound_set_min_distance(sound, event->getAudioEventInfo()->m_minDistance);
				ma_sound_set_max_distance(sound, event->getAudioEventInfo()->m_maxDistance);
			}
			const Coord3D *pos = event->getCurrentPosition();
			ma_sound_set_position(sound, pos->x, pos->y, pos->z);
			m_sound->notifyOf3DSampleStart();
		}
		else
		{
			// Push it onto the list of playing things
			audio->m_sound = sound;
			audio->m_type = PAT_Sample;
			m_sound->notifyOf2DSampleStart();
#ifdef INTENSIVE_AUDIO_DEBUG
			DEBUG_LOG((" Playing.\n"));
#endif
		}
		break;
	}
	}

	// Set initial volume based on sound type
	Real initialVolume = event->getVolume();
	if (info->m_soundType == AT_Music) {
		initialVolume *= m_musicVolume;
	}
	else if (info->m_soundType == AT_Streaming) {
		initialVolume *= m_speechVolume;
	}
	else if (event->isPositionalAudio()) {
		initialVolume *= m_sound3DVolume;
	}
	else {
		initialVolume *= m_soundVolume;
	}
	ma_sound_set_volume(sound, initialVolume);

	ma_result result = ma_sound_start(sound);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to start sound. Error code: %d", result));
		releasePlayingAudio(audio);
		return;
	}
	m_playingSounds.push_back(audio);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAudioEvent(AudioHandle handle)
{
#ifdef INTENSIVE_AUDIO_DEBUG
	DEBUG_LOG(("MINIAUDIO (%d) - Processing stop request: %d\n", TheGameLogic->getFrame(), handle));
#endif

	std::list<PlayingAudio *>::iterator it;

	// Handle special music stop commands
	if (handle == AHSV_StopTheMusic || handle == AHSV_StopTheMusicFade) {
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
			PlayingAudio *audio = (*it);
			if (!audio || !audio->m_audioEventRTS) {
				continue;
			}

			const AudioEventInfo *stopInfo = audio->m_audioEventRTS->getAudioEventInfo();
			if (stopInfo && stopInfo->m_soundType == AT_Music)
			{
				if (handle == AHSV_StopTheMusicFade)
				{
					m_fadingAudio.push_back(audio);
				}
				else
				{
					releasePlayingAudio(audio);
				}
				m_playingSounds.erase(it);
				break;
			}
		}
	}

	// Find and mark the audio event for stopping
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		PlayingAudio *audio = (*it);
		if (!audio || !audio->m_audioEventRTS) {
			continue;
		}

		if (audio->m_audioEventRTS->getPlayingHandle() == handle) {
			audio->m_requestStop = true;
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::killAudioEventImmediately(AudioHandle audioEvent)
{
	// First look for it in the request list.
	std::list<AudioRequest *>::iterator ait;
	for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ait++)
	{
		AudioRequest *req = (*ait);
		if (req && req->m_request == AR_Play && req->m_handleToInteractOn == audioEvent)
		{
			deleteInstance(req);
			ait = m_audioRequests.erase(ait);
			return;
		}
	}

	// Look for matching sound to kill
	std::list<PlayingAudio *>::iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); it++)
	{
		PlayingAudio *audio = (*it);
		if (!audio)
		{
			continue;
		}

		if (audio->m_audioEventRTS && audio->m_audioEventRTS->getPlayingHandle() == audioEvent)
		{
			releasePlayingAudio(audio);
			m_playingSounds.erase(it);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::pauseAudioEvent(AudioHandle handle)
{
	// pause audio
}

//-------------------------------------------------------------------------------------------------
PlayingAudio *MiniAudioManager::allocatePlayingAudio(void)
{
	PlayingAudio *aud = NEW PlayingAudio;
	// poolify
	return aud;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::releaseMiniAudioHandles(PlayingAudio *release)
{
	if (release->m_sound) {
		ma_sound_uninit(release->m_sound);
		free(release->m_sound);
	}
	release->m_type = PAT_INVALID;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::releasePlayingAudio(PlayingAudio *release)
{
	if (release->m_audioEventRTS && release->m_audioEventRTS->getAudioEventInfo()) {
		if (release->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_SoundEffect && release->m_sound) {
			if (release->m_type == PAT_Sample) {
				m_sound->notifyOf2DSampleCompletion();
			}
			else {
				m_sound->notifyOf3DSampleCompletion();
			}
		}
	}
	releaseMiniAudioHandles(release);
	// forces stop of this audio
	if (release->m_cleanupAudioEventRTS) {
		releaseAudioEventRTS(release->m_audioEventRTS);
	}
	delete release;
	release = NULL;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAllAudioImmediately(void)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ) {
		playing = *it;
		if (!playing) {
			continue;
		}

		releasePlayingAudio(playing);
		it = m_playingSounds.erase(it);
	}

	for (it = m_fadingAudio.begin(); it != m_fadingAudio.end(); ) {
		playing = *it;
		if (!playing) {
			continue;
		}

		releasePlayingAudio(playing);
		it = m_fadingAudio.erase(it);
	}

	for (it = m_stoppedAudio.begin(); it != m_stoppedAudio.end(); ) {
		playing = *it;
		if (!playing) {
			continue;
		}

		releasePlayingAudio(playing);
		it = m_stoppedAudio.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::freeAllMiniAudioHandles(void)
{
	ma_engine_stop(&m_engine);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::adjustPlayingVolume(PlayingAudio *audio)
{
	if (!audio->m_audioEventRTS)
		return;
	Real desiredVolume = audio->m_audioEventRTS->getVolume() * audio->m_audioEventRTS->getVolumeShift();
	if (audio->m_sound) {
		if (audio->m_type == PAT_Sample) {
			ma_sound_set_volume(audio->m_sound, m_soundVolume * desiredVolume);
		}
		else if (audio->m_type == PAT_3DSample) {
			ma_sound_set_volume(audio->m_sound, m_sound3DVolume * desiredVolume);
		}
		else if (audio->m_type == PAT_Stream) {
			const AudioEventInfo *info = audio->m_audioEventRTS->getAudioEventInfo();
			if (info && info->m_soundType == AT_Music) {
				ma_sound_set_volume(audio->m_sound, m_musicVolume * desiredVolume);
			}
			else {
				ma_sound_set_volume(audio->m_sound, m_speechVolume * desiredVolume);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::stopAllSpeech(void)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ) {
		playing = (*it);
		if (!playing) {
			continue;
		}

		if (playing->m_audioEventRTS && playing->m_audioEventRTS->getAudioEventInfo() &&
			playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Streaming) {
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else {
			++it;
		}
	}
}

//-------------------------------------------------------------------------------------------------
AsciiString MiniAudioManager::nextMusicTrack(void)
{
	AsciiString trackName;
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getAudioEventInfo() &&
			playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			trackName = playing->m_audioEventRTS->getEventName();
		}
	}

	// Stop currently playing music
	TheAudio->removeAudioEvent(AHSV_StopTheMusic);

	trackName = nextTrackName(trackName);
	AudioEventRTS newTrack(trackName);
	TheAudio->addAudioEvent(&newTrack);

	return trackName;
}

//-------------------------------------------------------------------------------------------------
AsciiString MiniAudioManager::prevMusicTrack(void)
{
	AsciiString trackName;
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getAudioEventInfo() &&
			playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			trackName = playing->m_audioEventRTS->getEventName();
		}
	}

	// Stop currently playing music
	TheAudio->removeAudioEvent(AHSV_StopTheMusic);

	trackName = prevTrackName(trackName);
	AudioEventRTS newTrack(trackName);
	TheAudio->addAudioEvent(&newTrack);

	return trackName;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isMusicPlaying(void) const
{
	return ma_sound_group_is_playing(&m_musicGroup) == MA_TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const
{
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
AsciiString MiniAudioManager::getMusicTrackName(void) const
{
	// First check the requests. If there's one there, then report that as the currently playing track.
	std::list<AudioRequest *>::const_iterator ait;
	for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ++ait) {
		if ((*ait)->m_request != AR_Play) {
			continue;
		}

		if (!(*ait)->m_usePendingEvent) {
			continue;
		}

		if ((*ait)->m_pendingEvent->getAudioEventInfo()->m_soundType == AT_Music) {
			return (*ait)->m_pendingEvent->getEventName();
		}
	}

	std::list<PlayingAudio *>::const_iterator it;
	PlayingAudio *playing;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getAudioEventInfo() &&
			playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Music) {
			return playing->m_audioEventRTS->getEventName();
		}
	}

	return AsciiString::TheEmptyString;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::openDevice(void)
{
	if (!TheGlobalData->m_audioOn) {
		return;
	}

	ma_result result;
	ma_resource_manager_config resourceManagerConfig;
	ma_context_config contextConfig;
	ma_engine_config engineConfig;
	static ma_vfs_callbacks vfs = {};
	vfs.onOpen = vfsFileOpen;
	vfs.onClose = vfsFileClose;
	vfs.onInfo = vfsFileInfo;
	vfs.onRead = vfsFileRead;
	vfs.onTell = vfsFileTell;
	vfs.onSeek = vfsFileSeek;

	// Use a custom resource manager, so we can load from our virtual file system.
	resourceManagerConfig = ma_resource_manager_config_init();
	resourceManagerConfig.pVFS = (ma_vfs *)&vfs;
	result = ma_resource_manager_init(&resourceManagerConfig, &m_resourceManager);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to initialize miniaudio resource manager. Error code: %d\n", result));
		// if we couldn't initialize any devices, turn sound off (fail silently)
		setOn(false, AudioAffect_All);
		return;  // Failed to initialize the resource manager.
	}

	result = ma_log_init(NULL, &m_log);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to initialize miniaudio log. Error code: %d\n", result));
		setOn(false, AudioAffect_All);
		return;
	}

	auto on_log = [](void *pUserData, ma_uint32 logLevel, const char *message) {
		DEBUG_LOG(("miniaudio: %s - %s", ma_log_level_to_string(logLevel), message));
	};
	ma_log_register_callback(&m_log, ma_log_callback_init(on_log, NULL));
	contextConfig = ma_context_config_init();
	contextConfig.pLog = &m_log;

	/* We're going to want a context so we can enumerate our playback devices. */
	result = ma_context_init(NULL, 0, &contextConfig, &m_context);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to initialize miniaudio context. Error code: %d\n", result));
		setOn(false, AudioAffect_All);
		return;
	}

	/*
	Now that we have a context we will want to enumerate over each device so we can display them to the user and give
	them a chance to select the output devices they want to use.
	*/
	result = ma_context_get_devices(&m_context, &m_playbackDevices, &m_playbackDeviceCount, NULL, NULL);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to enumerate miniaudio playback devices. Error code: %d\n", result));
		ma_context_uninit(&m_context);
		setOn(false, AudioAffect_All);
		return;
	}

	engineConfig = ma_engine_config_init();
	engineConfig.pResourceManager = &m_resourceManager;

	result = ma_engine_init(&engineConfig, &m_engine);
	if (result != MA_SUCCESS) {
		DEBUG_LOG(("Failed to initialize the Miniaudio engine. Error code: %d\n", result));
		// if we couldn't initialize any devices, turn sound off (fail silently)
		setOn(false, AudioAffect_All);
		return;  // Failed to initialize the engine.
	}

	ma_sound_group_init(&m_engine, 0, NULL, &m_musicGroup);
	ma_sound_group_init(&m_engine, 0, NULL, &m_soundGroup);
	ma_sound_group_init(&m_engine, 0, NULL, &m_sound3DGroup);
	ma_sound_group_init(&m_engine, 0, NULL, &m_speechGroup);

	// Log the audio backend information for diagnostics
	fprintf(stderr, "AUDIO: MiniAudio backend loaded - version %s, device: %s, playback devices: %d\n",
		ma_version_string(),
		m_playbackDeviceCount > 0 ? m_playbackDevices[0].name : "default",
		m_playbackDeviceCount);

	// Now that we're all done, update the cached variables so that everything is in sync.
	TheAudio->refreshCachedVariables();
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::closeDevice(void)
{
	ma_sound_group_uninit(&m_speechGroup);
	ma_sound_group_uninit(&m_sound3DGroup);
	ma_sound_group_uninit(&m_soundGroup);
	ma_sound_group_uninit(&m_musicGroup);
	ma_engine_uninit(&m_engine);
	ma_resource_manager_uninit(&m_resourceManager);
	ma_context_uninit(&m_context);
	ma_log_uninit(&m_log);
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isCurrentlyPlaying(AudioHandle handle)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getPlayingHandle() == handle) {
			return true;
		}
	}

	// if something is requested, it is also considered playing
	std::list<AudioRequest *>::iterator ait;
	AudioRequest *req = NULL;
	for (ait = m_audioRequests.begin(); ait != m_audioRequests.end(); ++ait) {
		req = *ait;
		if (req && req->m_usePendingEvent && req->m_pendingEvent->getPlayingHandle() == handle) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags)
{
	PlayingAudio *playing = findPlayingAudioFrom(audioCompleted, flags);
	if (!playing) {
		DEBUG_CRASH(("Audio has completed playing, but we can't seem to find it. - jkmcd"));
		return;
	}

	if (!playing->m_audioEventRTS || !playing->m_audioEventRTS->getAudioEventInfo())
		return;

	if (getDisallowSpeech() && playing->m_audioEventRTS->getAudioEventInfo()->m_soundType == AT_Streaming) {
		setDisallowSpeech(FALSE);
	}

	if (playing->m_audioEventRTS->getAudioEventInfo()->m_control & AC_LOOP) {
		if (playing->m_audioEventRTS->getNextPlayPortion() == PP_Attack) {
			playing->m_audioEventRTS->setNextPlayPortion(PP_Sound);
		}
		if (playing->m_audioEventRTS->getNextPlayPortion() == PP_Sound) {
			// First, decrease the loop count.
			playing->m_audioEventRTS->decreaseLoopCount();

			// Now, try to start the next loop
			if (startNextLoop(playing)) {
				return;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
PlayingAudio *MiniAudioManager::findPlayingAudioFrom(UnsignedInt audioCompleted, UnsignedInt flags)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	ma_sound *sound = (ma_sound *)audioCompleted;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_sound == sound) {
			return playing;
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getProviderCount(void) const
{
	return m_playbackDeviceCount;
}

//-------------------------------------------------------------------------------------------------
AsciiString MiniAudioManager::getProviderName(UnsignedInt providerNum) const
{
	if (isOn(AudioAffect_Sound3D) && providerNum < m_playbackDeviceCount) {
		return m_playbackDevices[providerNum].name;
	}

	return AsciiString::TheEmptyString;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getProviderIndex(AsciiString providerName) const
{
	for (UnsignedInt i = 0; i < m_playbackDeviceCount; ++i) {
		if (providerName == m_playbackDevices[i].name) {
			return i;
		}
	}
	return PROVIDER_ERROR;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::selectProvider(UnsignedInt providerNdx)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::unselectProvider(void)
{
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getSelectedProvider(void) const
{
	return m_selectedPlaybackDevice;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::setSpeakerType(UnsignedInt speakerType)
{
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getSpeakerType(void)
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
void *MiniAudioManager::getHandleForBink(void)
{
	if (!m_binkHandle) {
		DEBUG_LOG(("Creating MiniAudio bink stream\n"));
		m_binkHandle = NEW MiniAudioStream;
	}
	return m_binkHandle;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::releaseHandleForBink(void)
{
	if (m_binkHandle) {
		DEBUG_LOG(("Releasing MiniAudio bink stream\n"));
		delete (MiniAudioStream *)m_binkHandle;
		m_binkHandle = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getNum2DSamples(void) const
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getNum3DSamples(void) const
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
UnsignedInt MiniAudioManager::getNumStreams(void) const
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::doesViolateLimit(AudioEventRTS *event) const
{
	Int limit = event->getAudioEventInfo()->m_limit;
	if (limit == 0) {
		return false;
	}

	Int totalCount = 0;
	Int totalRequestCount = 0;

	std::list<PlayingAudio *>::const_iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		if (!(*it)->m_audioEventRTS) continue;
		if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
			if (totalCount == 0) {
				// This is the oldest audio of this type playing.
				event->setHandleToKill((*it)->m_audioEventRTS->getPlayingHandle());
			}
			++totalCount;
		}
	}

	// Also check the request list in case we've requested to play this sound.
	std::list<AudioRequest *>::const_iterator arIt;
	for (arIt = m_audioRequests.begin(); arIt != m_audioRequests.end(); ++arIt) {
		AudioRequest *req = (*arIt);
		if (req == NULL) {
			continue;
		}
		if (req->m_usePendingEvent)
		{
			if (req->m_pendingEvent->getEventName() == event->getEventName())
			{
				totalRequestCount++;
				totalCount++;
			}
		}
	}

	// If our event is an interrupting type, then normally we would always add it.
	if (event->getAudioEventInfo()->m_control & AC_INTERRUPT)
	{
		if (totalRequestCount < limit)
		{
			Int totalPlayingCount = totalCount - totalRequestCount;
			if (totalRequestCount + totalPlayingCount < limit)
			{
				event->setHandleToKill(0);
				return false;
			}
			return false;
		}
	}

	if (totalCount < limit)
	{
		event->setHandleToKill(0);
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isPlayingAlready(AudioEventRTS *event) const
{
	std::list<PlayingAudio *>::const_iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		if (!(*it)->m_audioEventRTS) continue;
		if ((*it)->m_audioEventRTS->getEventName() == event->getEventName()) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isObjectPlayingVoice(UnsignedInt objID) const
{
	if (objID == 0) {
		return false;
	}

	std::list<PlayingAudio *>::const_iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		if (!(*it)->m_audioEventRTS) continue;
		const AudioEventInfo *info = (*it)->m_audioEventRTS->getAudioEventInfo();
		if (info && (*it)->m_audioEventRTS->getObjectID() == objID && (info->m_type & ST_VOICE)) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
AudioEventRTS *MiniAudioManager::findLowestPrioritySound(AudioEventRTS *event)
{
	AudioPriority priority = event->getAudioEventInfo()->m_priority;
	if (priority == AP_LOWEST)
	{
		return NULL;
	}
	AudioEventRTS *lowestPriorityEvent = NULL;
	AudioPriority lowestPriority;

	std::list<PlayingAudio *>::const_iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
	{
		AudioEventRTS *itEvent = (*it)->m_audioEventRTS;
		if (!itEvent) continue;
		const AudioEventInfo *itInfo = itEvent->getAudioEventInfo();
		if (!itInfo) continue;
		AudioPriority itPriority = itInfo->m_priority;
		if (itPriority < priority)
		{
			if (!lowestPriorityEvent || lowestPriority > itPriority)
			{
				lowestPriorityEvent = itEvent;
				lowestPriority = itPriority;
				if (lowestPriority == AP_LOWEST)
				{
					return lowestPriorityEvent;
				}
			}
		}
	}
	return lowestPriorityEvent;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isPlayingLowerPriority(AudioEventRTS *event) const
{
	AudioPriority priority = event->getAudioEventInfo()->m_priority;
	if (priority == AP_LOWEST)
	{
		return false;
	}
	std::list<PlayingAudio *>::const_iterator it;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		if (!(*it)->m_audioEventRTS) continue;
		const AudioEventInfo *info = (*it)->m_audioEventRTS->getAudioEventInfo();
		if (info && info->m_priority < priority) {
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::killLowestPrioritySoundImmediately(AudioEventRTS *event)
{
	AudioEventRTS *lowestPriorityEvent = findLowestPrioritySound(event);
	if (lowestPriorityEvent)
	{
		std::list<PlayingAudio *>::iterator it;
		for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
		{
			PlayingAudio *playing = (*it);
			if (!playing)
			{
				continue;
			}

			if (playing->m_audioEventRTS && playing->m_audioEventRTS == lowestPriorityEvent)
			{
				releasePlayingAudio(playing);
				m_playingSounds.erase(it);
				return TRUE;
			}
		}
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume)
{
	std::list<PlayingAudio *>::iterator it;

	PlayingAudio *playing = NULL;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it) {
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getEventName() == eventName) {
			// Adjust it
			playing->m_audioEventRTS->setVolume(newVolume);
			Real desiredVolume = playing->m_audioEventRTS->getVolume() * playing->m_audioEventRTS->getVolumeShift();
			if (playing->m_sound) {
				if (playing->m_type == PAT_Sample) {
					ma_sound_set_volume(playing->m_sound, m_soundVolume * desiredVolume);
				}
				else if (playing->m_type == PAT_3DSample) {
					ma_sound_set_volume(playing->m_sound, m_sound3DVolume * desiredVolume);
				}
				else if (playing->m_type == PAT_Stream) {
					const AudioEventInfo *info = playing->m_audioEventRTS->getAudioEventInfo();
					if (info && info->m_soundType == AT_Music) {
						ma_sound_set_volume(playing->m_sound, m_musicVolume * desiredVolume);
					}
					else {
						ma_sound_set_volume(playing->m_sound, m_speechVolume * desiredVolume);
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::removePlayingAudio(AsciiString eventName)
{
	std::list<PlayingAudio *>::iterator it;

	PlayingAudio *playing = NULL;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); )
	{
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getEventName() == eventName)
		{
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else
		{
			it++;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::removeAllDisabledAudio(void)
{
	std::list<PlayingAudio *>::iterator it;

	PlayingAudio *playing = NULL;
	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); )
	{
		playing = *it;
		if (playing && playing->m_audioEventRTS && playing->m_audioEventRTS->getVolume() == 0.0f)
		{
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else
		{
			it++;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::processRequestList(void)
{
	std::list<AudioRequest *>::iterator it;
	for (it = m_audioRequests.begin(); it != m_audioRequests.end(); /* empty */) {
		AudioRequest *req = (*it);
		if (req == NULL) {
			continue;
		}

		if (!shouldProcessRequestThisFrame(req)) {
			adjustRequest(req);
			++it;
			continue;
		}

		if (!req->m_requiresCheckForSample || checkForSample(req)) {
			processRequest(req);
		}
		deleteInstance(req);
		it = m_audioRequests.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::processPlayingList(void)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	for (it = m_playingSounds.begin(); it != m_playingSounds.end(); /* empty */) {
		playing = (*it);
		if (!playing) {
			it = m_playingSounds.erase(it);
			continue;
		}

		if (playing->m_sound && !ma_sound_is_playing(playing->m_sound))
		{
			releasePlayingAudio(playing);
			it = m_playingSounds.erase(it);
		}
		else
		{
			if (m_volumeHasChanged)
			{
				adjustPlayingVolume(playing);
			}
			++it;
		}
	}

	if (m_volumeHasChanged) {
		m_volumeHasChanged = false;
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::processFadingList(void)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	for (it = m_fadingAudio.begin(); it != m_fadingAudio.end(); /* empty */) {
		playing = *it;
		if (!playing) {
			continue;
		}

		if (playing->m_framesFaded >= getAudioSettings()->m_fadeAudioFrames) {
			playing->m_requestStop = true;
			releasePlayingAudio(playing);
			it = m_fadingAudio.erase(it);
			continue;
		}

		++playing->m_framesFaded;
		Real volume = getEffectiveVolume(playing->m_audioEventRTS);
		volume *= (1.0f - 1.0f * playing->m_framesFaded / getAudioSettings()->m_fadeAudioFrames);

		if (playing->m_sound) {
			ma_sound_set_volume(playing->m_sound, volume);
		}

		++it;
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::processStoppedList(void)
{
	std::list<PlayingAudio *>::iterator it;
	PlayingAudio *playing;

	for (it = m_stoppedAudio.begin(); it != m_stoppedAudio.end(); /* empty */) {
		playing = *it;
		if (playing) {
			releasePlayingAudio(playing);
		}
		it = m_stoppedAudio.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::shouldProcessRequestThisFrame(AudioRequest *req) const
{
	if (!req->m_usePendingEvent) {
		return true;
	}

	if (req->m_pendingEvent->getDelay() < MSEC_PER_LOGICFRAME_REAL) {
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::adjustRequest(AudioRequest *req)
{
	if (!req->m_usePendingEvent) {
		return;
	}

	req->m_pendingEvent->decrementDelay(MSEC_PER_LOGICFRAME_REAL);
	req->m_requiresCheckForSample = true;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::checkForSample(AudioRequest *req)
{
	if (!req->m_usePendingEvent) {
		return true;
	}

	if (req->m_pendingEvent->getAudioEventInfo() == NULL)
	{
		// Fill in event info
		getInfoForAudioEvent(req->m_pendingEvent);
	}

	if (req->m_pendingEvent->getAudioEventInfo()->m_type != AT_SoundEffect)
	{
		return true;
	}

	return m_sound->canPlayNow(req->m_pendingEvent);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::setHardwareAccelerated(Bool accel)
{
	AudioManager::setHardwareAccelerated(accel);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::setSpeakerSurround(Bool surround)
{
	AudioManager::setSpeakerSurround(surround);
}

//-------------------------------------------------------------------------------------------------
Real MiniAudioManager::getFileLengthMS(AsciiString strToLoad) const
{
	if (strToLoad.isEmpty()) {
		return 0.0f;
	}

	// Use miniaudio's decoder to get file length via VFS
	ma_decoder decoder;
	ma_decoder_config config = ma_decoder_config_init_default();

	// Use our custom VFS for file access
	static ma_vfs_callbacks vfs = {};
	vfs.onOpen = vfsFileOpen;
	vfs.onClose = vfsFileClose;
	vfs.onInfo = vfsFileInfo;
	vfs.onRead = vfsFileRead;
	vfs.onTell = vfsFileTell;
	vfs.onSeek = vfsFileSeek;

	ma_result result = ma_decoder_init_vfs((ma_vfs *)&vfs, strToLoad.str(), &config, &decoder);
	if (result != MA_SUCCESS) {
		return 0.0f;
	}

	// Get the length in frames
	ma_uint64 lengthInFrames;
	result = ma_decoder_get_length_in_pcm_frames(&decoder, &lengthInFrames);
	if (result != MA_SUCCESS) {
		ma_decoder_uninit(&decoder);
		return 0.0f;
	}

	// Convert to milliseconds
	ma_uint32 sampleRate = decoder.outputSampleRate;
	if (sampleRate == 0) sampleRate = 44100; // fallback
	float lengthMS = (float)lengthInFrames / (float)sampleRate * 1000.0f;

	ma_decoder_uninit(&decoder);
	return lengthMS;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::closeAnySamplesUsingFile(const void *fileToClose)
{
	// For miniaudio, we don't track individual file handles the same way as OpenAL.
	// This is a no-op for now since miniaudio handles file management internally.
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::has3DSensitiveStreamsPlaying(void) const
{
	if (m_playingSounds.empty())
		return FALSE;

	for (std::list<PlayingAudio *>::const_iterator it = m_playingSounds.begin(); it != m_playingSounds.end(); ++it)
	{
		const PlayingAudio *playing = (*it);

		if (!playing || !playing->m_audioEventRTS)
			continue;

		// Only check streams (PAT_Stream type)
		if (playing->m_type != PAT_Stream)
			continue;

		const AudioEventInfo *info = playing->m_audioEventRTS->getAudioEventInfo();
		if (!info)
			continue;

		// Non-music streams are always 3D sensitive
		if (info->m_soundType != AT_Music)
		{
			return TRUE;
		}

		// Music streams that don't start with "Game_" are 3D sensitive
		if (playing->m_audioEventRTS->getEventName().startsWith("Game_") == FALSE)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::startNextLoop(PlayingAudio *looping)
{
	if (looping->m_requestStop) {
		return false;
	}

	if (looping->m_audioEventRTS->hasMoreLoops()) {
		// generate a new filename, and test to see whether we can play with it now
		looping->m_audioEventRTS->generateFilename();

		if (looping->m_audioEventRTS->getDelay() > MSEC_PER_LOGICFRAME_REAL) {
			// fake it out so that this sound appears done, but also so that it will not
			// delete the sound on completion (which would suck)
			looping->m_cleanupAudioEventRTS = false;
			looping->m_requestStop = true;

			AudioRequest *req = allocateAudioRequest(true);
			req->m_pendingEvent = looping->m_audioEventRTS;
			req->m_requiresCheckForSample = true;
			appendAudioRequest(req);
			return true;
		}

		// Re-initialize the sound from the new file
		if (looping->m_sound) {
			ma_sound_uninit(looping->m_sound);
		}

		AsciiString fileToPlay = looping->m_audioEventRTS->getFilename();
		ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH;
		if (looping->m_type == PAT_3DSample) {
			flags = 0; // Allow spatialization for 3D
		}

		ma_result result = ma_sound_init_from_file(&m_engine, fileToPlay.str(), flags,
			(looping->m_type == PAT_3DSample) ? &m_sound3DGroup : &m_soundGroup,
			NULL, looping->m_sound);

		if (result == MA_SUCCESS) {
			Real loopVolume = looping->m_audioEventRTS->getVolume();
			if (looping->m_type == PAT_Sample) {
				loopVolume *= m_soundVolume;
			}
			else if (looping->m_type == PAT_3DSample) {
				loopVolume *= m_sound3DVolume;
			}
			else if (looping->m_type == PAT_Stream) {
				const AudioEventInfo *info = looping->m_audioEventRTS->getAudioEventInfo();
				if (info && info->m_soundType == AT_Music) {
					loopVolume *= m_musicVolume;
				}
				else {
					loopVolume *= m_speechVolume;
				}
			}
			ma_sound_set_volume(looping->m_sound, loopVolume);
			ma_sound_start(looping->m_sound);
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::playStream(AudioEventRTS *event, ma_sound *sound)
{
	ma_sound_start(sound);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::setDeviceListenerPosition(void)
{
	ma_engine_listener_set_position(&m_engine, 0, m_listenerPosition.x, m_listenerPosition.y, m_listenerPosition.z);

	// Set orientation (forward vector)
	ma_engine_listener_set_direction(&m_engine, 0, m_listenerOrientation.x, m_listenerOrientation.y, m_listenerOrientation.z);
}

//-------------------------------------------------------------------------------------------------
const Coord3D *MiniAudioManager::getCurrentPositionFromEvent(AudioEventRTS *event)
{
	if (!event->isPositionalAudio()) {
		return NULL;
	}

	return event->getCurrentPosition();
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isOnScreen(const Coord3D *pos) const
{
	static ICoord2D dummy;
	// WorldToScreen will return True if the point is onscreen and false if it is offscreen.
	return TheTacticalView->worldToScreen(pos, &dummy);
}

//-------------------------------------------------------------------------------------------------
Real MiniAudioManager::getEffectiveVolume(AudioEventRTS *event) const
{
	if (!event)
		return 0.0f;
	Real volume = 1.0f;
	volume *= (event->getVolume() * event->getVolumeShift());
	const AudioEventInfo *evInfo = event->getAudioEventInfo();
	if (evInfo && evInfo->m_soundType == AT_Music)
	{
		volume *= m_musicVolume;
	}
	else if (evInfo && evInfo->m_soundType == AT_Streaming)
	{
		volume *= m_speechVolume;
	}
	else
	{
		if (event->isPositionalAudio())
		{
			volume *= m_sound3DVolume;
		}
		else
		{
			volume *= m_soundVolume;
		}
	}

	return volume;
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::buildProviderList(void)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::createListener(void)
{
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::initDelayFilter(void)
{
}

//-------------------------------------------------------------------------------------------------
Bool MiniAudioManager::isValidProvider(void)
{
	return (m_selectedPlaybackDevice < m_playbackDeviceCount);
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::initSamplePools(void)
{
	if (!(isOn(AudioAffect_Sound3D) && isValidProvider()))
	{
		return;
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::processRequest(AudioRequest *req)
{
	switch (req->m_request)
	{
	case AR_Play:
	{
		playAudioEvent(req->m_pendingEvent);
		break;
	}
	case AR_Pause:
	{
		pauseAudioEvent(req->m_handleToInteractOn);
		break;
	}
	case AR_Stop:
	{
		stopAudioEvent(req->m_handleToInteractOn);
		break;
	}
	}
}

//-------------------------------------------------------------------------------------------------
void MiniAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS *eventToPlay)
{
	if (!eventToPlay->getAudioEventInfo()) {
		getInfoForAudioEvent(eventToPlay);
		if (!eventToPlay->getAudioEventInfo()) {
			DEBUG_CRASH(("No info for forced audio event '%s'\n", eventToPlay->getEventName().str()));
			return;
		}
	}

	switch (eventToPlay->getAudioEventInfo()->m_soundType)
	{
	case AT_Music:
		if (!isOn(AudioAffect_Music))
			return;
		break;
	case AT_SoundEffect:
		if (!isOn(AudioAffect_Sound) || !isOn(AudioAffect_Sound3D))
			return;
		break;
	case AT_Streaming:
		if (!isOn(AudioAffect_Speech))
			return;
		break;
	}

	AudioEventRTS *event = NEW AudioEventRTS(*eventToPlay);

	event->generateFilename();
	event->generatePlayInfo();

	std::list<std::pair<AsciiString, Real> >::iterator it;
	for (it = m_adjustedVolumes.begin(); it != m_adjustedVolumes.end(); ++it) {
		if (it->first == event->getEventName()) {
			event->setVolume(it->second);
			break;
		}
	}

	playAudioEvent(event);
}

#if defined(_DEBUG) || defined(_INTERNAL)
//-------------------------------------------------------------------------------------------------
void MiniAudioManager::dumpAllAssetsUsed()
{
	if (!TheGlobalData->m_preloadReport) {
		return;
	}

	// Dump all the audio assets we've used.
	FILE *logfile = fopen("PreloadedAssets.txt", "a+"); //append to log
	if (!logfile)
		return;

	std::list<AsciiString> missingEvents;
	std::list<AsciiString> usedFiles;

	std::list<AsciiString>::iterator lit;

	fprintf(logfile, "\nAudio Asset Report - BEGIN\n");
	{
		SetAsciiStringIt it;
		std::vector<AsciiString>::iterator asIt;
		for (it = m_allEventsLoaded.begin(); it != m_allEventsLoaded.end(); ++it) {
			AsciiString astr = *it;
			AudioEventInfo *aei = findAudioEventInfo(astr);
			if (!aei) {
				missingEvents.push_back(astr);
				continue;
			}

			for (asIt = aei->m_attackSounds.begin(); asIt != aei->m_attackSounds.end(); ++asIt) {
				usedFiles.push_back(*asIt);
			}

			for (asIt = aei->m_sounds.begin(); asIt != aei->m_sounds.end(); ++asIt) {
				usedFiles.push_back(*asIt);
			}

			for (asIt = aei->m_decaySounds.begin(); asIt != aei->m_decaySounds.end(); ++asIt) {
				usedFiles.push_back(*asIt);
			}

			if (!aei->m_filename.isEmpty()) {
				usedFiles.push_back(aei->m_filename);
			}
		}

		fprintf(logfile, "\nEvents Requested that are missing information - BEGIN\n");
		for (lit = missingEvents.begin(); lit != missingEvents.end(); ++lit) {
			fprintf(logfile, "%s\n", (*lit).str());
		}
		fprintf(logfile, "\nEvents Requested that are missing information - END\n");

		fprintf(logfile, "\nFiles Used - BEGIN\n");
		for (lit = usedFiles.begin(); lit != usedFiles.end(); ++lit) {
			fprintf(logfile, "%s\n", (*lit).str());
		}
		fprintf(logfile, "\nFiles Used - END\n");
	}
	fprintf(logfile, "\nAudio Asset Report - END\n");
	fclose(logfile);
	logfile = NULL;
}
#endif

//-------------------------------------------------------------------------------------------------
// VFS callbacks for miniaudio - allow loading files from the game's virtual file system
//-------------------------------------------------------------------------------------------------

struct VFSFileHandle
{
	File *file;
};

static ma_result vfsFileOpen(ma_vfs *pVFS, const char *filename, ma_uint32 mode, ma_vfs_file *pFile)
{
	VFSFileHandle *handle = NEW VFSFileHandle;
	handle->file = TheFileSystem->openFile(filename);
	if (!handle->file) {
		delete handle;
		return MA_ERROR;
	}
	*pFile = (ma_vfs_file)handle;
	return MA_SUCCESS;
}

static ma_result vfsFileClose(ma_vfs *pVFS, ma_vfs_file file)
{
	VFSFileHandle *handle = (VFSFileHandle *)file;
	if (handle) {
		if (handle->file) {
			handle->file->close();
		}
		delete handle;
	}
	return MA_SUCCESS;
}

static ma_result vfsFileInfo(ma_vfs *pVFS, ma_vfs_file file, ma_file_info *pInfo)
{
	VFSFileHandle *handle = (VFSFileHandle *)file;
	if (!handle || !handle->file) {
		return MA_ERROR;
	}
	pInfo->sizeInBytes = handle->file->size();
	return MA_SUCCESS;
}

static ma_result vfsFileRead(ma_vfs *pVFS, ma_vfs_file file, void *pBuffer, size_t bytesToRead, size_t *pBytesRead)
{
	VFSFileHandle *handle = (VFSFileHandle *)file;
	if (!handle || !handle->file) {
		return MA_ERROR;
	}
	Int bytesRead = handle->file->read(pBuffer, bytesToRead);
	if (pBytesRead) {
		*pBytesRead = (size_t)bytesRead;
	}
	return MA_SUCCESS;
}

static ma_result vfsFileTell(ma_vfs *pVFS, ma_vfs_file file, ma_int64 *pCursor)
{
	VFSFileHandle *handle = (VFSFileHandle *)file;
	if (!handle || !handle->file) {
		return MA_ERROR;
	}
	*pCursor = (ma_int64)handle->file->position();
	return MA_SUCCESS;
}

static ma_result vfsFileSeek(ma_vfs *pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin)
{
	VFSFileHandle *handle = (VFSFileHandle *)file;
	if (!handle || !handle->file) {
		return MA_ERROR;
	}

	File::seekMode seekType;
	switch (origin) {
	case ma_seek_origin_start:
		seekType = File::START;
		break;
	case ma_seek_origin_current:
		seekType = File::CURRENT;
		break;
	case ma_seek_origin_end:
		seekType = File::END;
		break;
	default:
		return MA_ERROR;
	}

	handle->file->seek((Int)offset, seekType);
	return MA_SUCCESS;
}
