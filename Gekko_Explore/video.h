#ifndef __VIDEO_H__
#define __VIDEO_H__

/* Configuring demo: */
#define VIDEO_PAL
//efine VIDEO_NTSC

void vi_initialize(void);
int my_printf(char *msg, ...);
void put_logo(void);
void update_message_buf(void);
void vi_swap_chain (void);
int vi_get_frame_count (void);

#endif // __VIDEO_H__
