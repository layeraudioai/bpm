#include "Synth.h"
#include "notes.h"
#include "fixed.h"

// Stub implementation of SynthGBA.
// This allows the project to link. Real implementation would manipulate GBA sound registers or mix audio.

void Synth_Init(void) {
    // TODO: Initialize GBA sound hardware (REG_SOUNDCNT, etc.)
}

void Synth_Update(void) {
    // TODO: Update sequencer/mixer/envelopes
}

void Synth_SetParams(u32 ch, const SynthChannelParams* params) {
    (void)ch;
    (void)params;
}

void Synth_SetNote(u32 ch, u32 note, u32 finetune) {
    (void)ch;
    (void)note;
    (void)finetune;
}

void Synth_NoteOn(u32 ch) {
    (void)ch;
}

void Synth_NoteOff(u32 ch) {
    (void)ch;
}

void Synth_SetVibrato(u32 ch, u8 depth, u8 speed) {
    (void)ch;
    (void)depth;
    (void)speed;
}

void Synth_PlaySfx(const SynthChannelParams* params, u32 note, u32 finetune) {
    (void)params;
    (void)note;
    (void)finetune;
}
