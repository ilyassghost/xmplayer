/* 
 * File:   newmain.c
 * Author: cc
 *
 * Created on 27 avril 2012, 19:58
 */

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <fat/fat.h>
#include <libfat/fat.h>
#include <usb/usbmain.h>
#include <diskio/ata.h>
#include <xenon_soc/xenon_power.h>

#include <time/time.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_list.h"

#include "../build/home_left_png.h"
#include "../build/home_main_function_frame_bg_png.h"
#include "../build/welcome_background_bg_png.h"
#include "../build/home_music_sm_icon_n_png.h"
#include "../build/home_photo_sm_icon_n_png.h"
#include "../build/home_video_sm_icon_n_png.h"
#include "../build/home_settings_sm_icon_n_png.h"
#include "../build/home_hdd_sub_icon_n_png.h"
#include "../build/home_nohdd_sub_icon_n_png.h"
#include "../build/home_horizontal_select_bar_png.h"
#include "../build/font_ttf.h"
#include "../build/logo_png.h"
#include "../build/browser_list_btn_png.h"
#include "../build/browser_folder_icon_f_png.h"

// osd
#include "../build/video_control_frame_bg_png.h"
#include "../build/video_control_time_played_line_bg_png.h"
#include "../build/video_player_time_played_line_n_01_png.h"
#include "../build/video_control_fastforward_btn_png.h"
#include "../build/video_control_frame_bg_png.h"
#include "../build/video_control_next_btn_png.h"
#include "../build/video_control_pause_btn_png.h"
#include "../build/video_control_play_btn_png.h"
#include "../build/video_control_previous_btn_png.h"
#include "../build/video_control_rewind_btn_png.h"
#include "../build/video_control_stop_btn_png.h"
#include "../build/video_control_time_played_line_bg_png.h"

#include "filebrowser.h"

#include "mplayer_func.h"

enum {
	MENU_BACK = -1,
	HOME_PAGE = 1,
	MENU_MPLAYER,
	BROWSE = 0x10,
	BROWSE_VIDEO,
	BROWSE_AUDIO,
	BROWSE_PICTURE,
	OSD = 0x20,
};

const XeColor white = XeColor{255, 255, 255, 255};

static GuiImage * bgImg = NULL;
static GuiWindow * mainWindow = NULL;
static GuiTrigger * trigA;

XenosSurface * logo = NULL;

GuiImage * home_left = NULL;
GuiImage * home_main_function_frame_bg = NULL;

GuiImage * video_osd_progress_bar_front = NULL;
GuiImage * video_osd_progress_bar_back = NULL;
GuiImage * video_osd_bg = NULL;

GuiText * video_osd_info_filename = NULL;
GuiText * video_osd_info_cur_time = NULL;
GuiText * video_osd_info_duration = NULL;

GuiImage * video_osd_play = NULL;
GuiImage * video_osd_pause = NULL;
GuiImage * video_osd_stop = NULL;
GuiImage * video_osd_next = NULL;
GuiImage * video_osd_prev = NULL;
GuiImage * video_osd_rewind = NULL;
GuiImage * video_osd_forward = NULL;

GuiImage * decoration_state = NULL;
GuiImage * decoration_keyicon = NULL;
GuiImage * decoration_keyicon_ex = NULL;
GuiImage * decoration_wrongkeyicon = NULL;

static char mplayer_filename[2048];

static int last_menu;

static int current_menu = HOME_PAGE;

#define GuiCreateButton(name,text,_png,_png_over) \
	GuiImageData image_data_ ## name(_png); \
	GuiImage image_##name(&image_data_##name); \
    GuiImageData image_data_ ## name ## _over(_png_over); \
    GuiImage image_##name##_over(&image_data_##name##_over); \
    GuiButton button_##name(image_data_##name.GetWidth(), image_data_##name.GetHeight()); \
    button_##name.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE); \
	GuiText txt_##name(text, 26, white); \
    txt_##name.SetWrap(true, image_data_##name.GetWidth() - 30); \
    button_##name.SetLabel(&txt_##name); \
    button_##name.SetImage(&image_##name); \
    button_##name.SetImageOver(&image_##name##_over); \
    button_##name.SetEffectGrow(); 

