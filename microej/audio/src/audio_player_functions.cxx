/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This file has been modified by MicroEJ Corp.
 */

/****************************************************************************
 * audio_player/audio_player_main.cxx
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <asmp/mpshm.h>
#include <arch/chip/pm.h>
#include <arch/board/board.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
#include "playlist/playlist.h"
#endif

#include "inc/audio_player_functions.h"
#include "inc/msgq_id.h"
#include "inc/mem_layout.h"
#include "inc/memory_layout.h"
#include "inc/msgq_pool.h"
#include "inc/pool_layout.h"
#include "inc/fixed_fence.h"
#include "audio/audio_high_level_api.h"
#include "audio/audio_player_api.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/memory_manager/MemHandle.h"

using namespace MemMgrLite;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/*------------------------------
 * User definable definitions
 *------------------------------
 */

/* State machine of audio player */
volatile AudioPlayerStatus audio_player_status = AUDIO_PLAYER_NULL;

// #define AUDIO_PLAYER_DEBUG_TRACE

#ifdef AUDIO_PLAYER_DEBUG_TRACE
#define AUDIO_PLAYER_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define AUDIO_PLAYER_DEBUG_PRINTF(...) ((void)0)
#endif

/* Path of playback file. */

#define PLAYBACK_FILE_PATH "/mnt/sd0"

/* Path of DSP image file. */

#define DSPBIN_FILE_PATH "/mnt/sd0/BIN"

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
/* Path of playlist file. */

#define PLAYLIST_FILE_PATH "/mnt/sd0/PLAYLIST"

/* PlayList file name. */

#define PLAYLIST_FILE_NAME "TRACK_DB.CSV"
#endif

/* Default Volume. -20dB */

#define PLAYER_DEF_VOLUME -200

/* Play time(sec) */

#define PLAYER_PLAY_TIME 10

/* Play file number */

#define PLAYER_PLAY_FILE_NUM 5

/* Definition of FIFO info.
 * The FIFO defined here refers to the buffer to pass playback data
 * between application and AudioSubSystem.
 */

/* Recommended frame size differs for each codec.
 *    MP3       : 1440 bytes
 *    AAC       : 1024 bytes
 *    WAV 16bit : 2560 bytes
 *    WAV 24bit : 3840 bytes
 * If the codec to be played is fixed, use this size.
 * When playing multiple codecs, select the largest size.
 * There is no problem increasing the size. If it is made smaller,
 * FIFO under is possibly generated, so it is necessary to be careful.
 */

#define FIFO_FRAME_SIZE 3840

/* The number of elements in the FIFO queue used varies depending on
 * the performance of the system and must be sufficient.
 */

#define FIFO_ELEMENT_NUM 10

/* The number of elements pushed to the FIFO Queue at a time depends
 * on the load of the system. If the number of elements is small,
 * we will underflow if application dispatching is delayed.
 * Define a sufficient maximum value.
 */

#define PLAYER_FIFO_PUSH_NUM_MAX 5

/*------------------------------
 * Definition specified by config
 *------------------------------
 */

/* Definition for selection of output device.
 * Select speaker output or I2S output.
 */
#define CONFIG_EXAMPLES_AUDIO_PLAYER_OUTPUT_DEV_SPHP 1
#define CONFIG_EXAMPLES_AUDIO_PLAYER_MODE_NORMAL 1

#ifdef CONFIG_EXAMPLES_AUDIO_PLAYER_OUTPUT_DEV_SPHP
#define PLAYER_OUTPUT_DEV AS_SETPLAYER_OUTPUTDEVICE_SPHP
#define PLAYER_MIXER_OUT AS_OUT_SP
#else
#define PLAYER_OUTPUT_DEV AS_SETPLAYER_OUTPUTDEVICE_I2SOUTPUT
#define PLAYER_MIXER_OUT AS_OUT_I2S
#endif

/* Definition depending on player mode. */

#ifdef CONFIG_EXAMPLES_AUDIO_PLAYER_MODE_HIRES
#define FIFO_FRAME_NUM 4
#else
#define FIFO_FRAME_NUM 1
#endif

/*------------------------------
 * Definition of example fixed
 *------------------------------
 */

/* FIFO control value. */

#define FIFO_ELEMENT_SIZE (FIFO_FRAME_SIZE * FIFO_FRAME_NUM)
#define FIFO_QUEUE_SIZE (FIFO_ELEMENT_SIZE * FIFO_ELEMENT_NUM)

/* Local error code. */

#define FIFO_RESULT_OK 0
#define FIFO_RESULT_ERR 1
#define FIFO_RESULT_EOF 2
#define FIFO_RESULT_FUL 3

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* For FIFO */

