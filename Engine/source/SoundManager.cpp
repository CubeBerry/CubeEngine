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
#include "imgui.h"

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

void SoundManager::Initialize(int maxChannel)
{
	result = FMOD::System_Create(&system);
	ErrorCheck(result);

	result = system->init(maxChannel, FMOD_INIT_NORMAL, 0);
	ErrorCheck(result);

	for (int i = 0; i <= maxChannel; i++)
	{
		Channel channel;
		channels.push_back(std::move(channel));
		channels.back().channel->setVolume(channels.back().soundVolume);
	}
}

void SoundManager::Update()
{
	result = system->update();
	ErrorCheck(result);
}

void SoundManager::Shutdown()
{
	ClearSounds();

	channels.clear();

	system->release();
	system->close();
}

void SoundManager::ClearSounds()
{
	for (auto sound : sounds)
	{
		result = sound.sound->release();
		ErrorCheck(result);
	}

	for (auto channel : channels)
	{
		channel.channel->stop();
	}

	sounds.clear();
	soundMaxIndex = 0;
}

void SoundManager::ErrorCheck(FMOD_RESULT result_)
{
	if (result_ != FMOD_OK && result_ != FMOD_ERR_INVALID_HANDLE)
	{
		exit(-1);
	}
}

void SoundManager::LoadFile(std::string filepath, std::string name, bool loop)
{
	Sound sound;
	sounds.push_back(std::move(sound));
	if (loop == true)
	{
		result = system->createSound(filepath.c_str(), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_3D, nullptr, &sounds[soundMaxIndex].sound);
		ErrorCheck(result);
	}
	else
	{
		result = system->createSound(filepath.c_str(), FMOD_DEFAULT | FMOD_2D | FMOD_3D, nullptr, &sounds[soundMaxIndex].sound);
		ErrorCheck(result);
	}
	sounds[soundMaxIndex].name = name;
	++soundMaxIndex;
}

void SoundManager::LoadFile(std::wstring filepath, std::wstring name, bool loop)
{
	Sound sound;
	sounds.push_back(std::move(sound));
	std::string path = ConvertWideStringToUTF8(filepath);
	if (loop == true)
	{
		result = system->createSound(path.c_str(), FMOD_LOOP_NORMAL | FMOD_2D | FMOD_3D, nullptr, &sounds[soundMaxIndex].sound);
		ErrorCheck(result);
	}
	else
	{
		result = system->createSound(path.c_str(), FMOD_DEFAULT | FMOD_2D | FMOD_3D, nullptr, &sounds[soundMaxIndex].sound);
		ErrorCheck(result);
	}
	sounds[soundMaxIndex].name = ConvertWideStringToUTF8(name);
	sounds[soundMaxIndex].nameL = name;
	++soundMaxIndex;
}

void SoundManager::Play(std::string name, int channelIndex, bool loop)
{
	int index = FindSoundIndexWithName(name);
	Play(index, channelIndex, loop);
}

void SoundManager::Play(int index, int channelIndex, bool loop)
{
	result = system->update();
	ErrorCheck(result);

	if (sounds[index].sound != nullptr)
	{
		result = system->playSound(sounds[index].sound, nullptr, false, &channels[channelIndex].channel);
		ErrorCheck(result);

		result = channels[channelIndex].channel->setVolume(channels[channelIndex].soundVolume);
		ErrorCheck(result);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		result = sounds[index].sound->setMode(mode);
		ErrorCheck(result);

	}
}

void SoundManager::Stop(int channelIndex)
{
	if (channels[channelIndex].channel != nullptr)
	{
		if (IsPlaying(channelIndex) == true)
		{
			result = channels[channelIndex].channel->stop();
			ErrorCheck(result);
		}
	}
}

void SoundManager::Pause(int channelIndex, FMOD_BOOL state)
{
	if (channels[channelIndex].channel != nullptr)
	{
		if (IsPlaying(channelIndex) == true)
		{
			result = channels[channelIndex].channel->setPaused(state);
			ErrorCheck(result);
		}
	}
}

void SoundManager::Pause(int channelIndex)
{
	if (channels[channelIndex].channel != nullptr)
	{
		if (IsPlaying(channelIndex) == true)
		{
			if (IsPaused(channelIndex) == false)
			{
				result = channels[channelIndex].channel->setPaused(true);
				ErrorCheck(result);
			}
			else
			{
				result = channels[channelIndex].channel->setPaused(false);
				ErrorCheck(result);
			}
		}
	}
}

