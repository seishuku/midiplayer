// Compile with: gcc midi_fm_player.c -o midi_fm_player -lSDL2 -lm

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SAMPLE_RATE 44100
#define MAX_VOICES 64
#define PI 3.14159265359

typedef struct
{
    float attack;
    float decay;
    float sustain;
    float release;
} MIDI_Envelope_t;

MIDI_Envelope_t envelopes[16];

typedef struct
{
    bool active;
    bool released;
    int channel;
    int note;

    float freq;
    float phase;
    float modPhase;
    float freqEnvelope;

    float velocity;
    float envelopePhase;
    float envelopeValue;
    float noiseState;
} MIDI_Voice_t;

MIDI_Voice_t voices[MAX_VOICES];

float RandomNoise(float *state)
{
    *state=fmodf(*state*1.7f+1.3f, 1.0f);
    return *state*2.0f-1.0f;
}

float MIDI_NoteToFreq(int note)
{
    return 440.0f*powf(2, (note-69)/12.0f);
}

void MIDI_NoteOn(int channel, int note, int velocity)
{
    for(int i=0;i<MAX_VOICES;i++)
    {
        if(!voices[i].active)
        {
            voices[i].active=true;
            voices[i].released=false;
            voices[i].channel=channel;
            voices[i].note=note;
            voices[i].velocity=velocity/127.0f;
            voices[i].envelopePhase=0;
            voices[i].envelopeValue=0;
            voices[i].noiseState=(float)(note+velocity)/256.0f;

            if(channel==9)
            {
                // Drums
                voices[i].freq=100.0f;
                voices[i].freqEnvelope=1.0f;
                voices[i].phase=0;
            }
            else
            {
                // Everything else
                voices[i].freq=MIDI_NoteToFreq(note);
                voices[i].phase=0;
                voices[i].modPhase=0;
            }
            break;
        }
    }
}

void MIDI_NoteOff(int channel, int note)
{
    for(int i=0;i<MAX_VOICES;i++)
    {
        if(voices[i].active&&voices[i].note==note&&voices[i].channel==channel&&!voices[i].released)
        {
            voices[i].released=true;
            voices[i].envelopePhase=0;
        }
    }
}

double globalTime=0.0;

void mix_audio(void *userdata, Uint8 *stream, int len)
{
    float *buffer=(float *)stream;
    int samples=len/sizeof(float);
    memset(buffer, 0, len);

    float modulation_index=2.0f;

    for(int i=0;i<samples;i++)
    {
        float sample=0.0f;

        for(int v=0;v<MAX_VOICES;v++)
        {
            if(!voices[v].active)
                continue;

            MIDI_Voice_t *voice=&voices[v];
            float dt=1.0f/SAMPLE_RATE;
            MIDI_Envelope_t env=envelopes[voice->channel];

            // Envelope update
            if(!voice->released)
            {
                if(voice->envelopePhase<env.attack)
                    voice->envelopeValue=voice->envelopePhase/env.attack;
                else if(voice->envelopePhase<env.attack+env.decay)
                    voice->envelopeValue=1.0f-(1.0f-env.sustain)*((voice->envelopePhase-env.attack)/env.decay);
                else
                    voice->envelopeValue=env.sustain;

                voice->envelopePhase+=dt;
            }
            else
            {
                voice->envelopeValue-=dt/env.release;

                if(voice->envelopeValue<=0.0f)
                {
                    voice->active=false;
                    continue;
                }
            }

            float amp=voice->velocity*voice->envelopeValue*0.3f;
            float tone=0.0f;

            // Drums
            if(voice->channel==9)
            {
                switch(voice->note)
                {
                    // Kick
                    case 35:
                    case 36:
                    {
                        voice->phase+=2.0f*PI*(voice->freq*(1.0f-voice->envelopeValue*0.9f))*dt;
                        tone=sinf(voice->phase)*voice->envelopeValue;
                        break;
                    }

                    // Snare
                    case 38:
                    default:
                    {
                        tone=RandomNoise(&voice->noiseState)*voice->envelopeValue;
                        break;
                    }

                    // Hi-hat
                    case 42:
                    case 44:
                    {
                        tone=RandomNoise(&voice->noiseState)*powf(voice->envelopeValue, 3.0f);
                        break;
                    }
                }
            }
            // Everything else
            else
            {
                float mod=sinf(voice->modPhase);
                tone=sinf(voice->phase+modulation_index*mod);
                voice->phase+=2*PI*voice->freq/SAMPLE_RATE;
                voice->modPhase+=2*PI*voice->freq*2.0f/SAMPLE_RATE;
            }

            sample+=tone*amp;
        }

        buffer[i]=sample;
        globalTime+=1.0/SAMPLE_RATE;
    }
}

uint32_t ReadBIG32(FILE *f)
{
    uint8_t b[4];

    fread(b, 1, 4, f);

    return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3];
}

uint16_t ReadBIG16(FILE *f)
{
    uint8_t b[2];

    fread(b, 1, 2, f);

    return (b[0]<<8)|b[1];
}

