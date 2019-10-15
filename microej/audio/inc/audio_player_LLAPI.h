/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 *  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#ifndef AUDIO_PLAYER_API_H
#define AUDIO_PLAYER_API_H

#include "sni.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define Java_audio_player_initialize Java_ej_audio_AudioPlayerNative_initialize
#define Java_audio_player_pause Java_ej_audio_AudioPlayerNative_pause
#define Java_audio_player_play_file Java_ej_audio_AudioPlayerNative_play
#define Java_audio_player_fetch_result Java_ej_audio_AudioPlayerNative_fetchResult
#define Java_audio_player_set_volume Java_ej_audio_AudioPlayerNative_setGain
#define Java_audio_player_mute Java_ej_audio_AudioPlayerNative_mute

    /* Looping function executing command */
    void *Java_calling_thread_function_impl(void *args);

    /* Java commands */
    void Java_audio_player_initialize(void);
    void Java_audio_player_close(void);
    void Java_audio_player_pause(void);

    void Java_audio_player_play_file(uint8_t *filename, size_t filename_length,
                                     uint8_t audio_channel,
                                     uint8_t bit_length,
                                     uint32_t sampling_rate,
                                     uint8_t codec_type,
                                     int duration);

    void Java_audio_player_set_volume(float volume_db);
    void Java_audio_player_mute(void);

    /* Function throwing exception if the last command failed */
    void Java_audio_player_fetch_result(void);

    /* Implementation of Java commands in C */
    /* The reason for having a 2 level layer functions is to distinguish between the Java mechanism with throwing exception and the C function calls returning a status */
    bool Call_audio_player_play_file(struct audio_file_info_s *track);
    bool Call_audio_player_pause();
    bool Call_audio_player_set_volume(int volumeDB);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_PLAYER_API_H */
