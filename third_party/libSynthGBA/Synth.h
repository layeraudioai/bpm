#pragma once

#include <gba_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Channel parameters
typedef struct {
	u8 wave;
	u8 attack;
	u8 decay;
	u8 sustain;
	u8 release;
	u8 masterVolume;
	u8 bend;
	u8 bendSpeed;
	u8 vibratoDepth;
	u8 vibratoSpeed;
	u8 pwm;
	u8 pwmSpeed;
} SynthChannelParams;

void Synth_Init(void);
void Synth_Update(void);
void Synth_SetParams(u32 ch, const SynthChannelParams* params);
void Synth_SetNote(u32 ch, u32 note, u32 finetune);
void Synth_NoteOn(u32 ch);
void Synth_NoteOff(u32 ch);
void Synth_SetVibrato(u32 ch, u8 depth, u8 speed);
void Synth_PlaySfx(const SynthChannelParams* params, u32 note, u32 finetune);

#ifdef __cplusplus
}
#endif