static void update() {
	UpdatePads();
	mainWindow->Draw();
	Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
	udelay(100);
}

/** to do **/
static void loadRessources() {
	// HOME PAGE


	//	<image image="image/home_left.png" x="0" y="0" w="522" h="720" bg="1"/>
	//	<image image="image/home_main_function_frame_bg.png" x="0" y="341" w="1280" h="148" bg="1"/>

	home_left = new GuiImage(new GuiImageData(home_left_png));
	home_main_function_frame_bg = new GuiImage(new GuiImageData(home_main_function_frame_bg_png));

	home_main_function_frame_bg->SetPosition(0, 341);

	// OSD
	video_osd_progress_bar_front = new GuiImage(new GuiImageData(video_player_time_played_line_n_01_png));
	video_osd_progress_bar_back = new GuiImage(new GuiImageData(video_control_time_played_line_bg_png));
	video_osd_bg = new GuiImage(new GuiImageData(video_control_frame_bg_png));

	//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
	//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
	//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

	//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
	//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

	XeColor blue;
	blue.a = 0xff;
	blue.r = 0x00;
	blue.g = 0x96;
	blue.b = 0xfa;

	XeColor white;
	white.lcol = 0xFFFFFFFF;

	video_osd_info_filename = new GuiText("info_filename", 22, white);
	video_osd_info_cur_time = new GuiText("info_cur_time", 18, white);
	video_osd_info_duration = new GuiText("info_duration", 18, blue);

	/** play state **/
	video_osd_play = new GuiImage(new GuiImageData(video_control_play_btn_png));
	video_osd_pause = new GuiImage(new GuiImageData(video_control_pause_btn_png));
	video_osd_stop = new GuiImage(new GuiImageData(video_control_stop_btn_png));
	video_osd_next = new GuiImage(new GuiImageData(video_control_next_btn_png));
	video_osd_prev = new GuiImage(new GuiImageData(video_control_previous_btn_png));
	video_osd_rewind = new GuiImage(new GuiImageData(video_control_rewind_btn_png));
	video_osd_forward = new GuiImage(new GuiImageData(video_control_fastforward_btn_png));
}

static void common_setup() {
	trigA = new GuiTrigger();

	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData * background = new GuiImageData(welcome_background_bg_png);
	mainWindow = new GuiWindow(screenwidth, screenheight);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	loadRessources();
}

extern "C" double playerGetElapsed();
extern "C" double playerGetDuration();
extern "C" const char * playerGetFilename();
extern "C" int playerGetStatus();

