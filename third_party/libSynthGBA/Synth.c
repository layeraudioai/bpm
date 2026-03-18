#include "Synth.h"
#include "notes.h"
#include "fixed.h"
#include "gba_regs.h"
#include "gba_interrupt.h"

// Function attributes for GBA Interrupt Handlers
#define IWRAM_CODE __attribute__((section(".iwram"), target("arm")))

// Configuration
#define SAMPLE_RATE 18157
#define SAMPLES_PER_FRAME 304
#define BUFFER_SIZE (SAMPLES_PER_FRAME * 2)
#define CHANNEL_COUNT 4
#define AUDIO_FIFO_ADDR 0x040000A0

// Internal State
typedef struct {
    u8 active;
    u32 note;
    u32 frequency; // Fixed point (FX_BITS)
    u32 phase;     // Fixed point accumulation
    u32 increment; // Fixed point phase step per sample
    
    // Envelope
    u8 env_stage; // 0:off, 1:att, 2:dec, 3:sus, 4:rel
    s32 env_vol;  // Current envelope volume (fixed point 8.8)
    
    SynthChannelParams params;
} Channel;

static Channel channels[CHANNEL_COUNT];
static s8 audio_buffer[2][BUFFER_SIZE] __attribute__((aligned(4)));
static volatile u32 active_buffer = 0;
static u32 sample_count_lfo = 0;

static inline void ma_memzero(void* dest, u32 size) {
    u8* d = (u8*)dest;
    while (size--) *d++ = 0;
}

// LCG Random for noise
static u32 rand_state = 12345;
static inline u8 fast_rand() {
    rand_state = rand_state * 1103515245 + 12345;
    return (rand_state >> 16) & 0xFF;
}

static void IWRAM_CODE MixAudio(s8* target, u32 count) {
    ma_memzero(target, count);

    for (int i = 0; i < CHANNEL_COUNT; i++) {
        Channel* ch = &channels[i];
        if (!ch->active) continue;

        for (u32 s = 0; s < count; s++) {
            // Envelope Logic (simplified per sample for smoothness, optimized)
            switch(ch->env_stage) {
                case 1: // Attack
                    ch->env_vol += (ch->params.attack > 0) ? (256 / ch->params.attack) : 256;
                    if (ch->env_vol >= 255) { ch->env_vol = 255; ch->env_stage = 2; }
                    break;
                case 2: // Decay
                    ch->env_vol -= (ch->params.decay > 0) ? (1 + (255 - ch->params.sustain * 2) / ch->params.decay) : 10;
                    if (ch->env_vol <= (ch->params.sustain * 2)) { ch->env_vol = (ch->params.sustain * 2); ch->env_stage = 3; }
                    break;
                case 3: // Sustain
                    // Held by Update()
                    break;
                case 4: // Release
                    ch->env_vol -= (ch->params.release > 0) ? (256 / ch->params.release) : 20;
                    if (ch->env_vol <= 0) { ch->env_vol = 0; ch->active = 0; ch->env_stage = 0; }
                    break;
            }

            if (ch->active == 0) break;

            // Oscillator
            ch->phase += ch->increment;
            u32 phase_step = (ch->phase >> FX_BITS) & 0xFF; // 0-255 cycle
            s8 sample = 0;

            switch (ch->params.wave) {
                case 0: // Square
                    sample = (phase_step < (128 + ch->params.pwm)) ? 127 : -128;
                    break;
                case 1: // Saw
                    sample = (s8)(phase_step - 128);
                    break;
                case 2: // Triangle
                    if (phase_step < 128) sample = -128 + (phase_step * 2);
                    else sample = 127 - ((phase_step - 128) * 2);
                    break;
                case 3: // Noise
                    if ((phase_step & 0xF) == 0) sample = (s8)fast_rand();
                    else sample = (s8)fast_rand(); // Simple white noise
                    break;
            }

            // Apply Volume
            s32 output = (sample * ch->env_vol) >> 8;
            output = (output * ch->params.masterVolume) >> 8;

            // Mix with clipping
            s32 mixed = target[s] + output;
            if (mixed > 127) mixed = 127;
            if (mixed < -128) mixed = -128;
            target[s] = (s8)mixed;
        }
    }
}

