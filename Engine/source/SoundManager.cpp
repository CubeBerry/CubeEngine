//Author: DOYEONG LEE
//Project: CubeEngine
//File: SoundManager.cpp
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <cmath>

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>

#include "SoundManager.hpp"
#include "Engine.hpp"
#ifdef _DEBUG
#include "imgui.h"
#endif

std::string ConvertWideStringToUTF8(const std::wstring& wideString)
{
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8String(requiredSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, &utf8String[0], requiredSize, nullptr, nullptr);
	return utf8String;

	/*std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wideString);*/
}

SoundManager::~SoundManager()
{
	Shutdown();
}

void SoundManager::Initialize()
{
	if (system == nullptr)
	{
		result = FMOD_System_Create(&system);
		ErrorCheck(result);
		result = FMOD_System_Init(system, MAX_SOUND_TRACK, FMOD_INIT_NORMAL, NULL);
		ErrorCheck(result);
	}
}

void SoundManager::Shutdown()
{
	for (auto sound : Sounds)
	{
		result = FMOD_Sound_Release(sound.second);
		ErrorCheck(result);
	}
	Channels.clear();

	ClearMusicList();
}

void SoundManager::ClearMusicList()
{
	for (auto sound : Musics)
	{
		result = FMOD_Sound_Release(sound.music);
		ErrorCheck(result);
	}
	Musics.clear();
}

void SoundManager::ErrorCheck(FMOD_RESULT result_)
{
	if (result_ != FMOD_OK)
	{
		exit(-1);
	}
}

void SoundManager::LoadSoundFile(std::string filepath, std::string name, bool loop)
{
	if (loop == true)
	{
		result = FMOD_System_CreateSound(system, filepath.c_str(), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_3D, nullptr, &Sounds[name]);
		ErrorCheck(result);
	}
	else
	{
		result = FMOD_System_CreateSound(system, filepath.c_str(), FMOD_DEFAULT | FMOD_2D | FMOD_3D, nullptr, &Sounds[name]);
		ErrorCheck(result);
	}
	FMOD_CHANNEL* channel = NULL;
	Channels[name] = channel;
}

void SoundManager::LoadMusicFile(std::wstring filepath, std::wstring name, bool loop)
{
	Musics.push_back(Music());
	std::string path = ConvertWideStringToUTF8(filepath);
	if (loop == true)
	{
		result = FMOD_System_CreateSound(system, path.c_str(), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_3D, nullptr, &Musics[musicMaxIndex].music);
		ErrorCheck(result);
	}
	else
	{
		result = FMOD_System_CreateSound(system, path.c_str(), FMOD_DEFAULT | FMOD_2D | FMOD_3D, nullptr, &Musics[musicMaxIndex].music);
		ErrorCheck(result);
	}
	FMOD_CHANNEL* channel = nullptr;
	Musics[musicMaxIndex].channel = channel;
	Musics[musicMaxIndex].name = ConvertWideStringToUTF8(name);
	Musics[musicMaxIndex].nameL = name;
	++musicMaxIndex;
}

void SoundManager::PlaySound(std::string name, bool loop)
{
	FMOD_System_Update(system);
	if (Sounds[name] != nullptr)
	{
		result = FMOD_System_PlaySound(system, Sounds[name], nullptr, false, &Channels[name]);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		FMOD_Sound_SetMode(Sounds[name], mode);

		ErrorCheck(result);
	}
	if (Channels[name] != nullptr)
	{
		result = FMOD_Channel_SetVolume(Channels[name], soundVolume);

		ErrorCheck(result);
	}
}

void SoundManager::PlaySoundInArea2D(std::string name, glm::vec2 pos, float maxDis, bool loop)
{
	FMOD_System_Update(system);
	if (Sounds[name] != nullptr)
	{
		result = FMOD_System_PlaySound(system, Sounds[name], nullptr, false, &Channels[name]);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		FMOD_Sound_SetMode(Sounds[name], mode);

		ErrorCheck(result);
	}
	UpdateSoundVolumeInArea2D(name, pos, maxDis);
}

void SoundManager::UpdateSoundVolumeInArea2D(std::string name, glm::vec2 pos, float maxDis)
{
	if (Channels[name] != nullptr)
	{
		float distance = static_cast<float>(std::sqrt(std::pow(pos.x - listenerPosition.x, 2) + std::pow(pos.y - listenerPosition.y, 2)));
		float volume = 1.0f - std::min(distance / maxDis, 1.0f);

		result = FMOD_Channel_SetVolume(Channels[name], musicVolume * volume);
		ErrorCheck(result);
	}
}