struct player_fifo_info_s
{
  CMN_SimpleFifoHandle handle;
  AsPlayerInputDeviceHdlrForRAM input_device;
  uint32_t fifo_area[FIFO_QUEUE_SIZE / sizeof(uint32_t)];
  uint8_t read_buf[FIFO_ELEMENT_SIZE];
};

#ifndef CONFIG_AUDIOUTILS_PLAYLIST
struct Track
{
  char title[64];
  uint8_t channel_number; /* Channel number. */
  uint8_t bit_length;     /* Bit length.     */
  uint32_t sampling_rate; /* Sampling rate.  */
  uint8_t codec_type;     /* Codec type.     */
};
#endif

/* For play file */

struct player_file_info_s
{
  Track track;
  int32_t size;
  DIR *dirp;
  int fd;
};

/* Player info */

struct player_info_s
{
  struct player_fifo_info_s fifo;
  struct player_file_info_s file;
#ifdef CONFIG_AUDIOUTILS_PLAYLIST
  Playlist *playlist_ins = NULL;
#endif
};

/****************************************************************************
 * Private Data
 ****************************************************************************/
/* For control player */

static struct player_info_s s_player_info;

/* For share memory. */

static mpshm_t s_shm;

/* For frequency lock. */

static struct pm_cpu_freqlock_s s_player_lock;

/****************************************************************************/
/* Private functions prototype */
/****************************************************************************/
static void app_init_freq_lock(void);
static void app_freq_lock(void);
static void app_freq_release(void);
static bool app_open_contents_dir(void);
static bool app_close_contents_dir(void);
static bool app_open_playlist(void);
static bool app_close_playlist(void);
static bool app_get_next_track(Track *track);
static bool app_get_prev_track(Track *track);

static void app_input_device_callback(uint32_t size);
static bool app_init_simple_fifo(void);
static int app_push_simple_fifo(int fd);
static bool app_first_push_simple_fifo(int fd);
static bool app_refill_simple_fifo(int fd);
static bool printAudCmdResult(uint8_t command_code, AudioResult &result);
static void app_attention_callback(const ErrorAttentionParam *attparam);
static bool app_create_audio_sub_system(void);
static void app_deact_audio_sub_system(void);
static bool app_power_on(void);
static bool app_power_off(void);
static bool app_set_ready(void);
static bool app_get_status(void);
static bool app_init_output_select(void);
static bool app_set_volume(int master_db);
static bool app_set_player_status(void);
static int app_init_player(uint8_t codec_type,
                           uint32_t sampling_rate,
                           uint8_t channel_number,
                           uint8_t bit_length);
static int app_play_player(void);
static bool app_stop_player(int mode);
static bool app_set_clkmode(int clk_mode);
static bool app_init_libraries(void);
static bool app_finalize_libraries(void);

static int app_play_file_open(FAR const char *file_path, FAR int32_t *file_size);
static bool app_open_file(const char *filename, uint8_t audio_channel, uint8_t bit_len, uint32_t sampling_rate, uint8_t codec_type);
static bool app_get_track(Track *track, const char *filename, uint8_t audio_channel, uint8_t bit_len, uint32_t sampling_rate, uint8_t codec_type);
static bool app_play_curr_file(void);
static bool app_open_next_file(void);
static bool app_open_prev_file(void);

static bool app_close_play_file(void);
static bool app_start(void);
static bool app_stop(void);
void app_play_process(uint32_t play_time);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void app_init_freq_lock(void)
{
  s_player_lock.count = 0;
  s_player_lock.info = PM_CPUFREQLOCK_TAG('A', 'P', 0);
  s_player_lock.flag = PM_CPUFREQLOCK_FLAG_HV;
}

static void app_freq_lock(void)
{
  up_pm_acquire_freqlock(&s_player_lock);
}

static void app_freq_release(void)
{
  up_pm_release_freqlock(&s_player_lock);
}

static bool app_open_contents_dir(void)
{
  DIR *dirp;
  const char *name = PLAYBACK_FILE_PATH;

  dirp = opendir(name);

  if (!dirp)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: %s directory path error. check the path!\n", name);
    return false;
  }

  s_player_info.file.dirp = dirp;

  return true;
}

