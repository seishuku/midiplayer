#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define MAX_VOICES 128
#define PI 3.14159265359f
#define TWO_PI (2.0f*PI)

// FM Patch definition
typedef struct
{
	float modRatio;
	float modIndex;
	float modRatio2;
	float modIndex2;
	float attack;
	float decay;
	float sustain;
	float release;
	float ampScale;
} FM_Patch_t;

static const FM_Patch_t GMPatches[128]=
{
	{ 1.0f, 1.2f, 2.0f, 0.30f, 0.002f, 0.80f, 0.30f, 0.50f, 0.90f },	// 0  Acoustic Grand Piano
	{ 1.0f, 1.5f, 3.0f, 0.20f, 0.002f, 0.60f, 0.20f, 0.40f, 0.90f },	// 1  Bright Acoustic Piano
	{ 1.0f, 1.8f, 2.0f, 0.40f, 0.003f, 0.50f, 0.40f, 0.40f, 0.85f },	// 2  Electric Grand Piano
	{ 1.0f, 2.0f, 3.0f, 0.50f, 0.003f, 0.50f, 0.30f, 0.30f, 0.85f },	// 3  Honky-tonk Piano
	{ 1.0f, 0.8f, 5.0f, 0.15f, 0.004f, 0.90f, 0.50f, 0.60f, 0.80f },	// 4  Electric Piano 1 (Rhodes)
	{ 1.0f, 1.0f, 6.0f, 0.20f, 0.003f, 0.70f, 0.50f, 0.50f, 0.80f },	// 5  Electric Piano 2 (DX7)
	{ 2.0f, 3.0f, 4.0f, 0.50f, 0.001f, 0.20f, 0.00f, 0.10f, 0.90f },	// 6  Harpsichord
	{ 1.0f, 3.5f, 5.0f, 0.60f, 0.001f, 0.30f, 0.10f, 0.15f, 0.90f },	// 7  Clavinet
	{ 2.0f, 1.5f, 4.0f, 0.30f, 0.001f, 0.40f, 0.10f, 0.50f, 0.70f },	// 8  Celesta
	{ 2.0f, 1.0f, 4.0f, 0.20f, 0.001f, 0.50f, 0.05f, 0.60f, 0.70f },	// 9  Glockenspiel
	{ 1.5f, 0.8f, 3.0f, 0.10f, 0.001f, 0.60f, 0.10f, 0.80f, 0.65f },	// 10 Music Box
	{ 1.0f, 0.5f, 2.0f, 0.10f, 0.003f, 0.30f, 0.60f, 0.90f, 0.75f },	// 11 Vibraphone
	{ 1.0f, 2.0f, 4.0f, 0.30f, 0.001f, 0.30f, 0.05f, 0.30f, 0.80f },	// 12 Marimba
	{ 2.0f, 2.5f, 4.0f, 0.40f, 0.001f, 0.20f, 0.02f, 0.20f, 0.80f },	// 13 Xylophone
	{ 1.5f, 2.0f, 3.5f, 0.60f, 0.002f, 0.80f, 0.20f, 1.50f, 0.70f },	// 14 Tubular Bells
	{ 2.0f, 1.5f, 3.0f, 0.30f, 0.002f, 0.40f, 0.20f, 0.40f, 0.75f },	// 15 Dulcimer
	{ 1.0f, 3.0f, 2.0f, 1.00f, 0.008f, 0.01f, 0.90f, 0.05f, 0.85f },	// 16 Drawbar Organ
	{ 2.0f, 2.0f, 3.0f, 0.80f, 0.004f, 0.10f, 0.70f, 0.10f, 0.85f },	// 17 Percussive Organ
	{ 1.0f, 4.0f, 2.0f, 1.50f, 0.006f, 0.02f, 0.90f, 0.06f, 0.85f },	// 18 Rock Organ
	{ 1.0f, 2.5f, 3.0f, 0.80f, 0.015f, 0.10f, 0.90f, 0.30f, 0.80f },	// 19 Church Organ
	{ 1.0f, 5.0f, 2.0f, 1.20f, 0.010f, 0.05f, 0.80f, 0.15f, 0.80f },	// 20 Reed Organ
	{ 1.0f, 4.0f, 3.0f, 0.80f, 0.012f, 0.05f, 0.85f, 0.12f, 0.80f },	// 21 Accordion
	{ 1.0f, 6.0f, 2.0f, 1.00f, 0.008f, 0.05f, 0.80f, 0.10f, 0.75f },	// 22 Harmonica
	{ 1.0f, 5.5f, 2.0f, 1.10f, 0.010f, 0.06f, 0.80f, 0.12f, 0.75f },	// 23 Bandoneon
	{ 2.0f, 1.5f, 3.0f, 0.30f, 0.002f, 0.50f, 0.20f, 0.40f, 0.85f },	// 24 Acoustic Guitar (nylon)
	{ 2.0f, 2.0f, 4.0f, 0.40f, 0.001f, 0.40f, 0.15f, 0.35f, 0.85f },	// 25 Acoustic Guitar (steel)
	{ 1.0f, 1.5f, 3.0f, 0.50f, 0.003f, 0.50f, 0.40f, 0.50f, 0.85f },	// 26 Electric Guitar (jazz)
	{ 1.0f, 2.0f, 4.0f, 0.40f, 0.002f, 0.40f, 0.30f, 0.40f, 0.85f },	// 27 Electric Guitar (clean)
	{ 1.0f, 3.0f, 5.0f, 0.50f, 0.001f, 0.08f, 0.00f, 0.05f, 0.90f },	// 28 Electric Guitar (muted)
	{ 1.0f, 5.0f, 3.0f, 1.50f, 0.003f, 0.10f, 0.70f, 0.15f, 0.80f },	// 29 Overdriven Guitar
	{ 1.0f, 7.0f, 3.0f, 2.50f, 0.003f, 0.08f, 0.80f, 0.10f, 0.75f },	// 30 Distortion Guitar
	{ 2.0f, 0.8f, 4.0f, 0.20f, 0.001f, 0.30f, 0.10f, 0.50f, 0.70f },	// 31 Guitar Harmonics
	{ 1.0f, 2.5f, 3.0f, 0.60f, 0.004f, 0.30f, 0.50f, 0.30f, 1.00f },	// 32 Acoustic Bass
	{ 1.0f, 2.0f, 3.0f, 0.50f, 0.003f, 0.20f, 0.60f, 0.25f, 1.00f },	// 33 Electric Bass (finger)
	{ 1.0f, 3.0f, 4.0f, 0.60f, 0.002f, 0.15f, 0.50f, 0.20f, 1.00f },	// 34 Electric Bass (pick)
	{ 1.0f, 1.5f, 2.0f, 0.40f, 0.005f, 0.20f, 0.70f, 0.30f, 0.95f },	// 35 Fretless Bass
	{ 1.0f, 4.0f, 3.0f, 1.00f, 0.002f, 0.10f, 0.30f, 0.15f, 0.95f },	// 36 Slap Bass 1
	{ 1.0f, 5.0f, 3.0f, 1.20f, 0.002f, 0.10f, 0.25f, 0.12f, 0.95f },	// 37 Slap Bass 2
	{ 1.0f, 3.0f, 2.0f, 0.80f, 0.005f, 0.20f, 0.60f, 0.20f, 0.95f },	// 38 Synth Bass 1
	{ 1.0f, 4.0f, 2.0f, 1.00f, 0.005f, 0.25f, 0.55f, 0.20f, 0.95f },	// 39 Synth Bass 2
	{ 1.0f, 4.0f, 2.0f, 0.80f, 0.020f, 0.10f, 0.80f, 0.20f, 0.80f },	// 40 Violin
	{ 1.0f, 3.5f, 2.0f, 0.70f, 0.025f, 0.12f, 0.75f, 0.22f, 0.80f },	// 41 Viola
	{ 1.0f, 3.0f, 2.0f, 0.60f, 0.030f, 0.15f, 0.70f, 0.25f, 0.85f },	// 42 Cello
	{ 1.0f, 2.5f, 2.0f, 0.50f, 0.035f, 0.18f, 0.65f, 0.30f, 0.90f },	// 43 Contrabass
	{ 1.0f, 3.5f, 2.0f, 0.60f, 0.010f, 0.08f, 0.80f, 0.15f, 0.75f },	// 44 Tremolo Strings
	{ 2.0f, 2.0f, 3.0f, 0.40f, 0.001f, 0.20f, 0.00f, 0.30f, 0.75f },	// 45 Pizzicato Strings
	{ 2.0f, 1.5f, 4.0f, 0.30f, 0.001f, 0.50f, 0.15f, 0.60f, 0.75f },	// 46 Orchestral Harp
	{ 1.0f, 3.0f, 0.5f, 0.80f, 0.003f, 0.60f, 0.20f, 0.80f, 0.90f },	// 47 Timpani
	{ 1.0f, 3.0f, 2.0f, 0.50f, 0.025f, 0.15f, 0.75f, 0.40f, 0.75f },	// 48 String Ensemble 1
	{ 1.0f, 2.5f, 2.0f, 0.40f, 0.030f, 0.20f, 0.70f, 0.50f, 0.75f },	// 49 String Ensemble 2
	{ 1.0f, 2.0f, 3.0f, 0.60f, 0.020f, 0.20f, 0.75f, 0.40f, 0.75f },	// 50 Synth Strings 1
	{ 1.0f, 1.8f, 3.0f, 0.50f, 0.025f, 0.25f, 0.70f, 0.45f, 0.75f },	// 51 Synth Strings 2
	{ 1.0f, 2.0f, 3.0f, 0.40f, 0.050f, 0.15f, 0.80f, 0.50f, 0.70f },	// 52 Choir Aahs
	{ 1.0f, 1.5f, 2.0f, 0.30f, 0.060f, 0.20f, 0.75f, 0.55f, 0.70f },	// 53 Voice Oohs
	{ 1.0f, 2.5f, 3.0f, 0.50f, 0.040f, 0.15f, 0.80f, 0.50f, 0.70f },	// 54 Synth Voice
	{ 1.0f, 5.0f, 3.0f, 1.50f, 0.003f, 0.30f, 0.40f, 0.40f, 0.85f },	// 55 Orchestra Hit
	{ 1.0f, 5.0f, 2.0f, 1.00f, 0.008f, 0.05f, 0.80f, 0.10f, 0.85f },	// 56 Trumpet
	{ 1.0f, 4.5f, 2.0f, 0.90f, 0.010f, 0.06f, 0.75f, 0.12f, 0.85f },	// 57 Trombone
	{ 1.0f, 4.0f, 2.0f, 0.80f, 0.012f, 0.08f, 0.70f, 0.15f, 0.90f },	// 58 Tuba
	{ 1.0f, 6.0f, 3.0f, 1.20f, 0.006f, 0.04f, 0.75f, 0.08f, 0.80f },	// 59 Muted Trumpet
	{ 1.0f, 3.5f, 2.0f, 0.70f, 0.015f, 0.10f, 0.70f, 0.20f, 0.80f },	// 60 French Horn
	{ 1.0f, 4.0f, 2.0f, 0.90f, 0.010f, 0.06f, 0.75f, 0.12f, 0.80f },	// 61 Brass Section
	{ 1.0f, 3.0f, 2.0f, 0.80f, 0.008f, 0.06f, 0.80f, 0.10f, 0.80f },	// 62 Synth Brass 1
	{ 1.0f, 2.5f, 2.0f, 0.60f, 0.010f, 0.08f, 0.80f, 0.12f, 0.80f },	// 63 Synth Brass 2
	{ 1.0f, 5.5f, 2.0f, 1.00f, 0.010f, 0.08f, 0.75f, 0.15f, 0.80f },	// 64 Soprano Sax
	{ 1.0f, 5.0f, 2.0f, 0.90f, 0.012f, 0.09f, 0.70f, 0.18f, 0.80f },	// 65 Alto Sax
	{ 1.0f, 4.5f, 2.0f, 0.80f, 0.014f, 0.10f, 0.70f, 0.20f, 0.80f },	// 66 Tenor Sax
	{ 1.0f, 4.0f, 2.0f, 0.70f, 0.016f, 0.12f, 0.65f, 0.25f, 0.85f },	// 67 Baritone Sax
	{ 1.0f, 6.0f, 3.0f, 1.20f, 0.010f, 0.07f, 0.75f, 0.12f, 0.75f },	// 68 Oboe
	{ 1.0f, 5.5f, 3.0f, 1.00f, 0.012f, 0.08f, 0.70f, 0.15f, 0.75f },	// 69 English Horn
	{ 1.0f, 5.0f, 2.0f, 0.80f, 0.015f, 0.10f, 0.65f, 0.20f, 0.80f },	// 70 Bassoon
	{ 1.0f, 3.0f, 5.0f, 0.50f, 0.012f, 0.08f, 0.75f, 0.15f, 0.75f },	// 71 Clarinet
	{ 1.0f, 2.0f, 3.0f, 0.40f, 0.008f, 0.06f, 0.75f, 0.10f, 0.70f },	// 72 Piccolo
	{ 1.0f, 1.5f, 2.0f, 0.30f, 0.010f, 0.06f, 0.80f, 0.12f, 0.70f },	// 73 Flute
	{ 1.0f, 1.8f, 2.0f, 0.35f, 0.009f, 0.06f, 0.75f, 0.12f, 0.70f },	// 74 Recorder
	{ 1.0f, 1.2f, 2.0f, 0.25f, 0.012f, 0.08f, 0.70f, 0.20f, 0.70f },	// 75 Pan Flute
	{ 1.0f, 2.0f, 3.0f, 0.50f, 0.008f, 0.10f, 0.60f, 0.15f, 0.65f },	// 76 Blown Bottle
	{ 1.0f, 2.5f, 2.0f, 0.60f, 0.015f, 0.10f, 0.65f, 0.20f, 0.65f },	// 77 Shakuhachi
	{ 1.0f, 0.8f, 2.0f, 0.15f, 0.008f, 0.05f, 0.85f, 0.10f, 0.65f },	// 78 Whistle
	{ 1.0f, 1.0f, 3.0f, 0.20f, 0.010f, 0.06f, 0.80f, 0.12f, 0.65f },	// 79 Ocarina
	{ 1.0f, 3.0f, 2.0f, 0.80f, 0.005f, 0.05f, 0.85f, 0.10f, 0.80f },	// 80 Lead 1 (square)
	{ 1.0f, 4.0f, 2.0f, 1.20f, 0.004f, 0.04f, 0.90f, 0.08f, 0.80f },	// 81 Lead 2 (sawtooth)
	{ 1.0f, 1.5f, 3.0f, 0.40f, 0.006f, 0.05f, 0.85f, 0.10f, 0.75f },	// 82 Lead 3 (calliope)
	{ 1.0f, 6.0f, 4.0f, 1.50f, 0.003f, 0.10f, 0.60f, 0.15f, 0.75f },	// 83 Lead 4 (chiff)
	{ 1.0f, 5.0f, 3.0f, 1.30f, 0.004f, 0.06f, 0.75f, 0.10f, 0.75f },	// 84 Lead 5 (charang)
	{ 1.0f, 2.5f, 3.0f, 0.50f, 0.030f, 0.10f, 0.80f, 0.20f, 0.70f },	// 85 Lead 6 (voice)
	{ 1.5f, 3.0f, 2.0f, 0.70f, 0.005f, 0.05f, 0.85f, 0.10f, 0.75f },	// 86 Lead 7 (fifths)
	{ 1.0f, 4.0f, 2.0f, 1.00f, 0.005f, 0.05f, 0.85f, 0.10f, 0.80f },	// 87 Lead 8 (bass+lead)
	{ 1.0f, 1.5f, 2.0f, 0.30f, 0.050f, 0.30f, 0.80f, 0.80f, 0.65f },	// 88 Pad 1 (new age)
	{ 1.0f, 1.2f, 2.0f, 0.25f, 0.060f, 0.35f, 0.80f, 1.00f, 0.65f },	// 89 Pad 2 (warm)
	{ 1.0f, 2.0f, 3.0f, 0.50f, 0.040f, 0.30f, 0.75f, 0.80f, 0.65f },	// 90 Pad 3 (polysynth)
	{ 1.0f, 1.8f, 2.5f, 0.40f, 0.055f, 0.30f, 0.80f, 1.00f, 0.65f },	// 91 Pad 4 (choir)
	{ 1.0f, 1.5f, 3.0f, 0.30f, 0.060f, 0.40f, 0.75f, 1.20f, 0.60f },	// 92 Pad 5 (bowed glass)
	{ 1.5f, 2.5f, 3.5f, 0.60f, 0.030f, 0.40f, 0.70f, 1.00f, 0.65f },	// 93 Pad 6 (metallic)
	{ 1.0f, 2.0f, 2.0f, 0.40f, 0.070f, 0.50f, 0.70f, 1.50f, 0.60f },	// 94 Pad 7 (halo)
	{ 0.5f, 3.0f, 1.5f, 0.70f, 0.060f, 0.60f, 0.65f, 1.50f, 0.60f },	// 95 Pad 8 (sweep)
	{ 2.0f, 3.0f, 5.0f, 0.80f, 0.020f, 0.50f, 0.40f, 0.80f, 0.60f },	// 96  FX 1 (rain)
	{ 1.0f, 2.0f, 3.0f, 0.60f, 0.040f, 0.50f, 0.60f, 1.00f, 0.60f },	// 97  FX 2 (soundtrack)
	{ 2.0f, 1.5f, 4.0f, 0.40f, 0.003f, 0.60f, 0.20f, 1.00f, 0.65f },	// 98  FX 3 (crystal)
	{ 1.0f, 1.8f, 2.0f, 0.50f, 0.060f, 0.60f, 0.65f, 1.50f, 0.60f },	// 99  FX 4 (atmosphere)
	{ 2.0f, 2.5f, 4.0f, 0.60f, 0.010f, 0.40f, 0.50f, 0.80f, 0.65f },	// 100 FX 5 (brightness)
	{ 1.0f, 4.0f, 3.0f, 1.00f, 0.030f, 0.50f, 0.60f, 1.00f, 0.60f },	// 101 FX 6 (goblins)
	{ 1.0f, 2.5f, 3.0f, 0.70f, 0.015f, 0.60f, 0.50f, 1.20f, 0.60f },	// 102 FX 7 (echoes)
	{ 1.5f, 4.0f, 2.5f, 1.20f, 0.010f, 0.40f, 0.60f, 1.00f, 0.60f },	// 103 FX 8 (sci-fi)
	{ 2.0f, 3.0f, 5.0f, 0.60f, 0.002f, 0.40f, 0.30f, 0.50f, 0.75f },	// 104 Sitar
	{ 2.0f, 2.5f, 4.0f, 0.50f, 0.001f, 0.30f, 0.10f, 0.35f, 0.80f },	// 105 Banjo
	{ 1.5f, 3.0f, 4.0f, 0.60f, 0.001f, 0.35f, 0.15f, 0.30f, 0.80f },	// 106 Shamisen
	{ 2.0f, 2.0f, 4.0f, 0.40f, 0.001f, 0.40f, 0.10f, 0.50f, 0.75f },	// 107 Koto
	{ 2.0f, 1.5f, 4.0f, 0.30f, 0.001f, 0.50f, 0.05f, 0.60f, 0.70f },	// 108 Kalimba
	{ 1.0f, 5.0f, 2.0f, 1.20f, 0.015f, 0.05f, 0.85f, 0.10f, 0.75f },	// 109 Bag pipe
	{ 1.0f, 4.0f, 2.0f, 0.80f, 0.018f, 0.10f, 0.80f, 0.20f, 0.75f },	// 110 Fiddle
	{ 1.0f, 6.0f, 3.0f, 1.30f, 0.010f, 0.07f, 0.75f, 0.12f, 0.70f },	// 111 Shanai
	{ 3.0f, 2.0f, 5.0f, 0.50f, 0.001f, 0.40f, 0.05f, 0.50f, 0.70f },	// 112 Tinkle Bell
	{ 2.5f, 2.5f, 5.0f, 0.60f, 0.001f, 0.30f, 0.05f, 0.30f, 0.70f },	// 113 Agogo
	{ 2.0f, 1.5f, 4.0f, 0.40f, 0.001f, 0.35f, 0.10f, 0.40f, 0.75f },	// 114 Steel Drums
	{ 3.0f, 4.0f, 6.0f, 1.00f, 0.001f, 0.05f, 0.00f, 0.05f, 0.80f },	// 115 Woodblock
	{ 0.5f, 5.0f, 2.0f, 1.50f, 0.002f, 0.40f, 0.10f, 0.30f, 0.90f },	// 116 Taiko Drum
	{ 1.0f, 3.0f, 2.0f, 0.80f, 0.002f, 0.35f, 0.15f, 0.30f, 0.85f },	// 117 Melodic Tom
	{ 0.5f, 6.0f, 2.0f, 2.00f, 0.002f, 0.50f, 0.10f, 0.40f, 0.85f },	// 118 Synth Drum
	{ 1.0f, 2.0f, 3.0f, 0.50f, 0.500f, 0.01f, 0.80f, 0.05f, 0.70f },	// 119 Reverse Cymbal
	{ 3.0f, 5.0f, 7.0f, 1.00f, 0.001f, 0.10f, 0.00f, 0.10f, 0.40f },	// 120 Guitar Fret Noise
	{ 1.0f, 1.0f, 2.0f, 0.20f, 0.008f, 0.10f, 0.30f, 0.20f, 0.40f },	// 121 Breath Noise
	{ 1.0f, 0.5f, 2.0f, 0.10f, 0.100f, 0.50f, 0.60f, 1.00f, 0.40f },	// 122 Seashore
	{ 2.0f, 1.0f, 4.0f, 0.30f, 0.010f, 0.10f, 0.50f, 0.20f, 0.40f },	// 123 Bird Tweet
	{ 3.0f, 4.0f, 6.0f, 0.80f, 0.005f, 0.10f, 0.80f, 0.10f, 0.50f },	// 124 Telephone Ring
	{ 0.5f, 8.0f, 1.0f, 2.00f, 0.010f, 0.10f, 0.90f, 0.20f, 0.50f },	// 125 Helicopter
	{ 1.0f, 0.5f, 2.0f, 0.10f, 0.200f, 0.50f, 0.70f, 1.50f, 0.40f },	// 126 Applause
	{ 1.0f, 8.0f, 3.0f, 3.00f, 0.001f, 0.30f, 0.00f, 0.20f, 0.80f },	// 127 Gunshot
};

