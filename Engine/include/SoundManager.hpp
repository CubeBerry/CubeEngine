//Author: DOYEONG LEE
//Project: CubeEngine
//File: SoundManager.hpp
#pragma once
#ifndef BG_SOUNDMANAGER_H
#define BG_SOUNDMANAGER_H

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "fmod.hpp"

struct PlaybackTime
{
	int minutes = 0;
	int seconds = 0;
};

struct Sound
{
	FMOD::Sound* sound = nullptr;
	std::wstring nameL;
	std::string name;
};

struct Channel
{
	FMOD::Channel* channel = nullptr;
	float soundVolume = 0.5f;
};

class SoundManager {
public:
	SoundManager() = default;
	~SoundManager();

	void Initialize(int maxChannel);
	void Update();
	void Shutdown();
	
	void ClearSounds();

	void LoadFile(std::string filepath, std::string name, bool loop = false);
	void LoadFile(std::wstring filepath, std::wstring name, bool loop = false);

	void Play(std::string name, int channelIndex, bool loop = false);
	void Play(int index, int channelIndex, bool loop = false);
	
	void PlaySoundInArea2D(std::string name, int channelIndex, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f, bool loop = false);
	void UpdateSoundVolumeInArea2D(int channelIndex, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f);
	void PlaySoundIn3D(std::string name, int channelIndex, glm::vec3 pos = { 0.f,0.f,0.f }, float minDis = 0.f, float maxDis = 1.f, bool loop = false); //wip
	void SetListenerPosition(glm::vec3 pos);

	void Stop(int channelIndex);
	void Pause(int channelIndex, FMOD_BOOL state);
	void Pause(int channelIndex);

	bool IsPlaying(int channelIndex);
	bool IsPaused(int channelIndex);

	void MoveSoundPlaybackPosition(int channelIndex, int currentSoundIndex, float pos);
	float GetSoundPlaybackPosition(int channelIndex, int currentSoundIndex);
	PlaybackTime GetSoundPlaybackTime(int channelIndex, int currentSoundIndex, float pos);

	void VolumeUp(int channelIndex);
	void VolumeDown(int channelIndex);
	void SetVolume(int channelIndex, float volume);
	float GetChannelVolume(int channelIndex) { return channels.at(channelIndex).soundVolume; }

	void LoadSoundFilesFromFolder(const std::string& folderPath);
	void LoadSoundFilesFromFolder(const std::wstring& folderPath);

	std::vector<Sound>& GetSoundList() { return sounds; }
	int GetAmontOfSounds() { return soundMaxIndex; }
	void MusicPlayerForImGui(int channelIndex);
private:
	void ErrorCheck(FMOD_RESULT result_);
	int FindSoundIndexWithName(std::string name);

	std::vector<Sound> sounds;
	std::vector<Channel> channels;

	int soundMaxIndex = 0;

	FMOD_VECTOR listenerPosition = { 0.f,0.f,0.f };

	FMOD::System* system;
	FMOD_RESULT result;

	int currentIndex = 0;
	bool isAdjusting = false;
};
#endif