static bool app_close_contents_dir(void)
{
  closedir(s_player_info.file.dirp);

  return true;
}

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
static bool app_open_playlist(void)
{
  bool result = false;

  if (s_player_info.playlist_ins != NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Open playlist failure. Playlist is already open\n");
    return false;
  }

  s_player_info.playlist_ins = new Playlist(PLAYLIST_FILE_NAME);

  result = s_player_info.playlist_ins->init(PLAYLIST_FILE_PATH);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Playlist::init() failure.\n");
    return false;
  }

  s_player_info.playlist_ins->setPlayMode(Playlist::PlayModeNormal);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Playlist::setPlayMode() failure.\n");
    return false;
  }

  s_player_info.playlist_ins->setRepeatMode(Playlist::RepeatModeOn);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Playlist::setRepeatMode() failure.\n");
    return false;
  }

  s_player_info.playlist_ins->select(Playlist::ListTypeAllTrack, NULL);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Playlist::select() failure.\n");
    return false;
  }

  return true;
}

static bool app_close_playlist(void)
{
  if (s_player_info.playlist_ins == NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Close playlist failure. Playlist is not open\n");
    return false;
  }

  delete s_player_info.playlist_ins;
  s_player_info.playlist_ins = NULL;

  return true;
}

static bool app_get_prev_track(Track *track)
{
  bool ret = true;

  if (s_player_info.playlist_ins == NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Get prev track failure. Playlist is not open\n");
    return false;
  }
  ret = s_player_info.playlist_ins->getPrevTrack(track);
  return ret;
}

static bool app_get_next_track(Track *track)
{
  bool ret = true;

  if (s_player_info.playlist_ins == NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Get next track failure. Playlist is not open\n");
    return false;
  }
  ret = s_player_info.playlist_ins->getNextTrack(track);
  return ret;
}

#endif /* #ifdef CONFIG_AUDIOUTILS_PLAYLIST */

static void app_input_device_callback(uint32_t size)
{
  /* do nothing */
}

static bool app_init_simple_fifo(void)
{
  if (CMN_SimpleFifoInitialize(&s_player_info.fifo.handle,
                               s_player_info.fifo.fifo_area,
                               FIFO_QUEUE_SIZE,
                               NULL) != 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Fail to initialize simple FIFO.");
    return false;
  }
  CMN_SimpleFifoClear(&s_player_info.fifo.handle);

  s_player_info.fifo.input_device.simple_fifo_handler = (void *)(&s_player_info.fifo.handle);
  s_player_info.fifo.input_device.callback_function = app_input_device_callback;

  return true;
}

static int app_push_simple_fifo(int fd)
{
  int ret;

  ret = read(fd, &s_player_info.fifo.read_buf, FIFO_ELEMENT_SIZE);
  if (ret < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Fail to read file. errno:%d\n", get_errno());
    return FIFO_RESULT_ERR;
  }

  if (CMN_SimpleFifoOffer(&s_player_info.fifo.handle, (const void *)(s_player_info.fifo.read_buf), ret) == 0)
  {
    return FIFO_RESULT_FUL;
  }
  s_player_info.file.size = (s_player_info.file.size - ret);
  if (s_player_info.file.size == 0)
  {
    return FIFO_RESULT_EOF;
  }
  return FIFO_RESULT_OK;
}

static bool app_first_push_simple_fifo(int fd)
{
  int i;
  int ret = 0;

  for (i = 0; i < FIFO_ELEMENT_NUM - 1; i++)
  {
    if ((ret = app_push_simple_fifo(fd)) != FIFO_RESULT_OK)
    {
      break;
    }
  }

  return (ret != FIFO_RESULT_ERR) ? true : false;
}

static bool app_refill_simple_fifo(int fd)
{
  int32_t ret = FIFO_RESULT_OK;
  size_t vacant_size;

  vacant_size = CMN_SimpleFifoGetVacantSize(&s_player_info.fifo.handle);

  if ((vacant_size != 0) && (vacant_size > FIFO_ELEMENT_SIZE))
  {
    int push_cnt = vacant_size / FIFO_ELEMENT_SIZE;

    push_cnt = (push_cnt >= PLAYER_FIFO_PUSH_NUM_MAX) ? PLAYER_FIFO_PUSH_NUM_MAX : push_cnt;

    for (int i = 0; i < push_cnt; i++)
    {
      if ((ret = app_push_simple_fifo(fd)) != FIFO_RESULT_OK)
      {
        break;
      }
    }
  }

  return (ret == FIFO_RESULT_OK) ? true : false;
}

static bool printAudCmdResult(uint8_t command_code, AudioResult &result)
{
  if (AUDRLT_ERRORRESPONSE == result.header.result_code)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Command code(0x%x): AUDRLT_ERRORRESPONSE:"
                              "Module id(0x%x): Error code(0x%x)\n",
                              command_code,
                              result.error_response_param.module_id,
                              result.error_response_param.error_code);
    return false;
  }
  else if (AUDRLT_ERRORATTENTION == result.header.result_code)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Command code(0x%x): AUDRLT_ERRORATTENTION\n", command_code);
    return false;
  }
  return true;
}