// Per-channel state
typedef struct
{
	int program;
	float volume;
	float expression;
	float pan;
	float pitchBend;
} Channel_t;

Channel_t channels[16];

// Voice
typedef struct
{
	bool active;
	bool released;
	int channel;
	int note;

	float baseFrequency;
	float phase;
	float modPhase;
	float modPhase2;

	float velocity;
	float envelopePhase;
	float envelopeValue;
	float noiseState;

	float drumFreq;
	float drumPitchEnv;
} Voice_t;

Voice_t voices[MAX_VOICES];

// Utilities
float RandomNoise(float *state)
{
	*state=fmodf(*state*1.7f+1.3f, 1.0f);

	return *state*2.0f-1.0f;
}

static float NoteToFreq(int note)
{
	return 440.0f*powf(2.0f, (note-69)/12.0f);
}

// Note on/off
void MIDI_NoteOn(int channel, int note, int velocity)
{
	// Find free voice; steal oldest released voice if needed
	int slot = -1;

	for(int i=0;i<MAX_VOICES;i++)
	{
		if(!voices[i].active)
		{
			slot=i;
			break;
		}
	}

	if(slot<0)
	{
		// steal first released voice
		for(int i=0;i<MAX_VOICES;i++)
		{
			if(voices[i].released)
			{
				slot=i;
				break;
			}
		}
	}

	if(slot<0)
		return; // all 128 voices truly busy

	Voice_t *v=&voices[slot];
	memset(v, 0, sizeof(*v));

	v->active=true;
	v->released=false;
	v->channel=channel;
	v->note=note;
	v->velocity=velocity/127.0f;
	v->envelopePhase=0.0f;
	v->envelopeValue=0.0f;
	v->noiseState=(float)(note^(velocity<<8)^(channel<<16))/(float)0x7FFFFF;

	// Drums
	if(channel==9)
	{
		switch(note)
		{
			case 35:
			case 36:	v->drumFreq=55.0f;	break;	// kick
			case 41:	v->drumFreq=82.0f;	break;	// low floor tom
			case 43:	v->drumFreq=98.0f;	break;	// high floor tom
			case 45:	v->drumFreq=110.0f;	break;	// low tom
			case 47:	v->drumFreq=131.0f;	break;	// low-mid tom
			case 48:	v->drumFreq=147.0f;	break;	// hi-mid tom
			case 50:	v->drumFreq=175.0f;	break;	// high tom
			default:	v->drumFreq=200.0f;	break;
		}

		v->drumPitchEnv=1.0f;
		v->envelopeValue=1.0f;
		v->envelopePhase=0.001f;
	}
	else
		v->baseFrequency=NoteToFreq(note);
}