static void osd() {
	// <image image="image/video_control_frame_bg.png" x="149" y="597" w="983" h="73" disable="@@progress_disable" bg="1"/>
	video_osd_bg->SetPosition(149, 597);

	// <progress_bar name="video_time_bar" x="262" y="652" w="858" h="4" pb_back_img="image/video_control_time_played_line_bg.png" pb_front_img="image/Video_Player_time_played_line_n_01.png" pb_point_img="image/slideshow_player_time_played_line_mark.png" pb_delay="500" disable="@@progress_disable"/>
	video_osd_progress_bar_front->SetPosition(262, 652);
	//video_osd_progress_bar_back->SetPosition(262,652);
	video_osd_progress_bar_back->SetPosition(262, 652);

	//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
	//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
	//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

	XeColor blue;
	blue.a = 0xff;
	blue.r = 0x00;
	blue.g = 0x96;
	blue.b = 0xfa;

	XeColor white;
	white.lcol = 0xFFFFFFFF;

	GuiText info_filename("info_filename", 22, white);
	GuiText info_cur_time("info_cur_time", 18, white);
	GuiText info_duration("info_duration", 18, blue);

	info_filename.SetPosition(264, 621);
	info_cur_time.SetPosition(943, 625);
	info_duration.SetPosition(1023, 625);
	//	
	info_filename.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	info_cur_time.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	info_duration.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	// order
	mainWindow->Append(video_osd_bg);
	mainWindow->Append(video_osd_progress_bar_back);
	mainWindow->Append(video_osd_progress_bar_front);

	mainWindow->Append(&info_filename);
	mainWindow->Append(&info_cur_time);
	mainWindow->Append(&info_duration);

	int bar_width = video_osd_progress_bar_front->GetWidth();

	// change image width
	struct XenosSurface * img = video_osd_progress_bar_front->GetImage();

	info_filename.SetText(playerGetFilename());

	char duration[10];
	char cur_time[10];



	while (1) {
		float pourcents = (float) (playerGetElapsed()*100) / (float) playerGetDuration();
		float width = (float) bar_width * (pourcents / 100.0);
		img->width = width;

		//		printf("t%d\r\n", playerGetElapsed());
		//		printf("p%f\r\n", pourcents);
		//		printf("i%d\r\n", img->width);
		//
		//		sprintf(duration, "%d", playerGetDuration());
		//		sprintf(cur_time, "%d", playerGetElapsed());

		info_cur_time.SetText(cur_time);
		info_duration.SetText(duration);

		video_osd_progress_bar_front->SetImage(img, img->width, img->height);

		update();
	}

	img->width = bar_width;

	mainWindow->Remove(video_osd_bg);
	mainWindow->Remove(video_osd_progress_bar_front);
	mainWindow->Remove(video_osd_progress_bar_back);


	mainWindow->Remove(&info_filename);
	mainWindow->Remove(&info_cur_time);
	mainWindow->Remove(&info_duration);
}

static int osd_duration_bar_width;
static int osd_show = 0;
static char osd_duration[10];
static char osd_cur_time[10];

extern "C" void mplayer_osd_open() {
	if (osd_show == 0) {
		// <image image="image/video_control_frame_bg.png" x="149" y="597" w="983" h="73" disable="@@progress_disable" bg="1"/>
		video_osd_bg->SetPosition(149, 597);

		// <progress_bar name="video_time_bar" x="262" y="652" w="858" h="4" pb_back_img="image/video_control_time_played_line_bg.png" pb_front_img="image/Video_Player_time_played_line_n_01.png" pb_point_img="image/slideshow_player_time_played_line_mark.png" pb_delay="500" disable="@@progress_disable"/>
		video_osd_progress_bar_front->SetPosition(262, 652);
		//video_osd_progress_bar_back->SetPosition(262,652);
		video_osd_progress_bar_back->SetPosition(262, 652);

		//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
		//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
		//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

		video_osd_info_filename->SetPosition(264, 621);
		video_osd_info_cur_time->SetPosition(943, 625);
		video_osd_info_duration->SetPosition(1023, 625);
		//	
		video_osd_info_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_info_cur_time->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_info_duration->SetAlignment(ALIGN_LEFT, ALIGN_TOP);


		//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
		//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

		video_osd_play->SetPosition(202, 613);
		video_osd_pause->SetPosition(202, 613);
		video_osd_stop->SetPosition(202, 613);
		video_osd_next->SetPosition(202, 613);
		video_osd_prev->SetPosition(202, 613);
		video_osd_rewind->SetPosition(202, 613);
		video_osd_forward->SetPosition(202, 613);

		video_osd_play->SetVisible(false);
		video_osd_pause->SetVisible(false);
		video_osd_stop->SetVisible(false);
		video_osd_next->SetVisible(false);
		video_osd_prev->SetVisible(false);
		video_osd_rewind->SetVisible(false);
		video_osd_forward->SetVisible(false);

		// order
		mainWindow->Append(video_osd_bg);
		mainWindow->Append(video_osd_progress_bar_back);
		mainWindow->Append(video_osd_progress_bar_front);

		mainWindow->Append(video_osd_info_filename);
		mainWindow->Append(video_osd_info_cur_time);
		mainWindow->Append(video_osd_info_duration);

		// 		
		mainWindow->Append(video_osd_play);
		mainWindow->Append(video_osd_pause);
		mainWindow->Append(video_osd_stop);
		mainWindow->Append(video_osd_next);
		mainWindow->Append(video_osd_prev);
		mainWindow->Append(video_osd_rewind);
		mainWindow->Append(video_osd_forward);

		// remove bg
		mainWindow->Remove(bgImg);

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		osd_duration_bar_width = img->width;
	}
	osd_show = 1;
}