void SoundManager::PlaySoundIn3D(std::string name, glm::vec3 pos, float minDis, float maxDis)
{
	FMOD_VECTOR position = { pos.x, pos.y, pos.z };
	FMOD_VECTOR velocity = { 0.f, 0.f, 0.f };

	FMOD_System_Update(system);
	if (Sounds[name] != nullptr)
	{
		result = FMOD_System_PlaySound(system, Sounds[name], nullptr, false, &Channels[name]);
		result = FMOD_Sound_Set3DMinMaxDistance(Sounds[name], minDis, maxDis);
		ErrorCheck(result);
	}
	if (Channels[name] != nullptr)
	{
		result = FMOD_Channel_SetVolume(Channels[name], soundVolume);
		result = FMOD_Channel_Set3DAttributes(Channels[name], &position, &velocity);
		ErrorCheck(result);
	}
}

void SoundManager::StopSound(std::string name)
{
	if (Channels[name] != nullptr)
	{
		result = FMOD_Channel_Stop(Channels[name]);
		ErrorCheck(result);
	}
}

void SoundManager::PauseSound(std::string name, FMOD_BOOL state)
{
	if (Channels[name] != nullptr)
	{
		result = FMOD_Channel_SetPaused(Channels[name], state);
		ErrorCheck(result);
	}
}

void SoundManager::PlayMusic(int index, bool loop)
{
	FMOD_System_Update(system);
	if (Musics[index].music != nullptr)
	{
		result = FMOD_System_PlaySound(system, Musics[index].music, nullptr, false, &Musics[index].channel);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		FMOD_Sound_SetMode(Musics[index].music, mode);

		ErrorCheck(result);
	}
	if (Musics[index].channel != nullptr)
	{
		result = FMOD_Channel_SetVolume(Musics[index].channel, musicVolume);
		ErrorCheck(result);
	}
}

void SoundManager::PlayMusicInArea2D(int index, glm::vec2 pos, float maxDis, bool loop)
{
	FMOD_System_Update(system);
	if (Musics[index].music != nullptr)
	{
		result = FMOD_System_PlaySound(system, Musics[index].music, nullptr, false, &Musics[index].channel);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		FMOD_Sound_SetMode(Musics[index].music, mode);

		ErrorCheck(result);
	}
	UpdateMusicVolumeInArea2D(index, pos, maxDis);
}

void SoundManager::UpdateMusicVolumeInArea2D(int index, glm::vec2 pos, float maxDis)
{
	if (Musics[index].channel != nullptr)
	{
		float distance = static_cast<float>(std::sqrt(std::pow(pos.x - listenerPosition.x, 2) + std::pow(pos.y - listenerPosition.y, 2)));
		float volume = 1.0f - std::min(distance / maxDis, 1.0f);

		result = FMOD_Channel_SetVolume(Musics[index].channel, musicVolume * volume);
		ErrorCheck(result);
	}
}

void SoundManager::PlayMusicIn3D(int index, glm::vec3 pos, float minDis, float maxDis)
{
	FMOD_VECTOR position = { pos.x, pos.y, pos.z };
	FMOD_VECTOR velocity = { 0.f, 0.f, 0.f };

	FMOD_System_Update(system);
	if (Musics[index].music != nullptr)
	{
		result = FMOD_System_PlaySound(system, Musics[index].music, nullptr, false, &Musics[index].channel);
		result = FMOD_Sound_Set3DMinMaxDistance(Musics[index].music, minDis, maxDis);
		ErrorCheck(result);
	}
	if (Musics[index].channel != nullptr)
	{
		result = FMOD_Channel_SetVolume(Musics[index].channel, musicVolume);
		result = FMOD_Channel_Set3DAttributes(Musics[index].channel, &position, &velocity);
		ErrorCheck(result);
	}
}

void SoundManager::StopMusic(int index)
{
	FMOD_BOOL isplay;
	if (Musics[index].channel != nullptr)
	{
		FMOD_Channel_IsPlaying(Musics[index].channel, &isplay);
		if (isplay == static_cast<FMOD_BOOL>(true))
		{
			result = FMOD_Channel_Stop(Musics[index].channel);
			ErrorCheck(result);
		}
	}
}