void MIDI_NoteOff(int channel, int note)
{
	for(int i=0;i<MAX_VOICES;i++)
	{
		if(voices[i].active&&voices[i].note==note&&voices[i].channel==channel&&!voices[i].released)
		{
			voices[i].released=true;
			voices[i].envelopePhase=0.0f;
		}
	}
}

// Audio callback
static double globalTime=0.0;

void mix_audio(void *userdata, Uint8 *stream, int len)
{
	float *buffer=(float *)stream;
	int samples=len/sizeof(float)/2;

	memset(buffer, 0, len);

	const float dt=1.0f/SAMPLE_RATE;

	for(int i=0;i<samples;i++)
	{
		float sampleL=0.0f, sampleR=0.0f;

		for(int vi=0;vi<MAX_VOICES;vi++)
		{
			if(!voices[vi].active)
				continue;
			
			Voice_t *v=&voices[vi];
			Channel_t *ch=&channels[v->channel];

			// patch selection
			const FM_Patch_t *patch;

			// build a quick patch for each drum type
			FM_Patch_t drumPatch;
			if(v->channel==9)
			{
				switch(v->note)
				{
					case 35:
					case 36: // kick
						drumPatch.attack=0.001f;
						drumPatch.decay=0.5f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.15f;
						drumPatch.modRatio=0.5f;
						drumPatch.modIndex=3.0f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 38:
					case 40: // snare/electric snare
						drumPatch.attack=0.001f;
						drumPatch.decay=0.18f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.06f;
						drumPatch.modRatio=2.0f;
						drumPatch.modIndex=0.5f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 42: // closed hi-hat
						drumPatch.attack=0.001f;
						drumPatch.decay=0.04f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.02f;
						drumPatch.modRatio=6.0f;
						drumPatch.modIndex=1.5f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 44: // pedal hi-hat
						drumPatch.attack=0.001f;
						drumPatch.decay=0.06f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.03f;
						drumPatch.modRatio=6.0f;
						drumPatch.modIndex=1.5f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 46: // open hi-hat
						drumPatch.attack=0.001f;
						drumPatch.decay=0.30f;
						drumPatch.sustain=0.05f;
						drumPatch.release=0.15f;
						drumPatch.modRatio=6.0f;
						drumPatch.modIndex=1.5f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 39: // clap
						drumPatch.attack=0.001f;
						drumPatch.decay=0.10f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.06f;
						drumPatch.modRatio=1.0f;
						drumPatch.modIndex=0.0f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 49:
					case 51:
					case 55:
					case 57: // crash/ride
						drumPatch.attack=0.001f;
						drumPatch.decay=0.8f;
						drumPatch.sustain=0.1f;
						drumPatch.release=1.0f;
						drumPatch.modRatio=7.0f;
						drumPatch.modIndex=2.0f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					case 41:
					case 43:
					case 45:
					case 47:
					case 48:
					case 50: // toms
						drumPatch.attack=0.001f;
						drumPatch.decay=0.25f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.12f;
						drumPatch.modRatio=0.5f;
						drumPatch.modIndex=2.0f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
					default:
						drumPatch.attack=0.001f;
						drumPatch.decay=0.3f;
						drumPatch.sustain=0.0f;
						drumPatch.release=0.1f;
						drumPatch.modRatio=2.0f;
						drumPatch.modIndex=1.0f;
						drumPatch.modRatio2=0.0f;
						drumPatch.modIndex2=0.0f;
						break;
				}

				patch=&drumPatch;
			}
			else
				patch=&GMPatches[ch->program];

			// envelope
			if(!v->released)
			{
				if(v->envelopePhase<patch->attack)
					v->envelopeValue=v->envelopePhase/patch->attack;
				else if(v->envelopePhase<patch->attack+patch->decay)
					v->envelopeValue=1.0f-(1.0f-patch->sustain)*((v->envelopePhase-patch->attack)/patch->decay);
				else
					v->envelopeValue=patch->sustain;

				v->envelopePhase+=dt;
			}
			else
			{
				float rel=(patch->release>0.001f)?patch->release:0.001f;

				v->envelopeValue-=dt/rel;

				if(v->envelopeValue<=0.0f)
				{
					v->active=false;
					continue;
				}
			}

			// amplitude
			float channelAmp=ch->volume*ch->expression*patch->ampScale;
			float amplitude=v->velocity*v->envelopeValue*channelAmp*0.18f;

			// --- synthesis ---
			float tone=0.0f;

			if(v->channel==9)
			{
				// Advance the dedicated pitch envelope — fast exponential drop,
				// completely independent of the amplitude ADSR.
				// Kick drops in ~60ms, toms in ~120ms.
				float freq=v->drumFreq;

				switch(v->note)
				{
					case 35:
					case 36:
					{
						// Kick
						v->drumPitchEnv*=powf(0.0001f, dt/0.060f);

						float pitchFactor=1.0f+v->drumPitchEnv*8.0f;

						v->phase+=TWO_PI*freq*pitchFactor*dt;
						v->modPhase+=TWO_PI*freq*pitchFactor*patch->modRatio*dt;
						
						tone=sinf(v->phase+sinf(v->modPhase)*patch->modIndex);
						break;
					}

					case 38:
					case 40:
					{
						// Snare
						v->phase+=TWO_PI*200.0f*dt;

						tone=RandomNoise(&v->noiseState)*0.7f+sinf(v->phase)*0.3f*v->envelopeValue;
						break;
					}

					case 39:
					{
						// Clap
						float burst=(v->envelopePhase<0.008f)?1.0f:(v->envelopePhase<0.020f)?0.3f:1.0f;

						tone=RandomNoise(&v->noiseState)*v->envelopeValue*burst;
						break;
					}

					case 42:
					case 44:
					{
						// Closed hi-hat
						v->phase+=TWO_PI*freq*patch->modRatio*dt;
						v->modPhase+=TWO_PI*freq*patch->modRatio*1.4f*dt;

						tone=(RandomNoise(&v->noiseState)*0.6f+sinf(v->phase+(sinf(v->modPhase)*patch->modIndex))*0.4f)*powf(v->envelopeValue, 2.5f);
						break;
					}

					case 46:
					{
						// Open hi-hat
						v->phase+=TWO_PI*freq*patch->modRatio*dt;
						v->modPhase+=TWO_PI*freq*patch->modRatio*1.4f*dt;

						tone=(RandomNoise(&v->noiseState)*0.6f+sinf(v->phase+(sinf(v->modPhase)*patch->modIndex))*0.4f)*v->envelopeValue;
						break;
					}

					case 49:
					case 51:
					case 55:
					case 57:
					{
						// Crash/ride
						v->phase+=TWO_PI*freq*patch->modRatio*dt;
						v->modPhase+=TWO_PI*freq*patch->modRatio*1.41f*dt;

						tone=(sinf(v->phase+(sinf(v->modPhase)*patch->modIndex))*0.5f+RandomNoise(&v->noiseState)*0.5f)*v->envelopeValue;
						break;
					}

					default:
					{
						// Toms
						v->drumPitchEnv*=powf(0.0001f, dt/0.120f);

						float pitchFactor=1.0f+v->drumPitchEnv*3.0f;

						v->phase+=TWO_PI*freq*pitchFactor*dt;
						v->modPhase+=TWO_PI*freq*pitchFactor*patch->modRatio*dt;

						tone=sinf(v->phase+sinf(v->modPhase)*patch->modIndex);
						break;
					}
				}
			}
			else
			{
				// 2-operator FM with pitch bend
				float freq=v->baseFrequency*powf(2.0f, ch->pitchBend/12.0f);

				float mod1=sinf(v->modPhase)*patch->modIndex;
				float mod2=(patch->modRatio2>0.0f)?sinf(v->modPhase2)*patch->modIndex2:0.0f;

				tone=sinf(v->phase+mod1+mod2);

				v->phase+=TWO_PI*freq*dt;
				v->modPhase+=TWO_PI*freq*patch->modRatio*dt;
				v->modPhase2+=TWO_PI*freq*patch->modRatio2*dt;

				v->phase=fmodf(v->phase, TWO_PI);
				v->modPhase=fmodf(v->modPhase, TWO_PI);
				v->modPhase2=fmodf(v->modPhase2, TWO_PI);
			}

			float out=tone*amplitude;

			// simple stereo pan
			float panAngle=(ch->pan+1.0f)*0.25f*PI;
			sampleL+=out*cosf(panAngle);
			sampleR+=out*sinf(panAngle);
		}

		// soft clip
		sampleL=tanhf(sampleL);
		sampleR=tanhf(sampleR);

		buffer[i*2+0]=sampleL;
		buffer[i*2+1]=sampleR;

		globalTime+=1.0/SAMPLE_RATE;
	}
}