void IWRAM_CODE Synth_VBlank_Handler(void) {
    // DMA Transfer for the current buffer (happens automatically via DMA hardware logic, 
    // we just ensure we point to the right data if we were doing manual double buffering 
    // reset, but for circular DMA we usually just let it run. 
    // However, robust GBA audio often resets DMA source every frame to stay in sync).
    
    REG_DMA1CNT = 0; // Stop DMA
    REG_DMA1SAD = (u32)audio_buffer[active_buffer];
    REG_DMA1DAD = AUDIO_FIFO_ADDR;
    REG_DMA1CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA_32 | DMA_SPECIAL | DMA_ENABLE;

    // Swap buffer and mix next frame
    active_buffer = !active_buffer;
    MixAudio(audio_buffer[!active_buffer], SAMPLES_PER_FRAME);
}

void Synth_Init(void) {
    // 1. Clear State
    ma_memzero(channels, sizeof(channels));
    ma_memzero(audio_buffer, sizeof(audio_buffer));
    active_buffer = 0;

    // 2. Enable Sound Controller
    REG_SOUNDCNT_X = 0x80; // Master enable
    REG_SOUNDCNT_L = 0;    // Disable standard channels for now
    
    // Enable DirectSound A, Full Volume, Timer 0, Reset FIFO
    // (1<<2) = Enable DSA Right, (1<<3) = Enable DSA Left
    // (1<<5) = FIFO Reset DSA (0x0020), not 0x0200 (Bit 9 is DSB)
    REG_SOUNDCNT_H = 0x0020 | 0x000C | 0x0003; // DSA 100%, Timer 0

    // 3. Setup Timer 0 for Sampling Rate
    // CYCLES = 16777216 / SAMPLE_RATE
    // 16777216 / 18157 = 924
    // Overflow = 65536 - 924 = 64612
    REG_TM0D = 65536 - (16777216 / SAMPLE_RATE);
    REG_TM0CNT = TIMER_ENABLE;

    // 4. Setup Interrupts
    irqSet(IRQ_VBLANK, Synth_VBlank_Handler);
    irqEnable(IRQ_VBLANK);
    
    // Pre-mix first buffer
    MixAudio(audio_buffer[0], SAMPLES_PER_FRAME);
    MixAudio(audio_buffer[1], SAMPLES_PER_FRAME);

    // Start DMA immediately so we have sound
    Synth_VBlank_Handler();
}

void Synth_Update(void) {
    // Called periodically (e.g. by main loop or sequencer)
    // Updates LFOs or macro parameters if needed
    sample_count_lfo++;
}

void Synth_SetParams(u32 ch, const SynthChannelParams* params) {
    if (ch >= CHANNEL_COUNT) return;
    channels[ch].params = *params;
    
    // Update frequency derived from note if it was already playing
    if (channels[ch].active) {
        // Re-calc increment? Usually done in SetNote or PlaySfx
    }
}

void Synth_SetNote(u32 ch, u32 note, u32 finetune) {
    if (ch >= CHANNEL_COUNT) return;
    if (note > 127) note = 127;

    channels[ch].note = note;
    
    // Calculate frequency
    // From notes.c: note_table contains frequencies (or periods?)
    // If they are frequencies (Hz):
    // Increment = (Hz * (1<<FX_BITS)) / SAMPLE_RATE
    
    u32 freq = note_table[note];
    
    // Apply finetune (simple interpolation approximation)
    if (finetune > 0 && note < 127) {
        u32 next_freq = note_table[note + 1];
        freq += ((next_freq - freq) * finetune) >> 8; // assuming finetune is 0-255
    }

    channels[ch].frequency = freq;
    
    // Pre-calculate phase increment
    channels[ch].increment = (freq << FX_BITS) / SAMPLE_RATE;
}

void Synth_NoteOn(u32 ch) {
    if (ch >= CHANNEL_COUNT) return;
    
    // Restart envelope
    channels[ch].active = 1;
    channels[ch].env_stage = 1; // Attack
    channels[ch].env_vol = 0;
    channels[ch].phase = 0;
}

void Synth_NoteOff(u32 ch) {
    if (ch >= CHANNEL_COUNT) return;

    if (channels[ch].active) {
        channels[ch].env_stage = 4; // Release
    }
}

void Synth_SetVibrato(u32 ch, u8 depth, u8 speed) {
    if (ch >= CHANNEL_COUNT) return;
    channels[ch].params.vibratoDepth = depth;
    channels[ch].params.vibratoSpeed = speed;
}

void Synth_PlaySfx(const SynthChannelParams* params, u32 note, u32 finetune) {
    // Find a free or oldest channel
    u32 ch = 0;
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        if (!channels[i].active) {
            ch = i;
            break;
        }
    }
    
    // Fallback: steal channel 3
    if (channels[ch].active) ch = CHANNEL_COUNT - 1;

    Synth_SetParams(ch, params);
    Synth_SetNote(ch, note, finetune);
    Synth_NoteOn(ch);
}