static void app_attention_callback(const ErrorAttentionParam *attparam)
{
  AUDIO_PLAYER_DEBUG_PRINTF("Attention!! %s L%d ecode %d subcode %d\n",
                            attparam->error_filename,
                            attparam->line_number,
                            attparam->error_code,
                            attparam->error_att_sub_code);
}

static bool app_create_audio_sub_system(void)
{
  bool result = false;

  /* Create manager of AudioSubSystem. */

  AudioSubSystemIDs ids;
  ids.app = MSGQ_AUD_APP;
  ids.mng = MSGQ_AUD_MNG;
  ids.player_main = MSGQ_AUD_PLY;
  ids.player_sub = 0xFF;
  ids.mixer = MSGQ_AUD_OUTPUT_MIX;
  ids.recorder = 0xFF;
  ids.effector = 0xFF;
  ids.recognizer = 0xFF;

  AS_CreateAudioManager(ids, app_attention_callback);

  /* Create player feature. */

  AsCreatePlayerParam_t player_create_param;
  player_create_param.msgq_id.player = MSGQ_AUD_PLY;
  player_create_param.msgq_id.mng = MSGQ_AUD_MNG;
  player_create_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  player_create_param.msgq_id.dsp = MSGQ_AUD_DSP;
  player_create_param.pool_id.es = DEC_ES_MAIN_BUF_POOL;
  player_create_param.pool_id.pcm = REND_PCM_BUF_POOL;
  player_create_param.pool_id.dsp = DEC_APU_CMD_POOL;
  player_create_param.pool_id.src_work = SRC_WORK_BUF_POOL;

  /* When calling AS_CreatePlayerMulti(), use the pool area
   * for multi-core playback processing.
   * When calling AS_CreatePlayer(), use the heap area.
   */

  result = AS_CreatePlayerMulti(AS_PLAYER_ID_0, &player_create_param, NULL);

  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: AS_CratePlayer() failure. system memory insufficient!\n");
    return false;
  }

  /* Create mixer feature. */

  AsCreateOutputMixParam_t output_mix_act_param;
  output_mix_act_param.msgq_id.mixer = MSGQ_AUD_OUTPUT_MIX;
  output_mix_act_param.msgq_id.render_path0_filter_dsp = MSGQ_AUD_PFDSP0;
  output_mix_act_param.msgq_id.render_path1_filter_dsp = MSGQ_AUD_PFDSP1;
  output_mix_act_param.pool_id.render_path0_filter_pcm = PF0_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path1_filter_pcm = PF1_PCM_BUF_POOL;
  output_mix_act_param.pool_id.render_path0_filter_dsp = PF0_APU_CMD_POOL;
  output_mix_act_param.pool_id.render_path1_filter_dsp = PF1_APU_CMD_POOL;

  result = AS_CreateOutputMixer(&output_mix_act_param, NULL);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: AS_CreateOutputMixer() failed. system memory insufficient!\n");
    return false;
  }

  /* Create renderer feature. */

  AsCreateRendererParam_t renderer_create_param;
  renderer_create_param.msgq_id.dev0_req = MSGQ_AUD_RND_PLY;
  renderer_create_param.msgq_id.dev0_sync = MSGQ_AUD_RND_PLY_SYNC;
  renderer_create_param.msgq_id.dev1_req = 0xFF;
  renderer_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateRenderer(&renderer_create_param);
  if (!result)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: AS_CreateRenderer() failure. system memory insufficient!\n");
    return false;
  }

  return true;
}

static void app_deact_audio_sub_system(void)
{
  AS_DeleteAudioManager();
  AS_DeletePlayer(AS_PLAYER_ID_0);
  AS_DeleteOutputMix();
  AS_DeleteRenderer();
}

