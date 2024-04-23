#include "audiomanager.h"
#include "Random.h"

#include <Windows.h>
#ifdef PlaySound
#undef PlaySound
#endif

#include <random>

#include <iostream>
#include <fmod/fmod_errors.h>

void CheckError(FMOD_RESULT result, const char* file, int line)
{
	if (result != FMOD_OK)
	{
		printf("FMOD Error [%d, %s]: '%s' at %d\n", static_cast<int>(result), FMOD_ErrorString(result), file, line);
	}
}

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
}


void AudioManager::Initialize()
{
	if (m_IsInitialized)
		return;

	FMOD_RESULT result = FMOD::System_Create(&m_System);
	if (result != FMOD_OK)
	{
		printf("Failed to create the FMOD System!\n");
		return;
	}

	result = m_System->init(32, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		printf("Failed to initialize the audio system!\n");
		result = m_System->close();
		if (result != FMOD_OK)
		{
			printf("Failed to close system!\n");
		}
		return;
	}

	m_ExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	m_ExInfo.numchannels = 2;
	m_ExInfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
	m_ExInfo.defaultfrequency = 44100;
	m_ExInfo.length = m_ExInfo.defaultfrequency * m_ExInfo.numchannels * sizeof(float) * 5;

	// TODO: Create DSPs
	// STore in m_DSPs

	CreateDSP(REVERB);
	CreateDSP(DISTORTION);
	CreateDSP(PITCH_SHIFT);

	m_IsInitialized = true;
}

void AudioManager::Destroy()
{
	if (!m_IsInitialized)
		return;

	FMOD_RESULT result = FMOD_OK;

	result = m_Channel->stop();
	FMODCheckError(result);

	result = m_RecordingSound->release();
	FMODCheckError(result);

	result = m_System->release();
	FMODCheckError(result);

	m_IsInitialized = false;
}

void AudioManager::Update()
{
	if (!m_IsInitialized)
		return;

	FMOD_RESULT result = m_System->update();
	FMODCheckError(result);

	if (m_IsRecording)
	{
		ProcessRecording();
	}
}

void AudioManager::CreateDSP(DSPType type)
{
	FMOD::DSP* dsp = nullptr;
	FMOD_RESULT result = FMOD_OK;

	// TODO: Create DSP based on type
	switch (type)
	{
	case REVERB:
		result = m_System->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &dsp);
		dsp->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, 1500.0f); 
		dsp->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, 200.0f);  
		break;


	case DISTORTION:
		result = m_System->createDSPByType(FMOD_DSP_TYPE_DISTORTION, &dsp);
		dsp->setParameterFloat(FMOD_DSP_DISTORTION_LEVEL, 1.0f); 
		break;

	case PITCH_SHIFT:
		result = m_System->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &dsp);
		dsp->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, 20.0f);
		break;

	}

	FMODCheckError(result);

	if (dsp)
	{
		m_DSPs.push_back(dsp);
	}
}

void AudioManager::SetActiveDSP(int index)
{
	m_ActiveDSP = index;

	FMOD_RESULT result = m_Channel->addDSP(0, m_DSPs[m_ActiveDSP]);
	FMODCheckError(result);
}

void AudioManager::BeginRecording()
{
	if (!m_IsInitialized)
		return;

	if (m_IsRecording)
		return;

	FMOD_RESULT result = m_System->createSound(0, FMOD_DEFAULT | FMOD_OPENUSER, &m_ExInfo, &m_RecordingSound);
	FMODCheckError(result);

	// https://www.fmod.com/docs/2.00/api/core-api-system.html#system_recordstart
	result = m_System->recordStart(0, m_RecordingSound, true);
	FMODCheckError(result);

	result = m_RecordingSound->getLength(&m_SoundLength, FMOD_TIMEUNIT_PCM);
	FMODCheckError(result);

	m_IsRecording = true;

	m_RecordingLength = 0;
	m_LastRecordPosition = 0;
	m_RecordPosition = 0;
}

void AudioManager::EndRecording()
{
	if (!m_IsInitialized)
		return;

	if (!m_IsRecording)
		return;

	// https://www.fmod.com/docs/2.00/api/core-api-system.html#system_recordstop
	FMOD_RESULT result = m_System->recordStop(0);
	FMODCheckError(result);
	

	m_RecordedSounds.push_back(m_RecordingSound);

	m_IsRecording = false;
}

