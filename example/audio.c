#include "audio.h"
#include "spotify.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <alsa/asoundlib.h>

#define PB_DEVICE ("plughw:0,0")

static snd_pcm_t *playback_handle;

int audio_init(void) {
    int err;
    char *pdevice = PB_DEVICE;
    snd_pcm_hw_params_t *hw_params;
    unsigned int sampleFrq = 44100;
    int dir = 0;
/*
    int periods = 8;
    snd_pcm_uframes_t periodsize = 1024;
*/

    if ((err = snd_pcm_open(&playback_handle, pdevice, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("cannot open audio device %s (%s)\n", pdevice, snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
        printf("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("cannot set access type (%s)\n", snd_strerror (err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        printf("cannot set sample format (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &sampleFrq, &dir)) < 0) {
        printf("cannot set sample rate (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2)) < 0) {
        printf("cannot set channel count (%s)\n", snd_strerror(err));
        return -1;
    }

/*
    if (snd_pcm_hw_params_set_periods(playback_handle, hw_params, periods, 0) < 0) {
      printf("Error setting periods.\n");
      return -1;
    }

    if (snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, (periodsize * periods)>>2) < 0) {
      printf("Error setting buffersize.\n");
      return -1;
    }
*/

   if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        printf("cannot set parameters (%s)\n", snd_strerror(err));
        return -1;
    }

    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(playback_handle)) < 0) {
        printf("cannot prepare audio interface for use (%s)\n", snd_strerror(err));
        return -1;
    }

    return 0;
}

void audio_frame(const void *frames, uint32_t num_frames, sp_audioformat *format) {
    int err;

    if (format->sample_type != SP_SAMPLETYPE_INT16_NATIVE_ENDIAN
            || format->sample_rate != 44100
            || format->channels != 2) {
        printf("Wrong audio format: %d %d %d\n", format->sample_type,
                format->sample_rate, format->channels);
    } else {
        printf("num_frames: %d\n", num_frames);
        if ((err = snd_pcm_writei(playback_handle, frames, num_frames)) != num_frames) {
            printf("write to audio interface failed (%s)\n", snd_strerror(err));
        }
    }
}

void audio_flush(void) {
}

void audio_volume(uint16_t volume) {
}

void audio_close(void) {
    snd_pcm_close(playback_handle);
}