void SoundManager::PauseMusic(int index)
{
	if (Musics[index].channel != nullptr)
	{
		if (IsPlaying(index) == true)
		{
			if (IsPaused(index) == false)
			{
				result = FMOD_Channel_SetPaused(Musics[index].channel, true);
			}
			else
			{
				result = FMOD_Channel_SetPaused(Musics[index].channel, false);
			}
		}
		ErrorCheck(result);
	}
}

bool SoundManager::IsPlaying(int index)
{
	FMOD_BOOL isplay = false;
	FMOD_Channel_IsPlaying(Musics[index].channel, &isplay);
	return isplay;
}

bool SoundManager::IsPaused(int index)
{
	FMOD_BOOL ispaused;
	FMOD_Channel_GetPaused(Musics[index].channel, &ispaused);
	return ispaused;
}

void SoundManager::MoveMusicPlaybackPosition(int index, float pos)
{
	if (Musics[index].channel != nullptr)
	{
		unsigned int totalLength = 0;
		result = FMOD_Sound_GetLength(Musics[index].music, &totalLength, FMOD_TIMEUNIT_MS);

		unsigned int newPosition = static_cast<unsigned int>(pos * totalLength);
		result = FMOD_Channel_SetPosition(Musics[index].channel, newPosition, FMOD_TIMEUNIT_MS);
	}
}

float SoundManager::GetMusicPlaybackPosition(int index)
{
	if (Musics[index].channel != nullptr)
	{
		unsigned int currentTime = 0;
		unsigned int totalLength = 0;

		result = FMOD_Channel_GetPosition(Musics[index].channel, &currentTime, FMOD_TIMEUNIT_MS);
		result = FMOD_Sound_GetLength(Musics[index].music, &totalLength, FMOD_TIMEUNIT_MS);

		if (totalLength > 0)
		{
			return static_cast<float>(currentTime) / totalLength;
		}
		else
		{
			return 0.0f;
		}
	}
	return 0.0f;
}

PlaybackTime SoundManager::GetMusicPlaybackTime(int index, float pos)
{
	PlaybackTime temp;
	if (Musics[index].channel != nullptr)
	{
		unsigned int totalLength = 0;
		result = FMOD_Sound_GetLength(Musics[index].music, &totalLength, FMOD_TIMEUNIT_MS);

		temp.minutes = static_cast<int>(static_cast<float>(pos * totalLength) / 1000.0f / 60);
		temp.seconds = static_cast<int>(static_cast<float>(pos * totalLength) / 1000.0f) % 60;
	}
	return temp;
}

void SoundManager::SoundVolumeUp()
{
	SetSoundVolume(soundVolume + 0.05f);
}

void SoundManager::SoundVolumeDown()
{
	SetSoundVolume(soundVolume - 0.05f);
}

void SoundManager::MusicVolumeUp()
{
	SetMusicVolume(musicVolume + 0.05f);
}

void SoundManager::MusicVolumeDown()
{
	SetMusicVolume(musicVolume - 0.05f);
}

void SoundManager::SetSoundVolume(float amount)
{
	soundVolume = std::min(amount, 1.f);
	for (auto channel : Channels)
	{
		if (channel.second != nullptr)
		{
			result = FMOD_Channel_SetVolume(channel.second, soundVolume);
			ErrorCheck(result);
		}
	}
}

void SoundManager::SetMusicVolume(float amount)
{
	musicVolume = std::min(amount, 1.f);
	int index = 0;
	for (auto channel : Musics)
	{
		if (channel.channel != nullptr && IsPlaying(index) == true)
		{
			result = FMOD_Channel_SetVolume(channel.channel, musicVolume);
			ErrorCheck(result);
		}
		++index;
	}
}

void SoundManager::SetListenerPosition(glm::vec3 pos)
{
	listenerPosition = { pos.x, pos.y, pos.z };
	FMOD_VECTOR listenerVelocity = { 0, 0, 0 };

	FMOD_System_Set3DListenerAttributes(system, 0, &listenerPosition, &listenerVelocity, 0, 0);
}