// MIDI file parser
static uint32_t ReadBIG32(FILE *f)
{
	uint8_t b[4];

	fread(b, 1, 4, f);

	return ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];
}

static uint16_t ReadBIG16(FILE *f)
{
	uint8_t b[2];

	fread(b, 1, 2, f);

	return (uint16_t)((b[0]<<8)|b[1]);
}

static uint32_t ReadVariableLength(FILE *f)
{
	uint32_t val=0;
	uint8_t byte;

	do
	{
		fread(&byte, 1, 1, f);

		val=(val<<7)|(byte & 0x7F);
	}
	while(byte&0x80);

	return val;
}

typedef struct
{
	uint32_t time;
	uint8_t status;
	uint8_t data1;
	uint8_t data2;
	int channel;
} MIDI_Event_t;

typedef struct
{
	MIDI_Event_t *events;
	size_t count, capacity;
} MIDI_EventList_t;

static void MIDI_AddEvent(MIDI_EventList_t *list, MIDI_Event_t evt)
{
	if(list->count>=list->capacity)
	{
		list->capacity=list->capacity*2+64;
		list->events=realloc(list->events, list->capacity*sizeof(MIDI_Event_t));
	}

	list->events[list->count++]=evt;
}

static int CompareEvents(const void *a, const void *b)
{
	uint32_t ta=((const MIDI_Event_t *)a)->time;
	uint32_t tb=((const MIDI_Event_t *)b)->time;

	return (ta>tb)-(ta<tb);
}