static bool app_power_on(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_POWERON;
  command.header.command_code = AUDCMD_POWERON;
  command.header.sub_code = 0x00;
  command.power_on_param.enable_sound_effect = AS_DISABLE_SOUNDEFFECT;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_power_off(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_POWEROFF_STATUS;
  command.header.command_code = AUDCMD_SETPOWEROFFSTATUS;
  command.header.sub_code = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_ready(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_READY_STATUS;
  command.header.command_code = AUDCMD_SETREADYSTATUS;
  command.header.sub_code = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_get_status(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_GETSTATUS;
  command.header.command_code = AUDCMD_GETSTATUS;
  command.header.sub_code = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return result.notify_status.status_info;
}

static bool app_init_output_select(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INITOUTPUTSELECT;
  command.header.command_code = AUDCMD_INITOUTPUTSELECT;
  command.header.sub_code = 0;
  command.init_output_select_param.output_device_sel = PLAYER_MIXER_OUT;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_volume(int master_db)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SETVOLUME;
  command.header.command_code = AUDCMD_SETVOLUME;
  command.header.sub_code = 0;
  command.set_volume_param.input1_db = 0;
  command.set_volume_param.input2_db = 0;
  command.set_volume_param.master_db = master_db;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_player_status(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SET_PLAYER_STATUS;
  command.header.command_code = AUDCMD_SETPLAYERSTATUS;
  command.header.sub_code = 0x00;
  command.set_player_sts_param.active_player = AS_ACTPLAYER_MAIN;
  command.set_player_sts_param.player0.input_device = AS_SETPLAYER_INPUTDEVICE_RAM;
  command.set_player_sts_param.player0.ram_handler = &s_player_info.fifo.input_device;
  command.set_player_sts_param.player0.output_device = PLAYER_OUTPUT_DEV;
  command.set_player_sts_param.player1.input_device = 0x00;
  command.set_player_sts_param.player1.ram_handler = NULL;
  command.set_player_sts_param.player1.output_device = 0x00;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static int app_init_player(uint8_t codec_type,
                           uint32_t sampling_rate,
                           uint8_t channel_number,
                           uint8_t bit_length)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_INIT_PLAYER;
  command.header.command_code = AUDCMD_INITPLAYER;
  command.header.sub_code = 0x00;
  command.player.player_id = AS_PLAYER_ID_0;
  command.player.init_param.codec_type = codec_type;
  command.player.init_param.bit_length = bit_length;
  command.player.init_param.channel_number = channel_number;
  command.player.init_param.sampling_rate = sampling_rate;
  snprintf(command.player.init_param.dsp_path,
           AS_AUDIO_DSP_PATH_LEN,
           "%s",
           DSPBIN_FILE_PATH);
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static int app_play_player(void)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_PLAY_PLAYER;
  command.header.command_code = AUDCMD_PLAYPLAYER;
  command.header.sub_code = 0x00;
  command.player.player_id = AS_PLAYER_ID_0;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_stop_player(int mode)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_STOP_PLAYER;
  command.header.command_code = AUDCMD_STOPPLAYER;
  command.header.sub_code = 0x00;
  command.player.player_id = AS_PLAYER_ID_0;
  command.player.stop_param.stop_mode = mode;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_set_clkmode(int clk_mode)
{
  AudioCommand command;
  command.header.packet_length = LENGTH_SETRENDERINGCLK;
  command.header.command_code = AUDCMD_SETRENDERINGCLK;
  command.header.sub_code = 0x00;
  command.set_renderingclk_param.clk_mode = clk_mode;
  AS_SendAudioCommand(&command);

  AudioResult result;
  AS_ReceiveAudioResult(&result);
  return printAudCmdResult(command.header.command_code, result);
}

static bool app_init_libraries(void)
{
  int ret;
  uint32_t addr = AUD_SRAM_ADDR;

  /* Initialize shared memory.*/

  ret = mpshm_init(&s_shm, 1, 1024 * 128 * 2);
  if (ret < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: mpshm_init() failure. %d\n", ret);
    return false;
  }

  ret = mpshm_remap(&s_shm, (void *)addr);
  if (ret < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: mpshm_remap() failure. %d\n", ret);
    return false;
  }

  /* Initalize MessageLib. */

  err_t err = MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  if (err != ERR_OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: MsgLib::initFirst() failure. 0x%x\n", err);
    return false;
  }

  err = MsgLib::initPerCpu();
  if (err != ERR_OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: MsgLib::initPerCpu() failure. 0x%x\n", err);
    return false;
  }

  void *mml_data_area = translatePoolAddrToVa(MEMMGR_DATA_AREA_ADDR);
  err = Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  if (err != ERR_OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Manager::initFirst() failure. 0x%x\n", err);
    return false;
  }

  err = Manager::initPerCpu(mml_data_area, NUM_MEM_POOLS);
  if (err != ERR_OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Manager::initPerCpu() failure. 0x%x\n", err);
    return false;
  }

  /* Create static memory pool of VoiceCall. */

  const NumLayout layout_no = MEM_LAYOUT_PLAYER_MAIN_ONLY;
  void *work_va = translatePoolAddrToVa(MEMMGR_WORK_AREA_ADDR);
  err = Manager::createStaticPools(layout_no,
                                   work_va,
                                   MEMMGR_MAX_WORK_SIZE,
                                   MemoryPoolLayouts[layout_no]);
  if (err != ERR_OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: Manager::initPerCpu() failure. %d\n", err);
    return false;
  }

  return true;
}

static bool app_finalize_libraries(void)
{
  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* Destroy static pools. */

  MemMgrLite::Manager::destroyStaticPools();

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */

  int ret;
  ret = mpshm_detach(&s_shm);
  if (ret < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: mpshm_detach() failure. %d\n", ret);
    return false;
  }

  ret = mpshm_destroy(&s_shm);
  if (ret < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: mpshm_destroy() failure. %d\n", ret);
    return false;
  }

  return true;
}

static int app_play_file_open(FAR const char *file_path, FAR int32_t *file_size)
{
  int fd = open(file_path, O_RDONLY);

  *file_size = 0;
  if (fd >= 0)
  {
    struct stat stat_buf;
    if (stat(file_path, &stat_buf) == OK)
    {
      *file_size = stat_buf.st_size;
    }
  }

  return fd;
}

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
static bool app_open_prev_file(void)
{
  /* Get next track */
  if (!app_get_prev_track(&s_player_info.file.track))
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: No more tracks to play.\n");
    return false;
  }
  return true;
}

static bool app_open_next_file(void)
{
  /* Get next track */
  if (!app_get_next_track(&s_player_info.file.track))
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: No more tracks to play.\n");
    return false;
  }
  return true;
}
#endif

static bool app_play_curr_file(void)
{
  char full_path[128];
  snprintf(full_path,
           sizeof(full_path),
           "%s/%s",
           PLAYBACK_FILE_PATH,
           s_player_info.file.track.title);

  s_player_info.file.fd = app_play_file_open(full_path, &s_player_info.file.size);
  if (s_player_info.file.fd < 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: %s open error. check paths and files!\n", full_path);
    return false;
  }
  if (s_player_info.file.size == 0)
  {
    close(s_player_info.file.fd);
    AUDIO_PLAYER_DEBUG_PRINTF("Error: %s file size is abnormal. check files!\n", full_path);
    return false;
  }

  /* Push data to simple fifo */

  if (!app_first_push_simple_fifo(s_player_info.file.fd))
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_first_push_simple_fifo() failure.\n");
    CMN_SimpleFifoClear(&s_player_info.fifo.handle);
    close(s_player_info.file.fd);
    return false;
  }
  return true;
}

static bool app_get_track(Track *track, const char *filename, uint8_t audio_channel, uint8_t bit_len, uint32_t sampling_rate, uint8_t codec_type)
{
  bool ret = true;
  snprintf(track->title, sizeof(track->title), "%s", filename);
  track->channel_number = audio_channel;
  track->bit_length = bit_len;
  track->sampling_rate = sampling_rate;
  track->codec_type = codec_type;
  return ret;
}

static bool app_open_file(const char *filename, uint8_t audio_channel, uint8_t bit_len, uint32_t sampling_rate, uint8_t codec_type)
{
  /* Get next track */
  if (!app_get_track(&s_player_info.file.track, filename, audio_channel, bit_len, sampling_rate, codec_type))
  {
    return false;
  }
  return true;
}

static bool app_close_play_file(void)
{
  if (close(s_player_info.file.fd) != 0)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: close() failure.\n");
    return false;
  }

  CMN_SimpleFifoClear(&s_player_info.fifo.handle);

  return true;
}

static bool app_start(void)
{
  /* Init Player */

  Track *t = &s_player_info.file.track;
  if (!app_init_player(t->codec_type,
                       t->sampling_rate,
                       t->channel_number,
                       t->bit_length))
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_init_player() failure.\n");
    app_close_play_file();
    return false;
  }

  /* Play Player */

  if (!app_play_player())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_play_player() failure.\n");
    app_close_play_file();
    return false;
  }

  return true;
}

