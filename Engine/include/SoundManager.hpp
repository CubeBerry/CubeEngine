//Author: DOYEONG LEE
//Project: CubeEngine
//File: SoundManager.hpp
#pragma once
#ifndef BG_SOUNDMANAGER_H
#define BG_SOUNDMANAGER_H
const int MAX_SOUND_TRACK = 36;

#include <string>
#include <iostream>
#include <map>
#include <vector>

#include "fmod.h"
#include "glm/glm.hpp"

struct PlaybackTime
{
	int minutes = 0;
	int seconds = 0;
};

struct Music
{
	FMOD_SOUND* music = nullptr;
	FMOD_CHANNEL* channel = nullptr;
	std::wstring nameL;
	std::string name;
};

class SoundManager {
public:
	SoundManager() {};
	~SoundManager();

	void Initialize();
	void Shutdown();
	
	void ClearMusicList();

	void ErrorCheck(FMOD_RESULT result_);

	void LoadSoundFile(std::string filepath, std::string name, bool loop = false);
	void LoadMusicFile(std::wstring filepath, std::wstring name, bool loop = true);

	void PlaySound(std::string name, bool loop = false);
	void PlaySoundInArea2D(std::string name, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f, bool loop = false);
	void UpdateSoundVolumeInArea2D(std::string name, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f);
	void PlaySoundIn3D(std::string name, glm::vec3 pos = { 0.f,0.f,0.f }, float minDis = 0.f, float maxDis = 1.f); //wip
	void StopSound(std::string name);
	void PauseSound(std::string name, FMOD_BOOL state); // need to modify with isplaying(with channel)

	void PlayMusic(int index, bool loop = true);
	void PlayMusicInArea2D(int index, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f, bool loop = true);
	void UpdateMusicVolumeInArea2D(int index, glm::vec2 pos = { 0.f,0.f }, float maxDis = 1.f);
	void PlayMusicIn3D(int index, glm::vec3 pos = { 0.f,0.f,0.f }, float minDis = 0.f, float maxDis = 1.f); //wip
	void StopMusic(int index);
	void PauseMusic(int index);
	bool IsPlaying(int index);
	bool IsPaused(int index);
	void MoveMusicPlaybackPosition(int index, float pos);
	float GetMusicPlaybackPosition(int index);
	PlaybackTime GetMusicPlaybackTime(int index, float pos);

	void SoundVolumeUp();
	void SoundVolumeDown();

	void MusicVolumeUp();
	void MusicVolumeDown();

	float GetSoundVolume() { return soundVolume; }
	float GetMusicVolume() { return musicVolume; }

	void SetSoundVolume(float amount); // need to modify with isplaying(with channel)
	void SetMusicVolume(float amount);

	void SetListenerPosition(glm::vec3 pos);

	void LoadSoundFilesFromFolder(const std::string& folderPath);
	void LoadMusicFilesFromFolder(const std::wstring& folderPath);

	std::vector<Music>& GetMusicList() { return Musics; }
	int GetAmontOfMusics() { return musicMaxIndex; }

#ifdef _DEBUG
	void MusicPlayerForImGui();
#endif
private:

	std::map<std::string, FMOD_SOUND*> Sounds;
	std::map<std::string, FMOD_CHANNEL*> Channels;

	std::vector<Music> Musics;
	int musicMaxIndex = 0;

	FMOD_VECTOR listenerPosition = { 0.f,0.f,0.f };

	FMOD_SYSTEM* system = nullptr;
	FMOD_RESULT result;

	float soundVolume = 0.5f;
	float musicVolume = 0.5f;

#ifdef _DEBUG
	int currentIndex = 0;
	bool isAdjusting = false;
#endif
};

#endif