typedef struct
{
	uint32_t tick;
	double secondsPerTick;
} TempoEvent_t;

typedef struct
{
	TempoEvent_t *events;
	size_t count, capacity;
} TempoList_t;

static void TempoList_Add(TempoList_t *tl, uint32_t tick, double spt)
{
	if(tl->count>=tl->capacity)
	{
		tl->capacity=tl->capacity*2+8;
		tl->events=realloc(tl->events, tl->capacity*sizeof(TempoEvent_t));
	}
	tl->events[tl->count++]=(TempoEvent_t){tick, spt};
}

static int CompareTempo(const void *a, const void *b)
{
	uint32_t ta=((const TempoEvent_t *)a)->tick;
	uint32_t tb=((const TempoEvent_t *)b)->tick;

	return (ta>tb)-(ta<tb);
}

void MIDI_Parse(const char *filename, MIDI_EventList_t *list, int *ppqOut, TempoList_t *tempos)
{
	FILE *f=fopen(filename, "rb");

	if(!f)
	{
		perror("open");
		exit(1);
	}

	char hdr[4];
	fread(hdr, 1, 4, f);

	if(memcmp(hdr, "MThd", 4))
	{
		printf("Bad MIDI header\n");
		exit(1);
	}

	ReadBIG32(f); // chunk length (always 6)
	ReadBIG16(f); // format (unused)

	uint16_t numTracks=ReadBIG16(f);
	uint16_t pulsePerQuarter=ReadBIG16(f);

	*ppqOut = pulsePerQuarter;

	// Seed with default 120 BPM at tick 0
	TempoList_Add(tempos, 0, 60.0/(120.0*pulsePerQuarter));

	for(int t=0;t<numTracks;t++)
	{
		fread(hdr, 1, 4, f);

		if(memcmp(hdr, "MTrk", 4))
		{
			printf("Bad track header\n");
			exit(1);
		}

		uint32_t len=ReadBIG32(f);

		long start=ftell(f);

		uint32_t time=0;
		uint8_t status=0;

		while(ftell(f)<start+(long)len)
		{
			uint32_t delta=ReadVariableLength(f);

			time+=delta;

			uint8_t peek;
			fread(&peek, 1, 1, f);

			if(peek&0x80)
				status=peek;
			else
				fseek(f, -1, SEEK_CUR);

			if(status==0xFF)
			{
				uint8_t metaType;
				fread(&metaType, 1, 1, f);

				uint32_t metaLen=ReadVariableLength(f);

				if(metaType==0x51&&metaLen==3)
				{
					uint8_t td[3];
					fread(td, 1, 3, f);

					int us=(td[0]<<16)|(td[1]<<8)|td[2];

					TempoList_Add(tempos, time, (double)us/1000000.0/pulsePerQuarter);
				}
				else
					fseek(f, metaLen, SEEK_CUR);

				status=0;
				continue;
			}

			if(status==0xF0||status==0xF7)
			{
				uint32_t slen=ReadVariableLength(f);
				fseek(f, slen, SEEK_CUR);

				status=0;
				continue;
			}

			uint8_t type=status&0xF0;
			uint8_t channel=status&0x0F;

			if(type==0x80||type==0x90)
			{
				uint8_t b1, b2;
				fread(&b1, 1, 1, f);
				fread(&b2, 1, 1, f);

				MIDI_AddEvent(list, (MIDI_Event_t) { time, status, b1, b2, channel });
			}
			else if(type==0xA0)
			{
				uint8_t b1, b2;

				fread(&b1, 1, 1, f);
				fread(&b2, 1, 1, f);
			}
			else if(type==0xB0)
			{
				uint8_t b1, b2;
				fread(&b1, 1, 1, f);
				fread(&b2, 1, 1, f);

				MIDI_AddEvent(list, (MIDI_Event_t) { time, status, b1, b2, channel });
			}
			else if(type==0xC0)
			{
				uint8_t b1;
				fread(&b1, 1, 1, f);

				MIDI_AddEvent(list, (MIDI_Event_t) { time, status, b1, 0, channel });
			}
			else if(type==0xD0)
			{
				uint8_t b1;
				fread(&b1, 1, 1, f);
			}
			else if(type==0xE0)
			{
				uint8_t b1, b2;
				fread(&b1, 1, 1, f);
				fread(&b2, 1, 1, f);

				MIDI_AddEvent(list, (MIDI_Event_t) { time, status, b1, b2, channel });
			}
		}
	}

	qsort(list->events, list->count, sizeof(MIDI_Event_t), CompareEvents);
	qsort(tempos->events, tempos->count, sizeof(TempoEvent_t), CompareTempo);

	size_t out=0;
	for(size_t i=0;i<tempos->count;i++)
	{
		if(out>0&&tempos->events[out-1].tick==tempos->events[i].tick)
			tempos->events[out-1]=tempos->events[i];
		else
			tempos->events[out++]=tempos->events[i];
	}

	tempos->count=out;

	fclose(f);
}

