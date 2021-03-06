//
// Created by Diego S. Seabra on 03/03/22.
//

#include "AudioEngine.h"
#include "../profiling/Profiler.h"
#include "../profiling/Logger.h"

CAudioEngineImpl* mImplementation = nullptr;

CAudioEngineImpl::CAudioEngineImpl()
{
    mStudioSystem = nullptr;
    AudioEngine::ErrorCheck(FMOD::Studio::System::create(&mStudioSystem));
    AudioEngine::ErrorCheck(mStudioSystem->initialize(32,
                                                      FMOD_STUDIO_INIT_LIVEUPDATE,
                                                      FMOD_INIT_PROFILE_ENABLE,
                                                      nullptr));

    mSystem = nullptr;
    AudioEngine::ErrorCheck(mStudioSystem->getCoreSystem(&mSystem));
}

CAudioEngineImpl::~CAudioEngineImpl()
{
    AudioEngine::ErrorCheck(mStudioSystem->unloadAll());
    AudioEngine::ErrorCheck(mStudioSystem->release());
}

void CAudioEngineImpl::Update()
{
    // check the current sound to see if any sounds are stopped
    std::vector<ChannelMap::iterator> vStoppedChannels;
    for(auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
    {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying)
        {
            vStoppedChannels.push_back(it);
        }
    }

    // erase the stopped sound from our checking
    for (auto& it : vStoppedChannels)
    {
        mChannels.erase(it);
    }

    // update the fmod system object
    AudioEngine::ErrorCheck(mStudioSystem->update());
}

void AudioEngine::Init()
{
    Logger::Info("Initializing audio engine");
    PROFILE_FUNCTION();
    mImplementation = new CAudioEngineImpl;
}

void AudioEngine::Update()
{
    PROFILE_FUNCTION();
    mImplementation->Update();
}

void AudioEngine::Shutdown()
{
    Logger::Info("Shutting down audio engine");
    PROFILE_FUNCTION();
    delete mImplementation;
}

int AudioEngine::ErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        Logger::Error("FMOD Error", (char *) result);
        throw std::runtime_error("FMOD error");
    }

    return 0;
}

void AudioEngine::LoadSound(const std::string &sSoundName, bool b3d, bool bLooping, bool bStream)
{
    // check if the sound is loaded
    auto tFoundIt = mImplementation->mSounds.find(sSoundName);
    if (tFoundIt != mImplementation->mSounds.end())
        return;

    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    // load the sound
    FMOD::Sound* sound = nullptr;
    AudioEngine::ErrorCheck(mImplementation->mSystem->createSound(sSoundName.c_str(), eMode, nullptr, &sound));
    if (sound)
    {
        mImplementation->mSounds[sSoundName] = sound;
    }
}

void AudioEngine::UnLoadSound(const std::string &sSoundName)
{
    // check if the sound is loaded
    auto foundIt = mImplementation->mSounds.find(sSoundName);
    if (foundIt == mImplementation->mSounds.end())
        return;

    // unload the sound
    AudioEngine::ErrorCheck(foundIt->second->release());
    mImplementation->mSounds.erase(foundIt);
}

int AudioEngine::PlaySoundFile(const std::string &sSoundName, const Vector3 &vPosition, float fVolumedB)
{
    // check if the sound is loaded
    int channelId = mImplementation->mNextChannelId++;
    auto foundIt = mImplementation->mSounds.find(sSoundName);

    // the sound is not loaded, so we load it
    if (foundIt == mImplementation->mSounds.end())
    {
        LoadSound(sSoundName, true);
        foundIt = mImplementation->mSounds.find(sSoundName);
        if (foundIt == mImplementation->mSounds.end())
        {
            return channelId;
        }
    }

    // play the sound in a new created channel
    FMOD::Channel* channel = nullptr;
    AudioEngine::ErrorCheck(mImplementation->mSystem->playSound(foundIt->second, nullptr, true, &channel));
    if (channel)
    {
        FMOD_MODE currMode;
        foundIt->second->getMode(&currMode);

        // if the sound is in 3d space, we set its 3d attributes in fmod
        if (currMode & FMOD_3D)
        {
            FMOD_VECTOR fmodVector = VectorToFmod(vPosition);
            AudioEngine::ErrorCheck(channel->set3DAttributes(&fmodVector, nullptr));
        }
        AudioEngine::ErrorCheck(channel->setVolume(dbToVolume(fVolumedB)));
        AudioEngine::ErrorCheck(channel->setPaused(false));

        mImplementation->mChannels[channelId] = channel;
    }

    return channelId;
}

