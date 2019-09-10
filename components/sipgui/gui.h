
/**
 * @file gui.h
 *
 */

#ifndef GUI_H
#define GUI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "../../../lvgl/lvgl.h"
#endif


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a demo application
 */
void sip_create_gui(void);
void ring_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void call_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void talk_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void redial_choice(lv_obj_t * parent,const char * butlabel,const char * number);
extern lv_obj_t * cont; //call in progress container
extern lv_obj_t * ring; //ring in progress container
extern lv_obj_t * talk; //ring in progress container
extern lv_obj_t * tv; //tabview
extern lv_obj_t * redial; //tabview
void clear_screen();

enum {
	standby,
	calling,
	ringing,
	talking,
	redialing,
};


/**********************
 *      MACROS
 **********************/



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GUI_H*/