extern "C" void mplayer_osd_close() {
	if (osd_show) {

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		img->width = osd_duration_bar_width;

		mainWindow->Remove(video_osd_bg);
		mainWindow->Remove(video_osd_progress_bar_front);
		mainWindow->Remove(video_osd_progress_bar_back);


		mainWindow->Remove(video_osd_info_filename);
		mainWindow->Remove(video_osd_info_cur_time);
		mainWindow->Remove(video_osd_info_duration);

		mainWindow->Remove(video_osd_play);
		mainWindow->Remove(video_osd_pause);
		mainWindow->Remove(video_osd_stop);
		mainWindow->Remove(video_osd_next);
		mainWindow->Remove(video_osd_prev);
		mainWindow->Remove(video_osd_rewind);
		mainWindow->Remove(video_osd_forward);

		// reapply bg
		mainWindow->Append(bgImg);
	}
	osd_show = 0;
}

extern "C" void mplayer_osd_draw() {
	if (osd_show) {

		double duration = playerGetDuration();
		double elapsed = playerGetElapsed();

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		float pourcents = (float) (elapsed * 100) / (float) duration;
		float width = (float) osd_duration_bar_width * (pourcents / 100.0);
		img->width = width;

		//		printf("t%d\r\n", playerGetElapsed());
		//		printf("p%f\r\n", pourcents);
		//		printf("i%d\r\n", img->width);
		{
			div_t hrmin, minsec;
			minsec = div(duration, 60);
			hrmin = div(minsec.quot, 60);

			sprintf(osd_duration, "%d:%02d:%02d", hrmin.quot, hrmin.rem, minsec.rem);
		}
		{
			div_t hrmin, minsec;
			minsec = div(elapsed, 60);
			hrmin = div(minsec.quot, 60);

			sprintf(osd_cur_time, "%d:%02d:%02d", hrmin.quot, hrmin.rem, minsec.rem);
		}

		video_osd_info_cur_time->SetText(osd_cur_time);
		video_osd_info_duration->SetText(osd_duration);

		video_osd_info_filename->SetText(playerGetFilename());

		video_osd_progress_bar_front->SetImage(img, img->width, img->height);

		video_osd_play->SetVisible(false);
		video_osd_pause->SetVisible(false);
		video_osd_stop->SetVisible(false);		
		video_osd_rewind->SetVisible(false);
		video_osd_forward->SetVisible(false);
		video_osd_next->SetVisible(false);
		video_osd_prev->SetVisible(false);

		switch (playerGetStatus()) {
			case 1:
				video_osd_play->SetVisible(true);
				break;
			case 2:
				video_osd_pause->SetVisible(true);
				break;
			case 3:
				video_osd_stop->SetVisible(true);
				break;
			case 4:
				video_osd_rewind->SetVisible(true);
				break;
			case 5:
				video_osd_forward->SetVisible(true);
				break;
			case 6:
				video_osd_next->SetVisible(true);
				break;
			case 7:
				video_osd_prev->SetVisible(true);
				break;

			default:
				break;
		}


		UpdatePads();
		Menu_Frame();
		mainWindow->Draw();
		//Menu_Render();
		for (int i = 0; i < 4; i++) {
			mainWindow->Update(&userInput[i]);
		}
	}
}