static bool app_stop(void)
{
  bool result = true;

  /* Set stop mode.
   * If the end of the file is detected, play back until the data empty
   * and then stop.(select AS_STOPPLAYER_ESEND)
   * Otherwise, stop immediate reproduction.(select AS_STOPPLAYER_NORMAL)
   */

  int stop_mode = (s_player_info.file.size != 0) ? AS_STOPPLAYER_NORMAL : AS_STOPPLAYER_ESEND;

  if (!app_stop_player(stop_mode))
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_stop_player() failure.\n");
    result = false;
  }

  if (!app_close_play_file())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_close_play_file() failure.\n");
    result = false;
  }

  return result;
}

void app_play_process(uint32_t play_time)
{
  audio_player_status = AUDIO_PLAYER_PLAYING;
  /* Timer Start */
  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do
  {
    /* Check the FIFO every 2 ms and fill if there is space. */

    usleep(2 * 1000);
    if (!app_refill_simple_fifo(s_player_info.file.fd))
    {
      break;
    }
    if (audio_player_status != AUDIO_PLAYER_PLAYING)
    {
      break;
    }
  } while ((time(&cur_time) - start_time) < play_time);
}

/****************************************************************************
 * Error Handling Functions
 ****************************************************************************/
void errout_start(void)
{
  if (board_external_amp_mute_control(true) != OK)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: board_external_amp_mute_control(true) failuer.\n");
  }
}