void AudioManager::PlayRecordedSound()
{
	if (m_RecordingSound)
	{
		FMOD_RESULT result = m_System->playSound(m_RecordingSound, 0, false, &m_Channel);
		FMODCheckError(result);

		result = m_Channel->addDSP(0, m_DSPs[m_ActiveDSP]);
		FMODCheckError(result);
	}
}

FMOD::Sound* AudioManager::GetRandomSound()
{
	int random = GetRandomIntNumber(0, m_RecordedSounds.size() - 1);

	return m_RecordedSounds[random];
}

void AudioManager::PlayARandomSound()
{

	FMOD::Sound* randomSound = GetRandomSound();

	if (randomSound)
	{
		FMOD_RESULT result = m_System->playSound(randomSound, 0, false, &m_Channel);
		FMODCheckError(result);


		result = m_Channel->addDSP(0, m_DSPs[m_ActiveDSP]);
		FMODCheckError(result);
	}
}

bool AudioManager::IsSoundPlaying() const
{
	if (m_Channel == nullptr)
		return false;

	bool isPlaying = false;

	FMOD_RESULT result = m_Channel->isPlaying(&isPlaying);
	FMODCheckError(result);

	return isPlaying;
}

FMOD_RESULT AudioManager::RecordCallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
{
	float* buffer = (float*)data;
	size_t sampleCount = datalen / 2;
	m_RingBuffer.Write(buffer, sampleCount);

	
	return FMOD_OK;
}

void AudioManager::ProcessRecording()
{
	// Retrieves the current recording position of the record buffer in PCM samples.
	// https://www.fmod.com/docs/2.00/api/core-api-system.html#system_getrecordposition
	FMOD_RESULT result = m_System->getRecordPosition(0, &m_RecordPosition);
	FMODCheckError(result);

	if (m_RecordPosition != m_LastRecordPosition)
	{
		void* ptr1 = nullptr;
		void* ptr2 = nullptr;
		unsigned int len1 = 0;
		unsigned int len2 = 0;
		int blockLength = 0;

		blockLength = m_RecordPosition - m_LastRecordPosition;
		if (blockLength < 0)
		{
			blockLength += m_SoundLength;
		}

		// Lock a block of memory of our sound to read data
		// https://www.fmod.com/docs/2.00/api/core-api-sound.html#sound_lock
		result = m_RecordingSound->lock(
			m_LastRecordPosition * m_ExInfo.numchannels * 2,
			blockLength * m_ExInfo.numchannels * 2,
			&ptr1, &ptr2, &len1, &len2);
		FMODCheckError(result);


		// If the data is looping then ptr1 and len1 will be the last section of the information, and 
		// ptr2 and len2 will start at index 0 again. If the first ptr1 and len1 does not exceed the
		// length then ptr2 will be nul and len2 will be 0
		if (ptr1 && len1 != 0)
		{
			m_RecordingLength += len1;
			// Write the data to our ringbuffer
			// TODO: m_RingBuffer.Write(...)

			m_RingBuffer.Write((float*)ptr1, len1 / sizeof(float));
		}
		if (ptr2 && len2 != 0)
		{
			m_RecordingLength += len2;
			// Write the data to our ringbuffer
			// TODO: m_RingBuffer.Write(...)

			m_RingBuffer.Write((float*)ptr2, len2 / sizeof(float));
		}

		m_TextFile.open("AudioFile.txt");
		if (!m_TextFile.is_open())
		{
			printf("Failed to open text file for writing.\n");
			return;
		}

		float buffer[1000];
		m_RingBuffer.Read(buffer, 1000);

		for (int i = 0; i < 1000; ++i)
		{
			m_TextFile << buffer[i] << " ";
		}
		m_TextFile << "\n";

		// Unlock the block of memory
		// https://www.fmod.com/docs/2.00/api/core-api-sound.html#sound_unlock
		result = m_RecordingSound->unlock(ptr1, ptr2, len1, len2);
		FMODCheckError(result);
	}

	printf("Record buffer pos = %6d : Record time = %02d:%02d.%02d\r",
		m_RecordPosition, m_RecordingLength / m_ExInfo.defaultfrequency / m_ExInfo.numchannels / 2 / 60,	// Minutes
		(m_RecordingLength / m_ExInfo.defaultfrequency / m_ExInfo.numchannels / 2) % 60,					// Seconds
		(m_RecordingLength / (m_ExInfo.defaultfrequency / 100) / m_ExInfo.numchannels / 2) % 100);			// Milliseconds

	m_LastRecordPosition = m_RecordPosition;

}

