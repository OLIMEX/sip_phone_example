/* SIP Phone Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"
#include "input_key_service.h"

#include "driver/gpio.h"
#include "lvgl/lvgl.h"
#include "esp_freertos_hooks.h"
#include "downmix.h"
#include "audio_mem.h"
#include "raw_stream.h"
#include "filter_resample.h"
#include "esp_sip.h"
#include "g711.h"
#include "mp3_decoder.h"
#include "drv/stmpe610.h"
#include "drv/ili9341.h"
#include "drv/disp_spi.h"

#include "sipgui/gui.h"
#include "board_pins_config.h"

#define FAILLING_END_GAIN       1
#define RISING_START_GAIN       2
#define MUSIC_GAIN_DB          -10
#define PLAY_STATUS            ESP_DOWNMIX_TYPE_OUTPUT_ONE_CHANNEL


// ==== Display dimensions in pixels ============================
int _width = DEFAULT_TFT_DISPLAY_WIDTH;
int _height = DEFAULT_TFT_DISPLAY_HEIGHT;
extern int guistatus;
extern const char * lastnumber;
static void IRAM_ATTR lv_tick_task(void);
/*
   To embed it in the app binary, the mp3 file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/

static int mp3_pos;
static const char *TAG = "SIP_PHONE";

#define RTP_HEADER_LEN 12
#define AUDIO_FRAME_SIZE (160)

#define I2S_SAMPLE_RATE     48000
#define I2S_CHANNEL         2
#define I2S_BITS            16

#define G711_SAMPLE_RATE    8000
#define G711_CHANNEL        1

sip_handle_t sip;
static esp_err_t g711enc_pipeline_open();
static esp_err_t g711dec_pipeline_open();
audio_element_handle_t i2s_stream_reader;
audio_element_handle_t raw_read;
audio_element_handle_t raw_write,el_raw_write,speaker_raw_write;
audio_pipeline_handle_t speaker,ringer,master;
audio_element_handle_t mp3_decoder;
audio_element_handle_t filter,downmixer;
audio_element_handle_t efilter;
audio_element_handle_t pipeline;
audio_pipeline_handle_t recorder;
audio_event_iface_handle_t evt;    
audio_element_handle_t i2s_stream_writer;

static uint8_t registrated = 0;
static uint8_t autoanswer = 0;
static uint8_t ringin = 0;
#define SAVE_FILE_RATE      44100
#define SAVE_FILE_CHANNEL   2
#define SAVE_FILE_BITS      16

#define PLAYBACK_RATE       48000
#define PLAYBACK_CHANNEL    2
#define PLAYBACK_BITS       16



extern const uint8_t adf_music_mp3_start[] asm("_binary_adf_music_mp3_start");
extern const uint8_t adf_music_mp3_end[]   asm("_binary_adf_music_mp3_end");

int mp3_music_read_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    int read_size = adf_music_mp3_end - adf_music_mp3_start - mp3_pos;
    if (read_size == 0) {
		ESP_LOGI(TAG, "RING STOPED");	
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, adf_music_mp3_start + mp3_pos, read_size);
   ESP_LOGI(TAG, "MP3 READ CALLBACK: pos:%d size:%d",mp3_pos,read_size);
    mp3_pos += read_size;
    return read_size;
}

static esp_err_t g711enc_pipeline_open()
{
   
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    recorder = audio_pipeline_init(&pipeline_cfg);
    if (NULL == recorder) {
        return ESP_FAIL;
    }
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
#if defined CONFIG_ESP_LYRAT_MINI_V1_1_BOARD
    i2s_cfg.i2s_port = 1;
#endif
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);
    audio_element_info_t i2s_info = {0};
    audio_element_getinfo(i2s_stream_reader, &i2s_info);
    i2s_info.bits = I2S_BITS;
    i2s_info.channels = I2S_CHANNEL;
    i2s_info.sample_rates = I2S_SAMPLE_RATE;
    audio_element_setinfo(i2s_stream_reader, &i2s_info);

    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = I2S_SAMPLE_RATE;
    rsp_cfg.src_ch = I2S_CHANNEL;
    rsp_cfg.dest_rate = G711_SAMPLE_RATE;
    rsp_cfg.dest_ch = G711_CHANNEL;
    rsp_cfg.type = AUDIO_CODEC_TYPE_ENCODER;
    efilter = rsp_filter_init(&rsp_cfg);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_READER;
    raw_read = raw_stream_init(&raw_cfg);
    audio_element_set_output_timeout(raw_read, portMAX_DELAY);

    audio_pipeline_register(recorder, i2s_stream_reader, "ei2s");
    audio_pipeline_register(recorder, efilter, "filter");

    audio_pipeline_register(recorder, raw_read, "eraw");
    audio_pipeline_link(recorder, (const char *[]) {"ei2s", "filter", "eraw"}, 3);
    audio_pipeline_run(recorder);
    ESP_LOGI(TAG, "Recorder has been created");
    return ESP_OK;
}

static int _g711_encode(char *data, int len)
{
    int out_len_bytes;
	if (guistatus != talking) return 0;
    char *enc_buffer = (char *)audio_malloc(2 * AUDIO_FRAME_SIZE);
    out_len_bytes = raw_stream_read(raw_read, enc_buffer, 2 * AUDIO_FRAME_SIZE);
    if (out_len_bytes > 0) {
        int16_t *enc_buffer_16 = (int16_t *)(enc_buffer);
        for (int i = 0; i < AUDIO_FRAME_SIZE; i++) {
#ifdef CONFIG_SIP_CODEC_G711A
            data[i] = esp_g711a_encode(enc_buffer_16[i]);
#else
            data[i] = esp_g711u_encode(enc_buffer_16[i]);
#endif
        }
        free(enc_buffer);
        return AUDIO_FRAME_SIZE;
    } else {
        free(enc_buffer);
        return 0;
    }
}

static  esp_err_t play_ring(void)
{
	
	ESP_LOGI(TAG, "RING : %d",ringin);
	ringin = 0;
	return ESP_OK;
	
	// TODO: Play ring tone here:
	//mp3_pos = 0;	
	//audio_pipeline_run(ringer);
	////audio_pipeline_resume(ringer);
	//ESP_LOGI(TAG, "[3.3] downmix_set_second_input");
    //downmix_set_second_input_rb_timeout(downmixer, portMAX_DELAY);
    //// When pipeline_tone is finished, downmixer will change status to `DOWNMIX_SWITCH_OFF`
    //downmix_set_play_status(downmixer, DOWNMIX_SWITCH_ON);

	  
	 //while (1) {
		 //vTaskDelay(2);
	
        //audio_event_iface_msg_t msg;
        //esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        //if (ret != ESP_OK) {
            //ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            //continue;
        //}
        //if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            //&& msg.source == (void *) mp3_decoder
            //&& msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            //audio_element_info_t music_info = {0};
            //audio_element_getinfo(mp3_decoder, &music_info);
            //ESP_LOGE(TAG, "[ * ] Receive music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
                     //music_info.sample_rates, music_info.bits, music_info.channels);
            //downmix_set_base_file_info(downmixer, music_info.sample_rates, music_info.channels);
            //music_info.channels = PLAY_STATUS + 1;
            //audio_element_setinfo(i2s_stream_writer, &music_info);
            //i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);   
            //continue;
        //}
        ///* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        //if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            //&& msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            //&& (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            //ESP_LOGW(TAG, "[ * ] Stop event received");
            //break;
        //}
	//}
	 
	 //ringin = 0;
//return ESP_OK;   
	
}

static esp_err_t g711dec_pipeline_open()
{
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    speaker = audio_pipeline_init(&pipeline_cfg);
    ringer = audio_pipeline_init(&pipeline_cfg);
    master = audio_pipeline_init(&pipeline_cfg);
    
  //  audio_pipeline_cfg_t pipeline_tone_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    

    if ((NULL == speaker) || (NULL == master) || (NULL == ringer)){
        return ESP_FAIL;
    }
    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    raw_write = raw_stream_init(&raw_cfg);

    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = G711_SAMPLE_RATE;
    rsp_cfg.src_ch = G711_CHANNEL;
    rsp_cfg.dest_rate = I2S_SAMPLE_RATE;
    rsp_cfg.dest_ch = I2S_CHANNEL;
    rsp_cfg.complexity = 5;
    rsp_cfg.type = AUDIO_CODEC_TYPE_DECODER;
    rsp_cfg.task_core = 0;
    filter = rsp_filter_init(&rsp_cfg);

    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    
    audio_element_info_t i2s_info = {0};
    audio_element_getinfo(i2s_stream_writer, &i2s_info);
    i2s_info.bits = I2S_BITS;
    i2s_info.channels = I2S_CHANNEL;
    i2s_info.sample_rates = I2S_SAMPLE_RATE;
    audio_element_setinfo(i2s_stream_writer, &i2s_info);
    
    ESP_LOGI(TAG, "[2.1] Create mp3 decoder to decode mp3 file and set custom read callback");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_cfg.task_core = 1;
    mp3_decoder = mp3_decoder_init(&mp3_cfg);
    audio_element_set_read_cb(mp3_decoder, mp3_music_read_cb, NULL);
    
    
    
    ESP_LOGI(TAG, "[2.3] Create downmixer");
    downmix_cfg_t downmix_cfg = DEFAULT_DOWNMIX_CONFIG();
    downmix_cfg.downmix_info.gain[FAILLING_END_GAIN] = MUSIC_GAIN_DB;
    downmix_cfg.downmix_info.gain[RISING_START_GAIN] = MUSIC_GAIN_DB;
    downmixer = downmix_init(&downmix_cfg);
    ESP_LOGI(TAG, "[3.1] Create raw stream to write data");
    
      
    raw_cfg.type = AUDIO_STREAM_WRITER;
    el_raw_write = raw_stream_init(&raw_cfg);
    speaker_raw_write = raw_stream_init(&raw_cfg);
    ESP_LOGI(TAG, "[2.2] Register all elements to audio pipeline");
      
    audio_pipeline_register(speaker, raw_write, "raw");
    audio_pipeline_register(speaker, filter, "filter");
    audio_pipeline_register(speaker, speaker_raw_write, "speaker_raw");
    
    audio_pipeline_register(master, downmixer,"mixer");
    audio_pipeline_register(master, i2s_stream_writer, "i2s");
    
    audio_pipeline_link(speaker, (const char *[]) {"raw", "filter","speaker_raw"}, 3);
    
       
    audio_pipeline_register(ringer, mp3_decoder, "mp3");
    audio_pipeline_register(ringer, el_raw_write, "ring_raw");
    
    ESP_LOGI(TAG, "[3.6] Link elements together fatfs_stream-->raw_stream");
    audio_pipeline_link(ringer, (const char *[]) {"mp3", "ring_raw"}, 2);
    
    
    
    
    
    audio_pipeline_link(master, (const char *[]) {"mixer", "i2s"}, 2);
 	ESP_LOGI(TAG, "[ 3 ] Set up  event listener");
 	
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    
	evt = audio_event_iface_init(&evt_cfg);
	


	audio_pipeline_set_listener(master,evt);
	    ESP_LOGI(TAG, "master,evt");
 	audio_pipeline_set_listener(ringer,evt);
	    ESP_LOGI(TAG, "ringer,evt");
	    audio_pipeline_set_listener(speaker,evt);
	    ESP_LOGI(TAG, "speaker,evt");
	audio_element_msg_set_listener(mp3_decoder, evt);
    ESP_LOGI(TAG, "Speaker has been created");

    

    ESP_LOGI(TAG, "[5.2] Connect input ringbuffer of pipeline_base & pipeline_newcome to downmixer input");
    ringbuf_handle_t rb = audio_element_get_input_ringbuf(speaker_raw_write);
    
    
    audio_element_set_input_ringbuf(downmixer, rb);
    
    rb = audio_element_get_input_ringbuf(el_raw_write);
    downmix_set_second_input_rb(downmixer, rb);
    
    audio_pipeline_run(speaker);    
    audio_pipeline_run(master);
    downmix_set_output_status(downmixer, PLAY_STATUS);
    return ESP_OK;
}


static int _g711_decode(char *data, int len)
{
	
	
	if (guistatus != talking)
	{
		ESP_LOGE(TAG, "_g711_decoder error");
		 return 0;
	}
	
	
	
    int16_t *dec_buffer = (int16_t *)audio_malloc(2 * (len - RTP_HEADER_LEN));

    for (int i = 0; i < (len - RTP_HEADER_LEN); i++) {
#ifdef CONFIG_SIP_CODEC_G711A
        dec_buffer[i] = esp_g711a_decode((unsigned char)data[RTP_HEADER_LEN + i]);
#else
        dec_buffer[i] = esp_g711u_decode((unsigned char)data[RTP_HEADER_LEN + i]);
#endif
    }

    raw_stream_write(raw_write, (char *)dec_buffer, 2 * (len - RTP_HEADER_LEN));
    free(dec_buffer);
   // ESP_LOGI(TAG, "_g711_decoded");
    return 2 * (len - RTP_HEADER_LEN);
}

static ip4_addr_t _get_network_ip()
{
    tcpip_adapter_ip_info_t ip;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
    return ip.ip;
}

static int _sip_event_handler(sip_event_msg_t *event)
{
    ip4_addr_t ip;
    switch ((int)event->type) {
        case SIP_EVENT_REQUEST_NETWORK_STATUS:
            ESP_LOGI(TAG, "SIP_EVENT_REQUEST_NETWORK_STATUS");
            ip = _get_network_ip();
            if (ip.addr) {
                return true;
            }
            return ESP_OK;
        case SIP_EVENT_REQUEST_NETWORK_IP:
            ESP_LOGI(TAG, "SIP_EVENT_REQUEST_NETWORK_IP");
            ip = _get_network_ip();
            int ip_len = sprintf((char *)event->data, "%s", ip4addr_ntoa(&ip));
            return ip_len;
        case SIP_EVENT_REGISTERED:
            ESP_LOGI(TAG, "SIP_EVENT_REGISTERED");
            if (registrated == 0)
            {
            
            registrated = 1;
			}
            break;
         case SIP_EVENT_UNREGISTERED:
			registrated = 0;
         break;   
        case SIP_EVENT_RINGING:
            ESP_LOGI(TAG, "ringing... RemotePhoneNum %s", (char *)event->data);
           	static uint8_t flasher = 0;
			flasher++;
			gpio_set_level(get_green_led_gpio(), flasher & 1);
            const char * number = "unknown";
            
            
            if (guistatus != ringing){
			ESP_LOGI(TAG, "Creating RING message...");
			number = event->data;
			lastnumber = number;
            ring_in_progress(lv_disp_get_scr_act(NULL),number,number);
            
			}
			if (autoanswer==1)
			{
					esp_sip_uas_answer(sip, true);
					break;
			}		
			if (ringin == 0)
			{
				ringin = 1;	
				play_ring();
			}			
			
			
					
            break;
        case SIP_EVENT_INVITING:
            ESP_LOGI(TAG, "SIP_EVENT_INVITING");
            if (guistatus != calling)
            {
				clear_screen();
				call_in_progress(lv_disp_get_scr_act(NULL),lastnumber,lastnumber);
            }
            break;
        case SIP_EVENT_BUSY:
            ESP_LOGI(TAG, "SIP_EVENT_BUSY");
            if (guistatus == calling)
            {
				clear_screen();
				ESP_LOGI(TAG, "Redial choice");
				redial_choice(lv_disp_get_scr_act(NULL),lastnumber,lastnumber);
            }
            break;
        case SIP_EVENT_HANGUP:
            ESP_LOGI(TAG, "SIP_EVENT_HANGUP");
            

		 		clear_screen();
				//if (ringin == 1) ringin = 2;	
            ringin = 0;
            break;
        case SIP_EVENT_AUDIO_SESSION_BEGIN:
            ESP_LOGI(TAG, "SIP_EVENT_AUDIO_SESSION_BEGIN");

			ringin = 0;
              clear_screen();
              if (guistatus != talking){
					
			    talk_in_progress(lv_disp_get_scr_act(NULL),lastnumber,lastnumber);
			}      
            break;
        case SIP_EVENT_AUDIO_SESSION_END:
            ESP_LOGI(TAG, "SIP_EVENT_AUDIO_SESSION_END");
            
            clear_screen();
            
            
            break;
        case SIP_EVENT_READ_AUDIO_DATA:
            return _g711_encode(event->data, event->data_len);
        case SIP_EVENT_WRITE_AUDIO_DATA:
            return _g711_decode(event->data, event->data_len);
    }
    
    
    
    
    
    return 0;
}

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    audio_board_handle_t board_handle = (audio_board_handle_t) ctx;
    int player_volume;
    audio_hal_get_volume(board_handle->audio_hal, &player_volume);
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        sip_state_t sip_state = esp_sip_get_state(sip);
        if (sip_state < SIP_STATE_REGISTERED) {
            return ESP_OK;
        }
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_REC:
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGI(TAG, "[ * ] [Play] input key event");
                if (sip_state & SIP_STATE_RINGING) {
                    esp_sip_uas_answer(sip, true);
                }
                if (sip_state & SIP_STATE_REGISTERED) {
                    esp_sip_uac_invite(sip, "999");
                }
                break;
            case INPUT_KEY_USER_ID_MODE:
            case INPUT_KEY_USER_ID_SET:
                ESP_LOGI(TAG, "[ * ] [Set] input key event");
                if (sip_state & SIP_STATE_RINGING) {
                    esp_sip_uas_answer(sip, false);
                } else if (sip_state & SIP_STATE_ON_CALL) {
                    esp_sip_uac_bye(sip);
                } else if  ((sip_state & SIP_STATE_CALLING) || (sip_state & SIP_STATE_SESS_PROGRESS)) {
                    esp_sip_uac_cancel(sip);
                }
                break;
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGI(TAG, "[ * ] [Vol+] input key event");
                player_volume += 10;
                if (player_volume > 100) {
                    player_volume = 100;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGI(TAG, "[ * ] [Vol-] input key event");
                player_volume -= 10;
                if (player_volume < 0) {
                    player_volume = 0;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
        }
    }
    
    
    

    return ESP_OK;
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_level_set("SIP_PHONE", ESP_LOG_DEBUG);
    esp_log_level_set("SIP", ESP_LOG_DEBUG);
    
	
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    tcpip_adapter_init();

    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.2] Initialize and start peripherals");
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[1.3] Start and wait for Wi-Fi network");
    periph_wifi_cfg_t wifi_cfg = {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    esp_periph_start(set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    periph_service_handle_t input_ser = input_key_service_create(set);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);

    ESP_LOGI(TAG, "[ 4 ] Create SIP Service");
    sip_config_t sip_cfg = {
        .uri = CONFIG_SIP_URI,
        .event_handler = _sip_event_handler,
#ifdef CONFIG_SIP_CODEC_G711A
        .acodec_type = SIP_ACODEC_G711A,
#else
        .acodec_type = SIP_ACODEC_G711U,
#endif
    };
    sip = esp_sip_init(&sip_cfg);
    

    ESP_LOGI(TAG, "[ 5 ] Create decoder and encoder pipelines");
    g711enc_pipeline_open();
    g711dec_pipeline_open();
    
    ESP_LOGI(TAG, "[ 6 ] Create GUI");
    disp_spi_init();
	ili9341_init();
	stmpe610_Init();
    lv_init();    
    
 	vTaskDelay(10 / portTICK_RATE_MS);
    uint32_t tver = stmpe610_getID();

    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = ili9341_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = TP_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
    
	esp_register_freertos_tick_hook(lv_tick_task);
 
	sip_create_gui();
	
	
	
	esp_sip_start(sip);
	
	
	
	gpio_set_direction(get_green_led_gpio(), GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(get_green_led_gpio(), 0);
	while(1) {
		vTaskDelay(2);
		lv_task_handler();

	}
 
   
}
static void IRAM_ATTR lv_tick_task(void)
{
	lv_tick_inc(portTICK_RATE_MS);

}