bool SoundManager::IsPlaying(int channelIndex)
{
	if (channels[channelIndex].channel != nullptr)
	{
		bool isplay = false;
		result = channels[channelIndex].channel->isPlaying(&isplay);
		ErrorCheck(result);
		return isplay;
	}
	return false;
}

bool SoundManager::IsPaused(int channelIndex)
{
	if (channels[channelIndex].channel != nullptr)
	{
		bool ispaused = false;
		result = channels[channelIndex].channel->getPaused(&ispaused);
		ErrorCheck(result);
		return ispaused;
	}
	return false;
}

void SoundManager::VolumeUp(int channelIndex)
{
	SetVolume(channelIndex, channels[channelIndex].soundVolume + 0.05f);
}

void SoundManager::VolumeDown(int channelIndex)
{
	SetVolume(channelIndex, channels[channelIndex].soundVolume - 0.05f);
}

void SoundManager::SetVolume(int channelIndex, float volume)
{
	if (channels[channelIndex].channel != nullptr)
	{
		result = channels[channelIndex].channel->setVolume(volume);
		ErrorCheck(result);

		channels[channelIndex].soundVolume = volume;
	}
}

int SoundManager::FindSoundIndexWithName(std::string name)
{
	auto it = std::find_if(sounds.begin(), sounds.end(), [&](const Sound& sound) {
		return sound.name == name; });

	if (static_cast<Sound>(*it).sound != nullptr)
	{
		return static_cast<int>(std::distance(sounds.begin(), it));
	}
	else
	{
		return NULL;
	}
}

void SoundManager::PlaySoundInArea2D(std::string name, int channelIndex, glm::vec2 pos, float maxDis, bool loop)
{
	result = system->update();
	ErrorCheck(result);

	Play(name, channelIndex, loop);
	UpdateSoundVolumeInArea2D(channelIndex, pos, maxDis);
}

void SoundManager::UpdateSoundVolumeInArea2D(int channelIndex, glm::vec2 pos, float maxDis)
{
	if (channels[channelIndex].channel != nullptr)
	{
		float distance = static_cast<float>(std::sqrt(std::pow(pos.x - listenerPosition.x, 2) + std::pow(pos.y - listenerPosition.y, 2)));
		float volume = 1.0f - std::min(distance / maxDis, 1.0f);

		SetVolume(channelIndex, channels[channelIndex].soundVolume * volume);
	}
}

void SoundManager::PlaySoundIn3D(std::string name, int channelIndex, glm::vec3 pos, float minDis, float maxDis, bool loop)
{
	FMOD_VECTOR position = { pos.x, pos.y, pos.z };
	FMOD_VECTOR velocity = { 0.f, 0.f, 0.f };

	result = system->update();
	ErrorCheck(result);
	int index = FindSoundIndexWithName(name);
	if (sounds[index].sound != nullptr)
	{
		result = system->playSound(sounds[index].sound, NULL, false, &channels[channelIndex].channel);
		ErrorCheck(result);

		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		result = sounds[index].sound->setMode(mode);
		ErrorCheck(result);

		result = sounds[index].sound->set3DMinMaxDistance(minDis, maxDis);
		ErrorCheck(result);
	}
	if (channels[channelIndex].channel != nullptr)
	{
		result = channels[channelIndex].channel->setVolume(channels[channelIndex].soundVolume);
		ErrorCheck(result);

		result = channels[channelIndex].channel->set3DAttributes(&position, &velocity);
		ErrorCheck(result);
	}
}

void SoundManager::MoveSoundPlaybackPosition(int channelIndex, int currentSoundIndex, float pos)
{
	if (channels[channelIndex].channel != nullptr)
	{
		unsigned int totalLength = 0;
		result = sounds[currentSoundIndex].sound->getLength(&totalLength, FMOD_TIMEUNIT_MS);
		ErrorCheck(result);

		unsigned int newPosition = static_cast<unsigned int>(pos * totalLength);
		result = channels[channelIndex].channel->setPosition(newPosition, FMOD_TIMEUNIT_MS);
	}
}

