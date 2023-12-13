#pragma once

#include <fmod/fmod.hpp>
#include <vector>
#include "ringbuffer.h"
#include <string>

void CheckError(FMOD_RESULT result, const char* file, int line);
#define FMODCheckError(result) CheckError(result, __FILE__, __LINE__)

class AudioManager
{
public:
	AudioManager();
	~AudioManager();

	void LoadSounds(std::vector<std::string>& soundPaths);

	void Initialize();
	void Destroy();

	void Update();

	// TODO: Create a function for creating a DSP of a specific Type (m_DSPs)
	//		 or create them in Initialize() flagged with a different TODO tag.

	// TODO: Create a function to set a DSP to be active. (m_ActiveDSP)

	void BeginRecording();
	void EndRecording();
	bool IsRecording() const { return m_IsRecording; }

	// TODO: Create a function to PlayRecordedSound
	// TODO: Create a function to PlayARandomSound

	bool IsSoundPlaying() const;

private:
	FMOD_RESULT RecordCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);
	void ProcessRecording();
	void GetRandomSound(FMOD::Sound** sound);

	bool m_IsInitialized = false;
	bool m_IsRecording = false;

	FMOD_CREATESOUNDEXINFO m_ExInfo = {};

	FMOD::System* m_System = nullptr;
	FMOD::Channel* m_Channel = nullptr;
	FMOD::Sound* m_RecordingSound = nullptr;

	std::vector<FMOD::DSP*> m_DSPs;
	std::vector<FMOD::Sound*> m_Sounds;

	RingBuffer<float, 10000> m_RingBuffer;

	unsigned int m_SoundLength = 0;
	unsigned int m_ActiveDSP = 0;
	unsigned int m_RecordPosition = 0;
	unsigned int m_LastRecordPosition = 0;
	unsigned int m_RecordingLength = 0;
};
