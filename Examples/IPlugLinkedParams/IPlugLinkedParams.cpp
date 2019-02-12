#include "IPlugLinkedParams.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugLinkedParams::IPlugLinkedParams(IPlugInstanceInfo instanceInfo)
: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  GetParam(kRangeLow)->InitDouble("Low", 0., 0., 100.0, 0.01, "%");
  GetParam(kRangeHigh)->InitDouble("High", 100., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(kUIResizerScale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont(ROBOTTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b, "Should not be possible to slide lower slider", IText(20)));
    pGraphics->AttachControl(new ITextControl(b.GetVShifted(20), "to higher value than upper slider and vice-versa.", IText(20)));
    pGraphics->AttachControl(new IVSliderControl(b.GetCentredInside(100, 20).GetVShifted(-100), kRangeLow, DEFAULT_SPEC, kHorizontal));
    pGraphics->AttachControl(new IVSliderControl(b.GetCentredInside(100, 20).GetVShifted(-150), kRangeHigh, DEFAULT_SPEC, kHorizontal));
  };
#endif
}

#if IPLUG_DSP
void IPlugLinkedParams::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kRangeLow)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}

void IPlugLinkedParams::OnParamChange(int paramIdx)
{
  switch (paramIdx)
  {
    case kRangeLow:
    {
      const double low = GetParam(kRangeLow)->Value();
      const double high = GetParam(kRangeHigh)->Value();
      // prevent the low param from getting bigger than the high param by clamping it based on the current value of high param
      if (low >= high - 0.01)
      {
        const double value = high - 0.01;
        GetParam(kRangeLow)->Set(value);
        InformHostOfParamChange(kRangeLow, GetParam(kRangeLow)->GetNormalized());
        SendParameterValueFromAPI(kRangeLow, value, false);
      }
    }
    break;

    case kRangeHigh:
    {
      const double low = GetParam(kRangeLow)->Value();
      const double high = GetParam(kRangeHigh)->Value();
      // prevent the high param from getting smaller than the low param by clamping it based on the current value of low param
      if (high <= low + 0.01)
      {
        const double value = low + 0.01;
        GetParam(kRangeHigh)->Set(value);
        InformHostOfParamChange(kRangeLow, GetParam(kRangeHigh)->GetNormalized());
        SendParameterValueFromAPI(kRangeHigh, value, false);
      }
    }
    break;
  }
}

#endif
