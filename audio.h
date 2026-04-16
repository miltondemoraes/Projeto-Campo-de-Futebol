#ifndef AUDIO_H
#define AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

void audioInit(void);
void audioShutdown(void);
void audioPlayKick(void);
void audioPlayGoal(void);

#ifdef __cplusplus
}
#endif

#endif