void SoundManager::LoadSoundFilesFromFolder(const std::string& folderPath)
{
	WIN32_FIND_DATAA   findData;
	HANDLE hFind;

	std::string searchPath = folderPath + "/*";

	hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			std::string fileName = findData.cFileName;
			std::string filePath = folderPath + "/" + fileName;

			std::string extension = filePath.substr(filePath.find_last_of('.') + 1);
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });

			if (extension == "wav")
			{
#ifdef _DEBUG
				std::cout << "Load Sound Complete : " << fileName << std::endl;
#endif
				LoadSoundFile(filePath, fileName);
			}

		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}
}

void SoundManager::LoadMusicFilesFromFolder(const std::wstring& folderPath)
{
	WIN32_FIND_DATAW  findData;
	HANDLE hFind;

	std::wstring searchPath = folderPath + L"\\*";

	hFind = FindFirstFileW(searchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			std::wstring fileName = findData.cFileName;
			std::wstring filePath = folderPath + L"\\" + fileName;

			std::wstring extension = filePath.substr(filePath.find_last_of('.') + 1); 
			std::transform(extension.begin(), extension.end(), extension.begin(),
				[](wchar_t c) { return std::tolower(c, std::locale()); });


			if (extension == L"mp3" || extension == L"ogg")
			{
				std::cout << "Load Music Complete : " << ConvertWideStringToUTF8(fileName).c_str() << std::endl;
				LoadMusicFile(filePath, fileName);
			}

		} while (FindNextFileW(hFind, &findData));

		FindClose(hFind);
	}
}

#ifdef _DEBUG
void SoundManager::MusicPlayerForImGui()
{
	if (musicMaxIndex > 0)
	{
		ImGui::Begin("MusicPlayer");
		ImGui::Text("Now Playing:");
		ImGui::SameLine();
		ImGui::Text(GetMusicList().at(currentIndex).name.c_str());

		//Button
		if (ImGui::Button("Prev")) //키 입력 함수
		{
			StopMusic(currentIndex);
			--currentIndex;
			if (currentIndex < 0)
			{
				currentIndex = GetAmontOfMusics() - 1;
			}
			PlayMusic(currentIndex);
		}
		if (IsPaused(currentIndex) == true)
		{
			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				if (IsPlaying(currentIndex) != true)
				{
					PlayMusic(currentIndex);
				}
				else
				{
					PauseMusic(currentIndex);
				}
			}
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Pause"))
			{
				PauseMusic(currentIndex);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			StopMusic(currentIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("Next"))
		{
			StopMusic(currentIndex);
			++currentIndex;
			if (currentIndex > GetAmontOfMusics() - 1)
			{
				currentIndex = 0;
			}
			PlayMusic(currentIndex);
		}
		//Button
		
		//music playback
		float currentPlaybackPosition = GetMusicPlaybackPosition(currentIndex);
		PlaybackTime pTime = GetMusicPlaybackTime(currentIndex, currentPlaybackPosition);
		ImGui::PushItemWidth(256);
		if (ImGui::SliderFloat(" ", &currentPlaybackPosition, 0.0f, 1.0f, ""))
		{
			if (isAdjusting == false)
			{
				if (IsPaused(currentIndex) == false)
				{
					PauseMusic(currentIndex);
				}
				isAdjusting = true;
			}
			MoveMusicPlaybackPosition(currentIndex, currentPlaybackPosition);
		}
		else
		{
			if (isAdjusting == true)
			{
				PauseMusic(currentIndex);
				isAdjusting = false;
			}
		}
		ImGui::SameLine();
		ImGui::Text("%d:%d", pTime.minutes, pTime.seconds);
		ImGui::PopItemWidth();
		//music playback

		//volume
		float volume = GetMusicVolume() * 100.0f;
		ImGui::PushItemWidth(128);
		if (ImGui::SliderFloat("MusicVolume", &volume, 0.0f, 100.0f, "%.2f"))
		{
			SetMusicVolume(volume / 100.f);
		}
		ImGui::PopItemWidth();
		//volume

		//music list
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
		ImGui::BeginChild("Scolling");
		int index = 0;
		for (auto& music : GetMusicList())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (currentIndex == index) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));
			if (ImGui::Selectable(music.name.c_str(), index))
			{
				StopMusic(currentIndex);
				currentIndex = index;
				PlayMusic(currentIndex);
			}
			ImGui::PopStyleColor();
			index++;
		}
		ImGui::EndChild();
		//music list
		ImGui::End();
	}
}
#endif