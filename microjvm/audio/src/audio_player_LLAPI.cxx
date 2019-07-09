/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
*  Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */

#include "inc/audio_player_functions.h"
#include "inc/audio_player_LLAPI.h"
#include "pthread.h"
#include "semaphore.h"

#ifdef __cplusplus
extern "C"
{
#endif
  // #define AUDIO_PLAYER_LLAPI_DEBUG_TRACE

#ifdef AUDIO_PLAYER_LLAPI_DEBUG_TRACE
#define AUDIO_PLAYER_LLAPI_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define AUDIO_PLAYER_LLAPI_DEBUG_PRINTF(...) ((void)0)
#endif
  /****************************************************************************
 * Thread Calling Functions and Define
 ****************************************************************************/

  sem_t sem;
  pthread_t java_calling_thread;
  pthread_mutex_t mutex_calling_thread;
  pthread_cond_t cv_calling_thread;
  struct sni_context_s sni_ctx;
  struct audio_file_info_s audio_file_info;
  char last_error_message[32];
  /*
 * Utility function to stop current playing file
 * Must be called from java context
 */
  static bool Java_stop_playing_current_file()
  {
    bool ret = true;
    if (!Call_audio_player_pause())
    {
      SNI_throwNativeException(-1, "audio_player : error pausing current file");
      ret = false;
    }
    return ret;
  }

  void *Java_calling_thread_function_impl(void *args)
  {
    volatile bool shouldRun = true;
    pthread_cond_init(&cv_calling_thread, NULL);
    sem_init(&sem, 0, 0);
    while (shouldRun)
    {
      sem_wait(&sem);
      switch (sni_ctx.order_id)
      {
      case (AUDIO_PLAYER_CLOSE):
      {
        shouldRun = false;
        break;
      }
      case (AUDIO_PLAYER_PLAY_FILE):
      {
        assert(sni_ctx.size == sizeof(struct audio_file_info_s));
        audio_file_info_s *info = (struct audio_file_info_s *)sni_ctx.args;
        bool ret = Call_audio_player_play_file(info);
        AUDIO_PLAYER_LLAPI_DEBUG_PRINTF("AUDIO_PLAYER_PLAY_FILE -> status : %i \n", ret);
        sni_ctx.result = ret;
        SNI_resumeJavaThread(sni_ctx.java_thread_id);
        break;
      }
      default:
      {
        puts("Error unknown AUDIO_PLAYER commands");
        break;
      }
      }
    }
    error_leave_player();
    audio_player_app_set_status_null();
    return NULL;
  }

  /* Pause the currently playing audio file */
  bool Call_audio_player_pause()
  {
    if (!audio_player_pause())
    {
      AUDIO_PLAYER_LLAPI_DEBUG_PRINTF("%s : sni_ctx.result  = false \n", __func__);
      strcpy(last_error_message, "audio_player : pause failed");
      return false;
    }
    return true;
  }

  /* Play the designated file */
  bool Call_audio_player_play_file(struct audio_file_info_s *track)
  {
    if (!audio_player_play_file(track->filename, track->channel_number, track->bit_length, track->sampling_rate, track->codec, track->duration))
    {
      AUDIO_PLAYER_LLAPI_DEBUG_PRINTF("%s : sni_ctx.result  = false \n", __func__);
      strcpy(last_error_message, "audio_player : play file failed");
      return false;
    }
    return true;
  }

  bool Call_audio_player_set_volume(int volume_db)
  {
    if (!audio_player_set_volume(volume_db))
    {
      strcpy(last_error_message, "audio_player : set volume failed");
      return false;
    }
    return true;
  }

  /****************************************************************************
 * Native Public Function Impl
 ****************************************************************************/
  /* Call the initialization sequence */
  void Java_audio_player_initialize()
  {
    if (!audio_player_initialize())
    {
      SNI_throwNativeException(-1, "audio_player_initialize failed");
    }
    pthread_attr_t attr;
    int result = pthread_attr_init(&attr);
    // Initialize pthread such as its resource will be
    assert(result == 0);
    result = pthread_attr_setstacksize(&attr, 4096);
    assert(result == 0);
    result = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    assert(result == 0);
    pthread_create(&java_calling_thread, &attr, &Java_calling_thread_function_impl, NULL);
    pthread_attr_destroy(&attr);
  }

  /* Call the closing sequence */
  void Java_audio_player_close()
  {
    int thread_id = SNI_getCurrentJavaThreadID();
    sni_ctx.java_thread_id = thread_id;
    sni_ctx.args = NULL;
    sni_ctx.size = 0;
    sni_ctx.order_id = AUDIO_PLAYER_CLOSE;
    sem_post(&sem);
    pthread_join(java_calling_thread, NULL);
  }

  /* Load and play a given song */
  void Java_audio_player_play_file(uint8_t *filename, size_t filename_length,
                                   uint8_t audio_channel,
                                   uint8_t bit_length,
                                   uint32_t sampling_rate,
                                   uint8_t codec_type,
                                   int duration)
  {
    if (!Java_stop_playing_current_file())
    {
      return;
    }
    int thread_id = SNI_getCurrentJavaThreadID();
    sni_ctx.java_thread_id = thread_id;
    assert(filename_length < 64);
    strcpy(audio_file_info.filename, (char *)filename);
    audio_file_info.channel_number = audio_channel;
    audio_file_info.bit_length = bit_length;
    audio_file_info.sampling_rate = sampling_rate;
    audio_file_info.codec = codec_type;
    audio_file_info.duration = duration;

    sni_ctx.args = &audio_file_info;
    sni_ctx.size = sizeof(audio_file_info);
    sni_ctx.order_id = AUDIO_PLAYER_PLAY_FILE;
    sem_post(&sem);
    SNI_suspendCurrentJavaThread(0);
  }

  /* Pause the currently playing song */
  void Java_audio_player_pause()
  {
    bool ret = Call_audio_player_pause();
    sni_ctx.result = ret;
  }

  void Java_audio_player_fetch_result()
  {
    AUDIO_PLAYER_LLAPI_DEBUG_PRINTF("%s : %i \n", __func__, sni_ctx.result);
    if (sni_ctx.result == false)
    {
      SNI_throwNativeException(-1, last_error_message);
    }
  }

  void Java_audio_player_mute()
  {
    bool ret = Call_audio_player_set_volume(AUDIO_PLAYER_VOLUME_MIN_GAIN);
    sni_ctx.result = ret;
  }

  void Java_audio_player_set_volume(double volume_db)
  {
    if (volume_db >= AUDIO_PLAYER_VOLUME_MAX_GAIN)
    {
      volume_db = AUDIO_PLAYER_VOLUME_MAX_GAIN;
    }
    else if (volume_db <= AUDIO_PLAYER_VOLUME_MIN_GAIN)
    {
      volume_db = AUDIO_PLAYER_VOLUME_MIN_GAIN;
    }
    else
    {
      volume_db = 10 * volume_db;
    }
    bool ret = Call_audio_player_set_volume(volume_db);
    sni_ctx.result = ret;
  }

#ifdef __cplusplus
}
#endif