void error_leave_player(void)
{
  if (AS_MNG_STATUS_READY != app_get_status())
  {
    if (!app_set_ready())
    {
      AUDIO_PLAYER_DEBUG_PRINTF("Error: app_set_ready() failure.\n");
      return;
    }
  }

  /* Change AudioSubsystem to PowerOff state. */

  if (!app_power_off())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_power_off() failure.\n");
    return;
  }

  /* Unlock cpu frequency. */

  app_freq_release();

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
  /* Close playlist. */

  if (!app_close_playlist())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_close_playlist() failure.\n");
    return;
  }
#endif
}

void errout_power_on()
{
  if (!app_close_contents_dir())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_close_contents_dir() failure.\n");
  }
}

void errout_open_contents_dir()
{
  app_deact_audio_sub_system();
}

void errout_act_audio_sub_system()
{
  if (!app_finalize_libraries())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: finalize_libraries() failure.\n");
  }
}

/****************************************************************************
 * Public functions
 ****************************************************************************/
bool audio_player_initialize()
{
  if (audio_player_status != AUDIO_PLAYER_NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: init already done \n");
    return false;
  }

  /* First, initialize the shared memory and memory utility used by AudioSubSystem. */
  if (!app_init_libraries())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: init_libraries() failure.\n");
    return false;
  }

  /* Next, Create the features used by AudioSubSystem. */

  if (!app_create_audio_sub_system())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: act_audiosubsystem() failure.\n");

    /* Abnormal termination processing */

    // goto errout_act_audio_sub_system;
    errout_act_audio_sub_system();
    return false;
  }

  /* Open directory of play contents. */
  if (!audio_player_open_directory())
  {
    return false;
  }
  /* Lock CPU frequency to high and power on the audio player app. */
  if (!audio_player_set_power_on())
  {
    return false;
  }

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
  /* Open playlist. */
  if (!audio_player_open_playlist())
  {
    return false;
  }
#endif

  /* Initialize simple fifo. */
  if (!app_init_simple_fifo())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_init_simple_fifo() failure.\n");
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }

  /* Set the device to output the mixed audio. */
  if (!app_init_output_select())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_init_output_select() failure.\n");
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }

  audio_player_status = AUDIO_PLAYER_IDLE;
  return true;
}

bool audio_player_open_directory()
{
  if (!app_open_contents_dir())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_open_contents_dir() failure.\n");

    /* Abnormal termination processing */
    errout_open_contents_dir();
    return false;
  }
  return true;
}

bool audio_player_set_power_on()
{
  /* Initialize frequency lock parameter. */
  app_init_freq_lock();

  /* Lock cpu frequency to high. */
  app_freq_lock();

  /* Change AudioSubsystem to Ready state so that I/O parameters can be changed. */
  if (!app_power_on())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_power_on() failure.\n");

    /* Abnormal termination processing */
    errout_power_on();
    return false;
  }
  return true;
}

bool audio_player_handle_clk_mode(int cur_clk_mode)
{
  static int clk_mode = -1;
  /* If clockmode is Hi-Res and player mode is not Hi-Res,
       * play the next file.
       */

#ifdef CONFIG_EXAMPLES_AUDIO_PLAYER_MODE_NORMAL
  if (cur_clk_mode == AS_CLKMODE_HIRES)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Hi-Res file is not supported.\n"
                              "Please change player mode to Hi-Res with config.\n");
    app_close_play_file();

    /* Play next file. */
    return false;
  }