float SoundManager::GetSoundPlaybackPosition(int channelIndex, int currentSoundIndex)
{
	if (channels[channelIndex].channel != nullptr)
	{
		unsigned int currentTime = 0;
		unsigned int totalLength = 0;

		if (IsPlaying(channelIndex) == true)
		{
			result = channels[channelIndex].channel->getPosition(&currentTime, FMOD_TIMEUNIT_MS);
			ErrorCheck(result);
		}

		result = sounds[currentSoundIndex].sound->getLength(&totalLength, FMOD_TIMEUNIT_MS);
		ErrorCheck(result);

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

PlaybackTime SoundManager::GetSoundPlaybackTime(int channelIndex, int currentSoundIndex, float pos)
{
	PlaybackTime temp;
	if (channels[channelIndex].channel != nullptr)
	{
		unsigned int totalLength = 0;
		result = sounds[currentSoundIndex].sound->getLength(&totalLength, FMOD_TIMEUNIT_MS);
		ErrorCheck(result);

		temp.minutes = static_cast<int>(static_cast<float>(pos * totalLength) / 1000.0f / 60);
		temp.seconds = static_cast<int>(static_cast<float>(pos * totalLength) / 1000.0f) % 60;
	}
	return temp;
}

void SoundManager::SetListenerPosition(glm::vec3 pos)
{
	listenerPosition = { pos.x, pos.y, pos.z };
	FMOD_VECTOR listenerVelocity = { 0, 0, 0 };

	result = system->set3DListenerAttributes(0, &listenerPosition, &listenerVelocity, 0, 0);
	ErrorCheck(result);
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
				LoadFile(filePath, fileName);
			}

		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}
}

void SoundManager::LoadSoundFilesFromFolder(const std::wstring& folderPath)
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
				LoadFile(filePath, fileName);
			}

		} while (FindNextFileW(hFind, &findData));

		FindClose(hFind);
	}
}

void SoundManager::MusicPlayerForImGui(int channelIndex)
{
	if (soundMaxIndex > 0)
	{
		ImGui::Begin("MusicPlayer");
		ImGui::Text("Now Playing:");
		ImGui::SameLine();
		ImGui::Text(GetSoundList().at(currentIndex).name.c_str());

		//Button
		if (ImGui::Button("Prev")) //Ű �Է� �Լ�
		{
			Stop(channelIndex);
			--currentIndex;
			if (currentIndex < 0)
			{
				currentIndex = GetAmontOfSounds() - 1;
			}
			Play(currentIndex, channelIndex);
		} 
		if (IsPlaying(channelIndex) != true)
		{
			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				Play(currentIndex, channelIndex);
			}
		}
		else if (IsPaused(channelIndex) == true)
		{
			ImGui::SameLine();
			if (ImGui::Button("Play"))
			{
				if (IsPlaying(channelIndex) != true)
				{
					Play(currentIndex, channelIndex);
				}
				else
				{
					Pause(channelIndex);
				}
			}
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Pause"))
			{
				Pause(channelIndex);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			Stop(channelIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("Next"))
		{
			Stop(channelIndex);
			++currentIndex;
			if (currentIndex > GetAmontOfSounds() - 1)
			{
				currentIndex = 0;
			}
			Play(currentIndex, channelIndex);
		}
		//Button

		//music playback
		float currentPlaybackPosition = GetSoundPlaybackPosition(channelIndex, currentIndex);
		PlaybackTime pTime = GetSoundPlaybackTime(channelIndex, currentIndex, currentPlaybackPosition);
		ImGui::PushItemWidth(256);
		if (ImGui::SliderFloat(" ", &currentPlaybackPosition, 0.0f, 1.0f, ""))
		{
			if (isAdjusting == false)
			{
				if (IsPaused(channelIndex) == false)
				{
					Pause(channelIndex);
				}
				isAdjusting = true;
			}
			MoveSoundPlaybackPosition(channelIndex, currentIndex, currentPlaybackPosition);
		}
		else
		{
			if (isAdjusting == true)
			{
				Pause(channelIndex);
				isAdjusting = false;
			}
		}
		ImGui::SameLine();
		ImGui::Text("%d:%d", pTime.minutes, pTime.seconds);
		ImGui::PopItemWidth();
		//music playback

		//volume
		float volume = channels.at(channelIndex).soundVolume * 100.0f;
		ImGui::PushItemWidth(128);
		if (ImGui::SliderFloat("MusicVolume", &volume, 0.0f, 100.0f, "%.2f"))
		{
			SetVolume(channelIndex, volume / 100.f);
		}
		ImGui::PopItemWidth();
		//volume

		//music list
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, 3 * ImGui::GetTextLineHeightWithSpacing()));
		ImGui::BeginChild("Scolling");
		int index = 0;
		for (auto& music : GetSoundList())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (currentIndex == index) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));
			if (ImGui::Selectable(music.name.c_str(), index))
			{
				Stop(channelIndex);
				currentIndex = index;
				Play(currentIndex, channelIndex);
			}
			ImGui::PopStyleColor();
			index++;
		}
		ImGui::EndChild();
		//music list
		ImGui::End();
	}
}