/**
 * @file demo.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_sip.h"
#include "numers.h"
#include "board_pins_config.h"
#include "board_def.h"




int orientation = PORTRAIT;
#define DIAL_BUT_W	66
#define DIAL_BUT_H	38	
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
// void destroy_buzzer(void);
static void write_create(lv_obj_t * parent);
static void btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void dial_create(lv_obj_t * parent);
//static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event);
//static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event);
//static void kb_hide_anim_end(lv_anim_t * a);
static void list_create(lv_obj_t * parent);
static void list_btn_event_handler(lv_obj_t * slider, lv_event_t event);
void ring_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void call_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void talk_in_progress(lv_obj_t * parent,const char * butlabel,const char * number);
void redial_choice(lv_obj_t * parent,const char * butlabel,const char * number);
extern sip_handle_t sip;
const char * lastnumber;

#if LV_DEMO_SLIDE_SHOW
static void tab_switcher(lv_task_t * task);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_obj_t * chart;
static lv_obj_t * ta;
static lv_obj_t * kb;

static lv_style_t style_kb;
static lv_style_t style_kb_rel;
static lv_style_t style_kb_pr;
int guistatus = 0;

lv_obj_t * cont; //call in progress container
lv_obj_t * ring; //ring in progress container
lv_obj_t * talk; //ring in progress container
lv_obj_t * tv; //tabview
lv_obj_t * redial;


typedef struct {
	char * number;
	char * name; 
	uint8_t lastaction;
	
}  log_record_t;

static log_record_t log[5];

#if LV_DEMO_WALLPAPER
LV_IMG_DECLARE(img_bubble_pattern)
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a demo application
 */
void sip_create_gui(void)
{
	orientation = PORTRAIT;
    lv_coord_t hres = lv_disp_get_hor_res(NULL);
    lv_coord_t vres = lv_disp_get_ver_res(NULL);

#if LV_DEMO_WALLPAPER
    lv_obj_t * wp = lv_img_create(lv_disp_get_scr_act(NULL), NULL);
    lv_img_set_src(wp, &img_bubble_pattern);
    lv_obj_set_width(wp, hres * 4);
    lv_obj_set_protect(wp, LV_PROTECT_POS);
#endif

    static lv_style_t style_tv_btn_bg;
    lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
    style_tv_btn_bg.body.main_color = lv_color_hex(0x487fb7);
    style_tv_btn_bg.body.grad_color = lv_color_hex(0x487fb7);
    style_tv_btn_bg.body.padding.top = 0;
    style_tv_btn_bg.body.padding.bottom = 0;

    static lv_style_t style_tv_btn_rel;
    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.body.opa = LV_OPA_TRANSP;
    style_tv_btn_rel.body.border.width = 0;

    static lv_style_t style_tv_btn_pr;
    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
    style_tv_btn_pr.body.radius = 0;
    style_tv_btn_pr.body.opa = LV_OPA_50;
    style_tv_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.border.width = 0;
    style_tv_btn_pr.text.color = LV_COLOR_GRAY;

    tv = lv_tabview_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_size(tv, hres, vres);

#if LV_DEMO_WALLPAPER
    lv_obj_set_parent(wp, ((lv_tabview_ext_t *) tv->ext_attr)->content);
    lv_obj_set_pos(wp, 0, -5);
#endif

    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "Book");
    lv_obj_t * tab2 = lv_tabview_add_tab(tv, "Dial");
  //  lv_obj_t * tab3 = lv_tabview_add_tab(tv, "Last");

#if LV_DEMO_WALLPAPER == 0
    /*Blue bg instead of wallpaper*/
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BG, &style_tv_btn_bg);
#endif
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_INDIC, &lv_style_plain);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

	write_create(tab2);
    dial_create(tab2);
    list_create(tab1);

#if LV_DEMO_SLIDE_SHOW
    lv_task_create(tab_switcher, 3000, LV_TASK_PRIO_MID, tv);
#endif
	guistatus=standby;
}