#endif

  /* If current clock mode is different from the previous clock mode,
       * perform initial setting.
       */
  if (clk_mode != cur_clk_mode)
  {
    /* Update clock mode. */
    clk_mode = cur_clk_mode;

    /* Since the initial setting is required to be in the Ready state,
           * if it is not in the Ready state, it is set to the Ready state.
           */

    if (AS_MNG_STATUS_READY != app_get_status())
    {
      if (board_external_amp_mute_control(true) != OK)
      {
        AUDIO_PLAYER_DEBUG_PRINTF("Error: board_external_amp_mute_control(true) failuer.\n");

        /* Abnormal termination processing */
        error_leave_player();
        return false;
      }

      if (!app_set_ready())
      {
        AUDIO_PLAYER_DEBUG_PRINTF("Error: app_set_ready() failure.\n");

        /* Abnormal termination processing */
        error_leave_player();
        return false;
      }
    }

    /* Set the clock mode of the output function. */
    if (!app_set_clkmode(clk_mode))
    {
      AUDIO_PLAYER_DEBUG_PRINTF("Error: app_set_clkmode() failure.\n");

      /* Abnormal termination processing */
      error_leave_player();
      return false;
    }

    /* Set player operation mode. */
    if (!app_set_player_status())
    {
      AUDIO_PLAYER_DEBUG_PRINTF("Error: app_set_player_status() failure.\n");

      /* Abnormal termination processing */
      error_leave_player();
      return false;
    }

    /* Cancel output mute. */
    app_set_volume(PLAYER_DEF_VOLUME);
    if (board_external_amp_mute_control(false) != OK)
    {
      AUDIO_PLAYER_DEBUG_PRINTF("Error: board_external_amp_mute_control(false) failuer.\n");

      /* Abnormal termination processing */
      error_leave_player();
      return false;
    }
  }
  return true;
}

bool audio_player_play_file(const char *filename, uint8_t audio_channel, uint8_t bit_length, uint32_t sampling_rate, uint8_t codec_type, int duration)
{
  AUDIO_PLAYER_DEBUG_PRINTF("audio_channel : %i \n", audio_channel);
  AUDIO_PLAYER_DEBUG_PRINTF("bit_length : %i \n", bit_length);
  AUDIO_PLAYER_DEBUG_PRINTF("sampling_rate : %i \n", sampling_rate);
  AUDIO_PLAYER_DEBUG_PRINTF("codec : %i \n", codec_type);
  if (!app_open_file(filename, audio_channel, bit_length, sampling_rate, codec_type))
  {
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }
  return audio_player_play(duration);
}

#ifdef CONFIG_AUDIOUTILS_PLAYLIST
bool audio_player_play_next(int duration)
{
  if (!app_open_next_file())
  {
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }
  bool ret = audio_player_play(duration);
  AUDIO_PLAYER_DEBUG_PRINTF("%s return %i \n", __func__, ret);
  return ret;
}

bool audio_player_play_prev(int duration)
{
  if (!app_open_prev_file())
  {
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }
  return audio_player_play(duration);
}

bool audio_player_open_playlist()
{
  /* Open playlist. */
  if (!app_open_playlist())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_open_playlist() failure.\n");
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }
  return true;
}

#endif
bool audio_player_play(int duration)
{
  if (audio_player_status == AUDIO_PLAYER_NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error audio player not initialized \n");
    error_leave_player();
    return false;
  }

  if (!app_play_curr_file())
  {
    /* Abnormal termination processing */
    error_leave_player();
    return false;
  }

  /* Get current clock mode.
       * If the sampling rate is less than 48 kHz,
       * it will be in Normal mode. Otherwise, Hi-Res mode is set.
       */
  int cur_clk_mode;
  if (s_player_info.file.track.sampling_rate <= AS_SAMPLINGRATE_48000)
  {
    cur_clk_mode = AS_CLKMODE_NORMAL;
  }
  else
  {
    cur_clk_mode = AS_CLKMODE_HIRES;
  }

  if (!audio_player_handle_clk_mode(cur_clk_mode))
  {
    return false;
  }

  /* Start player operation. */
  if (!app_start())
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error: app_start_player() failure.\n");
    /* Abnormal termination processing */
    errout_start();
    return false;
  }

  /* Running... */
  AUDIO_PLAYER_DEBUG_PRINTF("Running time is %d sec\n", duration);
  app_play_process(duration);

  if (!audio_player_pause())
  {
    return false;
  }
  return true;
}

bool audio_player_pause()
{
  if (audio_player_status == AUDIO_PLAYER_PLAYING)
  {
    audio_player_status = AUDIO_PLAYER_IDLE;
    /* Stop player operation. */
    if (!app_stop())
    {
      AUDIO_PLAYER_DEBUG_PRINTF("Error: app_stop() failure.\n");
      return false;
    }
    return true;
  }
  else if (audio_player_status == AUDIO_PLAYER_NULL)
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Error audio player not initialized \n");
    return false;
  }
  else
  {
    AUDIO_PLAYER_DEBUG_PRINTF("Can't pause if audio player is not playing \n");
    return true;
  }
}

bool audio_player_set_volume(int master_db)
{
  return app_set_volume(master_db);
}

void audio_player_app_set_status_null()
{
  audio_player_status = AUDIO_PLAYER_NULL;
}
