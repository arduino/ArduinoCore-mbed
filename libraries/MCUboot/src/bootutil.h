#ifndef __BOOTUTIL_H
#define __BOOTUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int boot_set_confirmed(void);
int boot_set_pending(int permanent);
int boot_set_debug(int enable);

#ifdef __cplusplus
}
#endif

#endif /* __BOOTUTIL_H */