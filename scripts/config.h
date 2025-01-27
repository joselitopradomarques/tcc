#ifndef CONFIG_H
#define CONFIG_H

#include <alsa/asoundlib.h>

// Declarações de variáveis externas para configuração do dispositivo PCM
extern snd_pcm_t *pcm_handle;          // Manipulador do dispositivo PCM
extern snd_pcm_hw_params_t *params;    // Parâmetros de hardware
extern unsigned int rate;              // Taxa de amostragem
extern snd_pcm_uframes_t buffer_size;  // Tamanho do buffer
extern short *buffer;                  // Buffer de áudio

#endif // CONFIG_H