void AudioEngine::SetChannel3dPosition(int nChannelId, const Vector3 &vPosition)
{
    auto foundIt = mImplementation->mChannels.find(nChannelId);
    if (foundIt == mImplementation->mChannels.end())
        return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    AudioEngine::ErrorCheck(foundIt->second->set3DAttributes(&position, nullptr));
}

void AudioEngine::SetChannelVolume(int nChannelId, float fVolumedB)
{
    auto foundIt = mImplementation->mChannels.find(nChannelId);
    if (foundIt == mImplementation->mChannels.end())
        return;

    AudioEngine::ErrorCheck(foundIt->second->setVolume(fVolumedB));
}

// banks are what stores all the sounds and informations for each FMOD event
void AudioEngine::LoadBank(const std::string &sBankName, FMOD_STUDIO_LOAD_BANK_FLAGS pflags)
{
    // check if the bank is loaded
    auto foundIt = mImplementation->mBanks.find(sBankName);
    if (foundIt != mImplementation->mBanks.end())
        return;

    // load the bank
    FMOD::Studio::Bank* bank;
    AudioEngine::ErrorCheck(mImplementation->mStudioSystem->loadBankFile(
            sBankName.c_str(), pflags, &bank));
    if (bank)
    {
        mImplementation->mBanks[sBankName] = bank;
    }
}

// FMOD events have a description and an instance
//   the description is the information and the instance is what actually plays the sound
// FIXME: Only works with the GUID for now
void AudioEngine::LoadEvent(const std::string &sEventName)
{
    // check if the event is loaded
    auto foundIt = mImplementation->mEvents.find(sEventName);
    if (foundIt != mImplementation->mEvents.end())
        return;

    FMOD::Studio::EventDescription* eventDescription = nullptr;
    AudioEngine::ErrorCheck(mImplementation->mStudioSystem->getEvent(sEventName.c_str(), &eventDescription));

    if (eventDescription)
    {
        FMOD::Studio::EventInstance* eventInstance = nullptr;
        AudioEngine::ErrorCheck(eventDescription->createInstance(&eventInstance));
        if (eventInstance)
        {
            mImplementation->mEvents[sEventName] = eventInstance;
        }
    }
}

// FIXME: Only works with the GUID for now
void AudioEngine::PlayEvent(const std::string &sEventName)
{
    // check if the event is loaded
    auto foundIt = mImplementation->mEvents.find(sEventName);

    // the sound is not loaded, so we load it
    if (foundIt == mImplementation->mEvents.end())
    {
        LoadEvent(sEventName);
        foundIt = mImplementation->mEvents.find(sEventName);
        if (foundIt == mImplementation->mEvents.end())
            return;
    }

    // play the event
    foundIt->second->start();
}

void AudioEngine::StopEvent(const std::string &sEventName, bool bImmediate)
{
    // check if the event is loaded
    auto foundIt = mImplementation->mEvents.find(sEventName);
    if (foundIt == mImplementation->mEvents.end())
        return;

    // stop the event
    FMOD_STUDIO_STOP_MODE mode;
    mode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    AudioEngine::ErrorCheck(foundIt->second->stop(mode));
}

bool AudioEngine::IsEventPlaying(const std::string &sEventName)
{
    // check is the event is loaded
    auto foundIt = mImplementation->mEvents.find(sEventName);
    if (foundIt == mImplementation->mEvents.end())
        return false;

    FMOD_STUDIO_PLAYBACK_STATE* state = nullptr;
    if (foundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING)
    {
        return true;
    }
    return false;
}

void AudioEngine::GetEventParameter(const std::string &sEventName, const std::string &sEventParameter, float *fParameter)
{
    // check if the event is loaded
    auto foundIt = mImplementation->mEvents.find(sEventName);
    if (foundIt == mImplementation->mEvents.end())
        return;

    AudioEngine::ErrorCheck(foundIt->second->getParameterByName(sEventName.c_str(), fParameter, nullptr));
}

void AudioEngine::SetEventParameter(const std::string &sEventName, const std::string &sParameterName, float fValue)
{
    auto foundIt = mImplementation->mEvents.find(sEventName);
    if (foundIt == mImplementation->mEvents.end())
        return;

    AudioEngine::ErrorCheck(foundIt->second->setParameterByName(sEventName.c_str(), fValue));
}

float AudioEngine::dbToVolume(float fdB)
{
    return powf(10.0f, 0.05f * fdB);
}

float AudioEngine::VolumeTodB(float fVolume)
{
    return 20.0f * log10f(fVolume);
}

FMOD_VECTOR AudioEngine::VectorToFmod(const Vector3 &vPosition)
{
    return FMOD_VECTOR{
            vPosition.x,
            vPosition.y,
            vPosition.z
    };
}