static bool quit=false;

int main(int argc, char **argv)
{
	if(argc<2)
	{
		printf("Usage: %s file.mid\n", argv[0]);
		return 1;
	}

	// Init channel defaults
	for(int i=0;i<16;i++)
	{
		channels[i].program=0;
		channels[i].volume=1.0f;
		channels[i].expression=1.0f;
		channels[i].pan=0.0f;
		channels[i].pitchBend=0.0f;
	}

	MIDI_EventList_t list={ 0 };
	TempoList_t tempos={ 0 };
	int ppq;

	MIDI_Parse(argv[1], &list, &ppq, &tempos);

	SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("MIDI Player - Esc to stop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 60, 0);

	SDL_AudioSpec spec=
	{
	    .freq=SAMPLE_RATE,
	    .format=AUDIO_F32SYS,
	    .channels=2,
	    .samples=512,
	    .callback=mix_audio
	};

	SDL_OpenAudio(&spec, NULL);
	SDL_PauseAudio(0);

	size_t position=0;
	size_t tempoIdx=0;
	double tickAccum=0.0;
	double lastTime=globalTime;

	while(!quit&&position<list.count)
	{
		double now=globalTime;
		double dt=now-lastTime;
		lastTime=now;

		while(dt>0.0)
		{
			double spt=tempos.events[tempoIdx].secondsPerTick;

			if(tempoIdx+1<tempos.count)
			{
				double ticksUntilChange=tempos.events[tempoIdx+1].tick-tickAccum;
				double secsUntilChange=ticksUntilChange*spt;

				if(dt>=secsUntilChange)
				{
					tickAccum+=ticksUntilChange;
					dt-=secsUntilChange;
					tempoIdx++;
					continue;
				}
			}

			tickAccum+=dt/spt;
			break;
		}

		uint32_t currentTick=(uint32_t)tickAccum;

		while(position<list.count&&list.events[position].time<=currentTick)
		{
			MIDI_Event_t *e=&list.events[position++];
			uint8_t type=e->status&0xF0;
			int ch=e->channel;

			if(type==0x90&&e->data2>0)
				MIDI_NoteOn(ch, e->data1, e->data2);
			else if(type==0x80||(type==0x90&&e->data2==0))
				MIDI_NoteOff(ch, e->data1);
			else if(type==0xC0)
				channels[ch].program=e->data1&0x7F;
			else if(type==0xB0)
			{
				switch(e->data1)
				{
					case 7:
						channels[ch].volume=e->data2/127.0f;
						break;

					case 11:
						channels[ch].expression=e->data2/127.0f;
						break;

					case 10:
						channels[ch].pan=(e->data2-64)/63.0f;
						break;

					case 123:
					case 120:
						for(int i=0;i<MAX_VOICES;i++)
						{
							if(voices[i].channel==ch)
							{
								voices[i].released=true;
								voices[i].envelopePhase=0.0f;
							}
						}
						break;

					default:
						break;
				}
			}
			else if(type==0xE0)
			{
				int raw=(e->data2<<7)|e->data1;
				channels[ch].pitchBend=((raw-8192)/8192.0f)*2.0f;
			}
		}

		SDL_Event event;

		while(SDL_PollEvent(&event))
		{
			if(event.type==SDL_QUIT)
				quit=true;

			if(event.type==SDL_KEYDOWN)
			{
				if(event.key.keysym.sym==SDLK_ESCAPE||event.key.keysym.sym==SDLK_q)
					quit=true;
			}
		}

		SDL_Delay(1);
	}

	free(tempos.events);
	free(list.events);

	if(!quit)
		SDL_Delay(2000); // let tails decay on natural end

	SDL_DestroyWindow(window);
	SDL_CloseAudio();
	SDL_Quit();

	return 0;
}