void clear_screen(void){
	
	switch (guistatus)
	{
		case ringing:
			lv_obj_del(ring);
			//destroy_buzzer();
			break;
		case talking:	
		    lv_obj_del(talk);
		     break;
		case calling:     
            lv_obj_del(cont);
             break;
        case redialing:
			lv_obj_del(redial);
			break;	
	}
	guistatus=0;
	
	lv_ta_set_text(ta,"");
	gpio_set_level(get_green_led_gpio(), 0);
}
void redial_choice(lv_obj_t * parent,const char * butlabel,const char * number)
{

    redial = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(redial, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(redial, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(redial, LV_FIT_FLOOD);
    lv_cont_set_layout(redial, LV_LAYOUT_OFF);

    lv_obj_t * label;
    label = lv_label_create(redial, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, -10, -10);
    lv_label_set_text(label, "BUSY");

    label = lv_label_create(redial, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(label, butlabel);
    
    
	lv_obj_t * btnc = lv_btn_create(redial, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnc, 10, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btnc, 100, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnc, btn_event_cb); 
    lv_obj_t * label1 = lv_label_create(btnc, NULL);
    lv_label_set_text(label1, "CLOSE");
    
    
	lv_obj_t * btna = lv_btn_create(redial, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btna, 130, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btna, 100, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btna, btn_event_cb); 
    lv_obj_t * label2 = lv_label_create(btna, NULL);
    lv_label_set_text(label2, "REDIAL");
    
    guistatus=redialing;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
static void dial_create(lv_obj_t * parent)
{

	
	lv_obj_t * btn1 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn1, 10, 35);                            /*Set its position*/
	lv_obj_set_size(btn1, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn1, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label1 = lv_label_create(btn1, NULL);          /*Add a label to the button*/
	lv_label_set_text(label1, "1");                     /*Set the labels text*/
	
	lv_obj_t * btn2 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn2, 86, 35);                            /*Set its position*/
	lv_obj_set_size(btn2, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn2, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label2 = lv_label_create(btn2, NULL);          /*Add a label to the button*/
	lv_label_set_text(label2, "2");                     /*Set the labels text*/
	
	lv_obj_t * btn3 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn3, 162, 35);                            /*Set its position*/
	lv_obj_set_size(btn3, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn3, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label3 = lv_label_create(btn3, NULL);          /*Add a label to the button*/
	lv_label_set_text(label3, "3");                     /*Set the labels text*/	
	
		
	lv_obj_t * btn4 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn4, 10, 45 + DIAL_BUT_H);                            /*Set its position*/
	lv_obj_set_size(btn4,DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn4, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label4 = lv_label_create(btn4, NULL);          /*Add a label to the button*/
	lv_label_set_text(label4, "4");                     /*Set the labels text*/
	
	lv_obj_t * btn5 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn5, 86, 45 + DIAL_BUT_H);                            /*Set its position*/
	lv_obj_set_size(btn5, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn5, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label5 = lv_label_create(btn5, NULL);          /*Add a label to the button*/
	lv_label_set_text(label5, "5");                     /*Set the labels text*/
	
	lv_obj_t * btn6 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn6, 162, 45 + DIAL_BUT_H);                            /*Set its position*/
	lv_obj_set_size(btn6, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn6, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label6 = lv_label_create(btn6, NULL);          /*Add a label to the button*/
	lv_label_set_text(label6, "6");                     /*Set the labels text*/	
	
			
	lv_obj_t * btn7 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn7, 10, 55 + DIAL_BUT_H * 2);                            /*Set its position*/
	lv_obj_set_size(btn7, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn7, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label7 = lv_label_create(btn7, NULL);          /*Add a label to the button*/
	lv_label_set_text(label7, "7");                     /*Set the labels text*/
	
	lv_obj_t * btn8 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn8, 86, 55 + DIAL_BUT_H * 2);                            /*Set its position*/
	lv_obj_set_size(btn8, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn8, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label8 = lv_label_create(btn8, NULL);          /*Add a label to the button*/
	lv_label_set_text(label8, "8");                     /*Set the labels text*/
	
	lv_obj_t * btn9 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn9, 162, 55 + DIAL_BUT_H * 2);                            /*Set its position*/
	lv_obj_set_size(btn9, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn9, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label9 = lv_label_create(btn9, NULL);          /*Add a label to the button*/
	lv_label_set_text(label9, "9");                     /*Set the labels text*/	
	
			
	lv_obj_t * btna = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btna, 10, 65 + DIAL_BUT_H * 3);                            /*Set its position*/
	lv_obj_set_size(btna, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btna, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labela = lv_label_create(btna, NULL);          /*Add a label to the button*/
	lv_label_set_text(labela, "*");                     /*Set the labels text*/
	
	lv_obj_t * btn0 = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn0, 86, 65 + DIAL_BUT_H * 3);                            /*Set its position*/
	lv_obj_set_size(btn0, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btn0, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * label0 = lv_label_create(btn0, NULL);          /*Add a label to the button*/
	lv_label_set_text(label0, "0");                     /*Set the labels text*/
	
	lv_obj_t * btnb = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnb, 162, 65 + DIAL_BUT_H * 3);                            /*Set its position*/
	lv_obj_set_size(btnb, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnb, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labelb = lv_label_create(btnb, NULL);          /*Add a label to the button*/
	lv_label_set_text(labelb, "#");                     /*Set the labels text*/	
	
	lv_obj_t * btnd = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnd, 10, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btnd, DIAL_BUT_W, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnd, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labeld = lv_label_create(btnd, NULL);          /*Add a label to the button*/
	lv_label_set_text(labeld, "C");                     /*Set the labels text*/
	
	lv_obj_t * btnc = lv_btn_create(parent, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnc, 86, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btnc, DIAL_BUT_W * 2 + 10, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnc, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labelc = lv_label_create(btnc, NULL);          /*Add a label to the button*/
	lv_label_set_text(labelc, "CALL");                     /*Set the labels text*/

}	
void ring_in_progress(lv_obj_t * parent,const char * butlabel,const char * number)
{
	

    ring = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(ring, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(ring, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(ring, LV_FIT_FLOOD);
    lv_cont_set_layout(ring, LV_LAYOUT_OFF);

    lv_obj_t * label;
    label = lv_label_create(ring, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, -30, -10);
    lv_label_set_text(label, "Incoming call");

    label = lv_label_create(ring, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(label, butlabel);
    
    
	lv_obj_t * btnc = lv_btn_create(ring, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnc, 10, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btnc, 100, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnc, btn_event_cb); 
       lv_obj_t * label1 = lv_label_create(btnc, NULL);
    lv_label_set_text(label1, "REJECT");
    
    
	lv_obj_t * btna = lv_btn_create(ring, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btna, 130, 75 + DIAL_BUT_H * 4);                            /*Set its position*/
	lv_obj_set_size(btna, 100, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btna, btn_event_cb); 
    lv_obj_t * label2 = lv_label_create(btna, NULL);
    lv_label_set_text(label2, "ANSWER");
    
    guistatus=ringing;
}
void talk_in_progress(lv_obj_t * parent,const char * butlabel,const char * number)
{
	

    talk = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(talk, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(talk, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(talk, LV_FIT_FLOOD);
    lv_cont_set_layout(talk, LV_LAYOUT_CENTER);

    lv_obj_t * label;
    label = lv_label_create(talk, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 100, 30);
    lv_label_set_text(label, "Conected");

    label = lv_label_create(talk, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 100, 30);
    lv_label_set_text(label, butlabel);
    
    
	lv_obj_t * btnc = lv_btn_create(talk, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnc, 46, 250);                            /*Set its position*/
	lv_obj_set_size(btnc, DIAL_BUT_W * 2 + 10, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnc, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labelc = lv_label_create(btnc, NULL);          /*Add a label to the button*/
	lv_label_set_text(labelc, "CLOSE");     
	guistatus=talking;   
}
void call_in_progress(lv_obj_t * parent,const char * butlabel,const char * number)
{
	

    cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(cont, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(cont, LV_FIT_FLOOD);
    lv_cont_set_layout(cont, LV_LAYOUT_CENTER);

    lv_obj_t * label;
    label = lv_label_create(cont, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 100, 30);
    lv_label_set_text(label, "Calling...");

    label = lv_label_create(cont, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); 
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 100, 30);
    lv_label_set_text(label, butlabel);
    
    
	lv_obj_t * btnc = lv_btn_create(cont, NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btnc, 46, 250);                            /*Set its position*/
	lv_obj_set_size(btnc, DIAL_BUT_W * 2 + 10, DIAL_BUT_H);                          /*Set its size*/
	lv_obj_set_event_cb(btnc, btn_event_cb);                 /*Assign a callback to the button*/
	lv_obj_t * labelc = lv_label_create(btnc, NULL);          /*Add a label to the button*/
	lv_label_set_text(labelc, "CLOSE");   
		guistatus=calling;     
}
static void write_create(lv_obj_t * parent)
{
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);

    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    static lv_style_t style_ta;
    lv_style_copy(&style_ta, &lv_style_pretty);
    style_ta.body.opa = LV_OPA_30;
    style_ta.body.radius = 0;
    style_ta.text.color = lv_color_hex3(0x222);

    ta = lv_ta_create(parent, NULL);
    lv_obj_set_size(ta, lv_page_get_scrl_width(parent), 30);
    lv_ta_set_style(ta, LV_TA_STYLE_BG, &style_ta);
    lv_ta_set_text(ta, "");
//    lv_obj_set_event_cb(ta, text_area_event_handler);
    lv_style_copy(&style_kb, &lv_style_plain);
    lv_ta_set_text_sel(ta, false);
#if 0
    style_kb.body.opa = LV_OPA_70;
    style_kb.body.main_color = lv_color_hex3(0x333);
    style_kb.body.grad_color = lv_color_hex3(0x333);
    style_kb.body.padding.left = 0;
    style_kb.body.padding.right = 0;
    style_kb.body.padding.top = 0;
    style_kb.body.padding.bottom = 0;
    style_kb.body.padding.inner = 0;

    lv_style_copy(&style_kb_rel, &lv_style_plain);
    style_kb_rel.body.opa = LV_OPA_TRANSP;
    style_kb_rel.body.radius = 0;
    style_kb_rel.body.border.width = 1;
    style_kb_rel.body.border.color = LV_COLOR_SILVER;
    style_kb_rel.body.border.opa = LV_OPA_50;
    style_kb_rel.body.main_color = lv_color_hex3(0x333);    /*Recommended if LV_VDB_SIZE == 0 and bpp > 1 fonts are used*/
    style_kb_rel.body.grad_color = lv_color_hex3(0x333);
    style_kb_rel.text.color = LV_COLOR_WHITE;

    lv_style_copy(&style_kb_pr, &lv_style_plain);
    style_kb_pr.body.radius = 0;
    style_kb_pr.body.opa = LV_OPA_50;
    style_kb_pr.body.main_color = LV_COLOR_WHITE;
    style_kb_pr.body.grad_color = LV_COLOR_WHITE;
    style_kb_pr.body.border.width = 1;
    style_kb_pr.body.border.color = LV_COLOR_SILVER;
#endif
}
#if 0
static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event)
{
    (void) text_area;    /*Unused*/

    /*Text area is on the scrollable part of the page but we need the page itself*/
    lv_obj_t * parent = lv_obj_get_parent(lv_obj_get_parent(ta));

    if(event == LV_EVENT_CLICKED) {
        if(kb == NULL) {
            kb = lv_kb_create(parent, NULL);
            lv_obj_set_size(kb, lv_obj_get_width_fit(parent), lv_obj_get_height_fit(parent) / 2);
            lv_obj_align(kb, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
            lv_kb_set_ta(kb, ta);
            lv_kb_set_style(kb, LV_KB_STYLE_BG, &style_kb);
            lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &style_kb_rel);
            lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &style_kb_pr);
            lv_obj_set_event_cb(kb, keyboard_event_cb);

#if LV_USE_ANIMATION
            lv_anim_t a;
            a.var = kb;
            a.start = LV_VER_RES;
            a.end = lv_obj_get_y(kb);
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_linear;
            a.ready_cb = NULL;
            a.act_time = 0;
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
#endif
        }
    }

}
#endif
static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
	

    if (event == LV_EVENT_CLICKED) {
       const char * butlabel = lv_list_get_btn_text(btn);
       
       if  ( strcmp(butlabel,"CALL") == 0) {
		   const char * number = lv_ta_get_text(ta);
		   if (strcmp(number,"") !=0){
		   //call_in_progress(lv_disp_get_scr_act(NULL),number,number);
		   //lv_ta_set_text(ta,"");
		   lastnumber = number;
		   esp_sip_uac_invite(sip, number);
		   
			}
	   } else if ( strcmp(butlabel,"REDIAL") == 0){
		   clear_screen();
		   call_in_progress(lv_disp_get_scr_act(NULL),lastnumber,lastnumber);
		   esp_sip_uac_invite(sip, lastnumber);
		   
			
			
		}  else if ( strcmp(butlabel,"C") == 0){
		   
 
	   lv_ta_set_text(ta, "");
	   
   } else if (strcmp(butlabel,"END") == 0){
	   
	   
	   esp_sip_uac_bye(sip); 
	   clear_screen();
	   
   } else if (strcmp(butlabel,"CLOSE") == 0){
	   
	 
		esp_sip_uac_cancel(sip);
	   esp_sip_uac_bye(sip);
	   clear_screen();
	   
   } else if (strcmp(butlabel,"ANSWER") == 0){
	   
	   //clear_screen(); 
	   
	   const char * number = "300";
	   
	   
	   esp_sip_uas_answer(sip, true);
		//   talk_in_progress(lv_disp_get_scr_act(NULL),number,number);
	   
	   
   } else if (strcmp(butlabel,"REJECT") == 0){
	   
	   esp_sip_uas_answer(sip, false);
	   esp_sip_uac_bye(sip);
	  // clear_screen();
	   
   } else {
        lv_ta_add_text(ta, butlabel);
    }
}
}

static void kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_del(a->var);
    kb = NULL;
}
static void list_create(lv_obj_t * parent)
{
    lv_coord_t hres = lv_disp_get_hor_res(NULL);

    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &lv_style_transp_fit);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &lv_style_transp_fit);

    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = lv_color_hex3(0x333);
    style_btn_rel.body.grad_color = LV_COLOR_BLACK;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 1;
    style_btn_rel.body.border.opa = LV_OPA_50;
    style_btn_rel.body.radius = 0;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = lv_color_make(0x55, 0x96, 0xd8);
    style_btn_pr.body.grad_color = lv_color_make(0x37, 0x62, 0x90);
    style_btn_pr.text.color = lv_color_make(0xbb, 0xd5, 0xf1);

    lv_obj_t * list = lv_list_create(parent, NULL);
    lv_obj_set_height(list, lv_obj_get_height(parent));
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, &style_btn_rel);
    lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, &style_btn_pr);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);//LV_DPI / 4);

    lv_obj_t * list_btn;
    list_btn = lv_list_add_btn(list, LV_SYMBOL_CALL, Names[0]);
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_CALL, Names[1]);
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_CALL, Names[2]);
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_CALL, Names[3]);
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_CALL, Names[4]);
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);
}


/**
 * Called when a a list button is clicked on the List tab
 * @param btn pointer to a list button
 * @return LV_RES_OK because the button is not deleted in the function
 */
static void list_btn_event_handler(lv_obj_t * btn, lv_event_t event)
{

    if(event == LV_EVENT_SHORT_CLICKED) {

        char * butlabel = lv_list_get_btn_text(btn);
		char * number = "999";//todo
		for (int x=0;x<5;x++) {
			if (strcmp(butlabel,Names[x]) == 0)
					number = Numbers[x];
					break;
	 
		}     
		
       
       
       //call_in_progress(lv_disp_get_scr_act(NULL),butlabel,number);
       lastnumber = number;
       esp_sip_uac_invite(sip, number);
	   
       
   
}
}
#if LV_DEMO_SLIDE_SHOW
/**
 * Called periodically (lv_task) to switch to the next tab
 */
static void tab_switcher(lv_task_t * task)
{
    static uint8_t tab = 0;
    lv_obj_t * tv = task->user_data;
    tab++;
    if(tab >= 3) tab = 0;
    lv_tabview_set_tab_act(tv, tab, true);
}
#endif