static void Browser(const char * title, const char * root) {
	BrowseDevice("", root);
	GuiFileBrowser gui_browser(980, 500, (u8*) browser_list_btn_png, (u8*) browser_folder_icon_f_png);

	gui_browser.SetPosition(150, 180);
	gui_browser.SetFontSize(20);
	gui_browser.SetSelectedFontSize(26);
	gui_browser.SetPageSize(10);

	//mainWindow->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
	mainWindow->Append(&gui_browser);

	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);

	GuiText menuBtnTxt("B", 18, white);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);
	
	
	GuiText browserTxt(title, 64, white);
	browserTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browserTxt.SetPosition(64, 45);
	browserTxt.SetEffectGrow();

	mainWindow->Append(&browserTxt);
	
	last_menu = current_menu;

	while (current_menu == last_menu) {

		// update file browser based on arrow xenon_buttons
		// set MENU_EXIT if A xenon_button pressed on a file
		for (int i = 0; i < gui_browser.getPageSize(); i++) {
			if (gui_browser.fileList[i]->GetState() == STATE_CLICKED) {
				gui_browser.fileList[i]->ResetState();
				// check corresponding browser entry
				if (browserList[browser.selIndex].isdir) {
					if (BrowserChangeFolder()) {
						gui_browser.ResetState();
						gui_browser.fileList[0]->SetState(STATE_SELECTED);
						gui_browser.TriggerUpdate();
					} else {
						break;
					}
				} else {
					current_menu = MENU_MPLAYER;
					//mplayer_main(NULL,NULL);

					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);

					CleanupPath(mplayer_filename);

					ShutoffRumble();
					gui_browser.ResetState();
				}
			}
		}

		if (menuBtn.GetState() == STATE_CLICKED) {
			current_menu = MENU_BACK;
		}

		update();
	}

	mainWindow->Remove(&browserTxt);
	mainWindow->Remove(&gui_browser);
	mainWindow->Remove(&menuBtn);
}

static void HomePage() {
	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);

	// <wgt_mlist name="VERT_MLIST" image="" x="305" y="140" w="200" h="440" fontsize="26" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="vert" itemcount="4" extra_index="3" extra_len="0"/>

	GuiList * list_v = new GuiList(200, 440);
	list_v->SetPosition(305, 140);
	list_v->SetCount(4);
	list_v->SetCenter(3);

	GuiCreateButton(video, "", home_video_sm_icon_n_png, home_video_sm_icon_n_png);
	GuiCreateButton(audio, "", home_music_sm_icon_n_png, home_music_sm_icon_n_png);
	GuiCreateButton(photo, "", home_photo_sm_icon_n_png, home_photo_sm_icon_n_png);
	GuiCreateButton(param, "", home_settings_sm_icon_n_png, home_settings_sm_icon_n_png);

	list_v->Append(&button_video);
	list_v->Append(&button_audio);
	list_v->Append(&button_photo);
	list_v->Append(&button_param);

	mainWindow->Append(list_v);

	// <wgt_mlist name="HORI_MLIST" x="560" y="360" w="465" h="110" fontsize="20" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="hori" itemcount="3"/>

	GuiList * list_h = new GuiList(465, 110, 'H');
	list_h->SetPosition(560, 360);
	list_h->SetCount(3);
	list_h->SetCenter(2);

	GuiCreateButton(usb, "", home_hdd_sub_icon_n_png, home_hdd_sub_icon_n_png);
	GuiCreateButton(more1, "", home_nohdd_sub_icon_n_png, home_nohdd_sub_icon_n_png);
	GuiCreateButton(more2, "", home_nohdd_sub_icon_n_png, home_nohdd_sub_icon_n_png);
	GuiCreateButton(more3, "", home_nohdd_sub_icon_n_png, home_nohdd_sub_icon_n_png);
	GuiCreateButton(more4, "", home_nohdd_sub_icon_n_png, home_nohdd_sub_icon_n_png);

	list_h->Append(&button_usb);
	list_h->Append(&button_more1);
	list_h->Append(&button_more2);
	list_h->Append(&button_more3);
	list_h->Append(&button_more4);

	list_h->SetFocus(1);

	GuiImageData *selector_png = new GuiImageData(home_horizontal_select_bar_png);
	GuiImage * selector = new GuiImage(selector_png);
	selector->SetPosition(715, 350);
	list_h->SetSelector(selector);

	mainWindow->Append(list_h);


	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText menuBtnTxt("A", 18, white);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);

	while (current_menu == HOME_PAGE) {
		if (menuBtn.GetState() == STATE_CLICKED) {
			current_menu = BROWSE_VIDEO;
		}
		update();
	}

	mainWindow->Remove(&menuBtn);
	mainWindow->Remove(list_h);
	mainWindow->Remove(list_v);
	mainWindow->Remove(home_left);
	mainWindow->Remove(home_main_function_frame_bg);

	delete list_v;
	delete list_h;

	delete selector_png;
	delete selector;
}

