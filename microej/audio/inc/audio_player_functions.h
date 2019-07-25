/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "sni.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define AUDIO_PLAYER_VOLUME_MIN_GAIN -102
#define AUDIO_PLAYER_VOLUME_MAX_GAIN 12

typedef enum AudioPlayerStatus
{
  AUDIO_PLAYER_NULL,
  AUDIO_PLAYER_IDLE,
  AUDIO_PLAYER_PLAYING,
} AudioPlayerStatus;

typedef enum AudioPlayerJavaCommand
{
  AUDIO_PLAYER_PLAY_FILE,
  AUDIO_PLAYER_CLOSE,
} AudioPlayerJavaCommand;

struct sni_context_s
{
  void *args;
  size_t size;
  volatile AudioPlayerJavaCommand order_id;
  volatile int java_thread_id;
  volatile bool result;
};

struct audio_file_info_s
{
  char filename[64];
  uint8_t channel_number;
  uint8_t bit_length;
  uint8_t codec;
  uint32_t sampling_rate;
  int32_t duration;
};

/****************************************************************************/
/* Error handling functions */
/****************************************************************************/
void errout_start(void);
void error_leave_player(void);
void errout_power_on();
void errout_open_contents_dir();
void errout_act_audio_sub_system();

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
bool audio_player_open_directory();
bool audio_player_set_power_on();
bool audio_player_handle_clk_mode(int cur_clk_mode);

/* Call  the initialization sequence */
bool audio_player_initialize();
/* Play the currently loaded song */
bool audio_player_play(int duration);
bool audio_player_open_playlist();
/* Load and play next song in playlist */
bool audio_player_play_next(int duration);
/* Load and play next song in playlist */
bool audio_player_play_prev(int duration);
/* Load and play a given song */
bool audio_player_play_file(const char *filename, uint8_t audio_channel, uint8_t bit_length, uint32_t sampling_rate, uint8_t codec_type, int duration);
/* Pause the currently playing song */
bool audio_player_pause();
bool audio_player_set_volume(int master_db);
void audio_player_app_set_status_null();

#endif /* AUDIO_PLAYER_H */