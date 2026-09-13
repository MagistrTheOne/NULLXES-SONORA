#include "soni/SoniVoice.h"

#include <thread>

#if JUCE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sapi.h>
#endif

namespace sonora::soni
{

Voice& Voice::get()
{
    static Voice voice;
    return voice;
}

void Voice::silence()
{
    ++generation_;
    speaking_.store(false);
#if JUCE_WINDOWS
    // next Speak with purge happens on a new worker
#endif
}

void Voice::speak(const juce::String& text)
{
    const auto line = text.trim();
    if (line.isEmpty())
        return;
    const int gen = ++generation_;
    speaking_.store(true);

#if JUCE_WINDOWS
    std::thread([this, line, gen] {
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
        {
            speaking_.store(false);
            return;
        }

        ISpVoice* voice = nullptr;
        if (FAILED(CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**) &voice)) || voice == nullptr)
        {
            CoUninitialize();
            if (generation_.load() == gen)
                speaking_.store(false);
            return;
        }

        ISpObjectTokenCategory* category = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_SpObjectTokenCategory, nullptr, CLSCTX_ALL, IID_ISpObjectTokenCategory, (void**) &category))
            && category != nullptr)
        {
            if (SUCCEEDED(category->SetId(SPCAT_VOICES, FALSE)))
            {
                IEnumSpObjectTokens* tokens = nullptr;
                if (SUCCEEDED(category->EnumTokens(L"Language=419", nullptr, &tokens)) && tokens != nullptr)
                {
                    ISpObjectToken* token = nullptr;
                    unsigned long fetched = 0;
                    if (SUCCEEDED(tokens->Next(1, &token, &fetched)) && fetched > 0 && token != nullptr)
                    {
                        voice->SetVoice(token);
                        token->Release();
                    }
                    tokens->Release();
                }
            }
            category->Release();
        }

        voice->SetRate(-1);
        voice->SetVolume(100);
        const auto wide = line.toWideCharPointer();
        if (generation_.load() == gen)
            voice->Speak(wide, SPF_DEFAULT | SPF_PURGEBEFORESPEAK, nullptr);
        voice->Release();
        CoUninitialize();
        if (generation_.load() == gen)
            speaking_.store(false);
    }).detach();
#else
    juce::ignoreUnused(gen);
    speaking_.store(false);
#endif
}

} // namespace sonora::soni