static void do_mplayer(char * filename) {
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char * argv[] = {
			"mplayer.xenon",
			//"-really-quiet",
			//"-demux mkv",
			"-menu",
			//"-menu-startup",
			//"-lavdopts","skiploopfilter=all:threads=2",
			"-lavdopts", "skiploopfilter=all:threads=5",
			//"uda:/mplayer/loop.mov","-loop","0",
			//"-lavdopts","skiploopfilter=all",
			//"-novideo",
			//"-vo","null",
			//"-vc","ffmpeg4",
			//"-v",
			//"-nosound",
			//"-vfm","xvid",
			//"-ao","null",
			//"-dvd-device","uda:/dvd/THE_SMURFS/","dvd://1",
			//			"uda:/video.avi",
			//			"uda:/dbz.avi",
			//"uda:/video.m2ts",
			//"uda:/video2.mp4",
			//"dvd://1"
			//"uda:/trailer.mkv",
			//"uda:/lockout-tlr1_h1080p.mov"
			filename,
		};
		mplayer_need_init = 0;
		int argc = sizeof (argv) / sizeof (char *);

		mplayer_main(argc, argv);
		// will never be here !!!
	} else {
		mplayer_load(filename);
		mplayer_return_to_player();
	}
}

void MenuMplayer() {
	//sprintf(foldername, "%s/", browser.dir);	
	printf("filename:%s\r\n", mplayer_filename);		
	do_mplayer(mplayer_filename);
}

static int need_gui = 1;

static void gui_loop() {
	while (need_gui) {
		//last_menu = current_menu;
		if (current_menu == OSD) {
			osd();
		}
		if (current_menu == HOME_PAGE) {
			HomePage();
		} else if (current_menu == BROWSE_VIDEO) {
			Browser("Videos","uda:/");
		}else if (current_menu == BROWSE_AUDIO) {
			Browser("Audio","uda:/");
		}
		else if (current_menu == MENU_MPLAYER) {
			MenuMplayer();
		} else if (current_menu == MENU_BACK) {
			current_menu = HOME_PAGE;
		}
	}
}

int main(int argc, char** argv) {
	XeColor white;
	white.lcol = 0xFFFFFFFF;

	xenon_make_it_faster(XENON_SPEED_FULL);

	init_mplayer();

	// Init Video
	InitVideo();

	/** loool **/
	logo = loadPNGFromMemory((unsigned char*) logo_png);

	Xe_SetClearColor(g_pVideoDevice, 0xFFFFFFFF);

	Menu_DrawImg(0, 0, 1280, 720, logo, 0, 1, 1, 0xff);

	Menu_Render();

	// Init Freetype
	InitFreeType((u8*) font_ttf, font_ttf_size);

	// usb	
	usb_init();
	xenon_ata_init();
	usb_do_poll();
	fatInitDefault();

	// pads
	SetupPads();

	ChangeFontSize(26);

	common_setup();

	current_menu = HOME_PAGE;

	//current_menu = OSD;
	while (1) {
		// never exit !!	
		need_gui = 1;
		gui_loop();
		TR;
	}

	return (EXIT_SUCCESS);
}

/**
 * return to gui - doesn't exit mplayer process
 */
extern "C" void mplayer_return_to_gui() {
	need_gui = 1;
	
	// make sur to leave the gui
	mplayer_osd_close();
	
	current_menu = last_menu;
	
	gui_loop();	
}

/**
 * return to mplayer
 */
extern "C" void mplayer_return_to_player() {
	need_gui = 0;
}