uint32_t ReadVariableLength(FILE *f)
{
    uint32_t val=0;
    uint8_t byte;

    do
    {
        fread(&byte, 1, 1, f);
        val=(val<<7)|(byte&0x7F);
    } while(byte&0x80);

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
    size_t count;
    size_t capacity;
} MIDI_EventList_t;

void MIDI_AddEvent(MIDI_EventList_t *list, MIDI_Event_t evt)
{
    if(list->count>=list->capacity)
    {
        list->capacity=list->capacity*2+64;
        list->events=realloc(list->events, list->capacity*sizeof(MIDI_Event_t));
    }
    list->events[list->count++]=evt;
}

static int compare_events(const void *a, const void *b)
{
    return ((MIDI_Event_t *)a)->time-((MIDI_Event_t *)b)->time;
}

void MIDI_Parse(const char *filename, MIDI_EventList_t *list, int *pulsePerQuarterNoteOut, float *tempoOut)
{
    FILE *f=fopen(filename, "rb");

    if(!f)
    {
        perror("open");
        exit(1);
    }

    char header[4];
    fread(header, 1, 4, f);

    if(memcmp(header, "MThd", 4))
    {
        printf("Bad MIDI header\n");
        exit(1);
    }

    ReadBIG32(f); // skip header size
    uint16_t format=ReadBIG16(f); // unused
    uint16_t numTracks=ReadBIG16(f);
    uint16_t pulsePerQuarterNote=ReadBIG16(f);
    *pulsePerQuarterNoteOut=pulsePerQuarterNote;
    *tempoOut=120.0f;

    for(int t=0;t<numTracks;t++)
    {
        fread(header, 1, 4, f);

        if(memcmp(header, "MTrk", 4))
        {
            printf("Bad track header\n");
            exit(1);
        }

        uint32_t len=ReadBIG32(f);
        uint32_t start=ftell(f);
        uint32_t time=0;
        uint8_t status=0;

        while(ftell(f)<start+len)
        {
            uint32_t delta=ReadVariableLength(f);
            time+=delta;

            uint8_t peek;
            fread(&peek, 1, 1, f);

            if(peek&0x80)
                status=peek;
            else
                fseek(f, -1, SEEK_CUR);

            uint8_t type=status&0xF0;
            uint8_t channel=status&0x0F;

            if(type==0x80||type==0x90)
            {
                uint8_t b1, b2;
                fread(&b1, 1, 1, f);
                fread(&b2, 1, 1, f);

                MIDI_AddEvent(list, (MIDI_Event_t) { time, status, b1, b2, channel });
            }
            else if(status==0xFF)
            {
                uint8_t metaType;
                fread(&metaType, 1, 1, f);

                uint32_t metaLength=ReadVariableLength(f);

                if(metaType==0x51&&metaLength==3)
                {
                    uint8_t tempo_data[3];
                    fread(tempo_data, 1, 3, f);
                    int us_per_quarter=(tempo_data[0]<<16)|(tempo_data[1]<<8)|tempo_data[2];
                    *tempoOut=60000000.0f/us_per_quarter;
                }
                else
                    fseek(f, metaLength, SEEK_CUR);
            }
            else
            {
                fread(&peek, 1, 1, f);

                if(type!=0xC0&&type!=0xD0)
                    fread(&peek, 1, 1, f);
            }
        }
    }

    qsort(list->events, list->count, sizeof(MIDI_Event_t), compare_events);
    fclose(f);
}

int main(int argc, char **argv)
{
    if(argc<2)
    {
        printf("Usage: %s file.mid\n", argv[0]);
        return 1;
    }

    MIDI_EventList_t list={ 0 };
    int pulsePerQuarterNote;
    float tempo;
    MIDI_Parse(argv[1], &list, &pulsePerQuarterNote, &tempo);

    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec=
    {
        .freq=SAMPLE_RATE,
        .format=AUDIO_F32SYS,
        .channels=1,
        .samples=512,
        .callback=mix_audio
    };
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);

    for(int i=0;i<16;i++)
        envelopes[i]=(MIDI_Envelope_t){ 0.01f, 0.1f, 0.6f, 0.2f };

    double secondsPerTick=60.0/(tempo*pulsePerQuarterNote);
    size_t position=0;

    while(position<list.count)
    {
        double now=globalTime;
        uint32_t currentTick=now/secondsPerTick;

        while(position<list.count&&list.events[position].time<=currentTick)
        {
            MIDI_Event_t *e=&list.events[position++];
            if((e->status&0xF0)==0x90&&e->data2>0)
                MIDI_NoteOn(e->channel, e->data1, e->data2);
            else if((e->status&0xF0)==0x80||((e->status&0xF0)==0x90&&e->data2==0))
                MIDI_NoteOff(e->channel, e->data1);
        }

        SDL_Delay(1);
    }

    SDL_Delay(3000); // Let tail play out
    SDL_Quit();
    return 0;
}
