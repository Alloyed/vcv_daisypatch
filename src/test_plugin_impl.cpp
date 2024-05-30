#include "test_plugin_impl.h"

#include "dsp/util.h"
#include <daisysp.h>

#include "dsp/psx.h"
#include "dsp/resampler.h"
#include "dsp/snes.h"

using namespace NotDaisy;
using namespace daisysp;

#define SAMPLE_RATE 32000

constexpr size_t snesBufferSize = 7680UL;
int16_t snesBuffer[7680];
SNES::Model snes(SNES::kOriginalSampleRate, snesBuffer, snesBufferSize);
Resampler snesSampler(SNES::kOriginalSampleRate, SAMPLE_RATE);

constexpr size_t psxBufferSize = 65536UL; // PSX::Model::GetBufferDesiredSizeFloats(PSX::kOriginalSampleRate);
float psxBuffer[65536UL];
PSX::Model psx(PSX::kOriginalSampleRate, psxBuffer, psxBufferSize);
Resampler psxSampler(PSX::kOriginalSampleRate, SAMPLE_RATE);

// 1.0f == psx, 0.0f == SNES
float snesToPsxFade = 0.0f;

// 10ms fade
static constexpr float fadeRate = (10.0f / 1000.0f) / SAMPLE_RATE;

void TestPluginImpl::Init(size_t samplerate)
{
    // This is were you would initialize anything you need to initialize at startup.
    // Basically, any code that you have in your main() function, before the while(1) loop.
    patch_.Init(samplerate);

    for (size_t i = 0; i < kNumOscs; ++i)
    {
        osc_[i].Init(samplerate);
        osc_[i].SetWaveform(Oscillator::WAVE_SIN + i);
    }
}

void TestPluginImpl::AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // Simple example of a quad sine oscillator using the 4 cv inputs as v/oct inputs.
    patch_.ProcessAllControls();

    snes.cfg.echoBufferSize = patch_.GetKnobValue(static_cast<DaisyPatch::Ctrl>(1));
    // snes.mod.echoBufferSize = jackValue(CV_5);
    snes.cfg.echoDelayMod = 1.0f; // TODO
    snes.cfg.echoFeedback = patch_.GetKnobValue(static_cast<DaisyPatch::Ctrl>(2));
    // snes.mod.echoFeedback = jackValue(CV_6);
    snes.cfg.filter = patch_.GetKnobValue(static_cast<DaisyPatch::Ctrl>(3));
    // snes.mod.filter = jackValue(CV_7);

    // PSX has no parameters yet D:

    // float wetDry = clampf(knobValue(CV_4) + jackValue(CV_8), 0.0f, 1.0f);
    float wetDry = 0.5f;
    // hw.WriteCvOut(2, 2.5 * wetDry);

    // if (button.RisingEdge() || hw.gate_in_1.Trig())
    //{
    //     snes.ClearBuffer();
    //     psx.ClearBuffer();
    // }

    for (size_t i = 0; i < size; i++)
    {
        float snesLeft, snesRight, psxLeft, psxRight;
        psxSampler.Process(in[0][i], in[1][i], psxLeft, psxRight,
                           [](float inLeft, float inRight, float& outLeft, float& outRight) {
                               psx.Process(inLeft, inRight, outLeft, outRight);
                           });

        snesSampler.Process(in[0][i], in[1][i], snesLeft, snesRight,
                            [](float inLeft, float inRight, float& outLeft, float& outRight) {
                                snes.Process(inLeft, inRight, outLeft, outRight);
                            });

        float left = fadeCpowf(snesLeft, psxLeft, snesToPsxFade);
        float right = fadeCpowf(snesRight, psxRight, snesToPsxFade);

        out[0][i] = lerpf(in[0][i], left, wetDry);
        out[1][i] = lerpf(in[1][i], right, wetDry);
    }
}

void TestPluginImpl::OnSampleRateChange(float sr)
{
    for (size_t i = 0; i < kNumOscs; ++i)
    {
        osc_[i].Init(sr);
        osc_[i].SetWaveform(Oscillator::WAVE_SIN + i);
    }
}
