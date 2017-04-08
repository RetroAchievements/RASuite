//-----------------------------------------------------------------------------
// MEKA - video.c
// Video / Miscellaneous - Code
//-----------------------------------------------------------------------------

#include "shared.h"
#include "blit.h"
#include "blitintf.h"
#include "capture.h"
#include "debugger.h"
#include "fskipper.h"
#include "glasses.h"
#include "inputs_i.h"
#include "inputs_t.h"
#include "palette.h"
#include "skin_bg.h"
#include "skin_fx.h"
#include "vdp.h"
#include "video.h"

//RA
#include "RA_Interface.h"

void RenderAchievementOverlays();

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

t_video	g_video;

t_video_driver	g_video_drivers[] =
{
#ifdef ARCH_WIN32
	{ "directx",	"DirectX",		0,					},	// Allegro for Win32 wants a zero here because it is "default".
#endif
	{ "opengl",		"OpenGL",		ALLEGRO_OPENGL,		},
	{ "opengl30",	"OpenGL 3.0",	ALLEGRO_OPENGL_3_0, },
	{ NULL, }
};

t_video_driver*	g_video_driver_default = &g_video_drivers[0];


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    Video_Init()
{
	//Video_CreateVideoBuffers();
	Video_EnumerateDisplayModes();

    // Clear variables
    g_video.res_x						= 0;
    g_video.res_y						= 0;
    g_video.clear_requests				= 0;
	g_video.game_area_x1				= 0;
	g_video.game_area_x2				= 0;
	g_video.game_area_y1				= 0;
	g_video.game_area_y2				= 0;
    g_video.driver						= 1;
	g_video.refresh_rate_requested		= 0;
	g_video.display_mode_current_index	= 0;
	fs_out								= NULL;
}

void Video_DestroyVideoBuffers()
{
	if (Screenbuffer_IsLocked())
		Screenbuffer_ReleaseLock();

	if (screenbuffer_1)
		al_destroy_bitmap(screenbuffer_1);
	if (screenbuffer_2)
		al_destroy_bitmap(screenbuffer_2);

    screenbuffer_1 = screenbuffer_2 = NULL;

    GUI_DestroyVideoBuffers();
    Data_DestroyVideoBuffers();
    Blit_DestroyVideoBuffers();
	SkinFx_DestroyVideoBuffers();
}

void Video_CreateVideoBuffers()
{
    Video_DestroyVideoBuffers();

    // Allocate buffers
	al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP | ALLEGRO_NO_PRESERVE_TEXTURE);
	al_set_new_bitmap_format(g_configuration.video_game_format_request);
    screenbuffer_1      = al_create_bitmap(MAX_RES_X + 32, MAX_RES_Y + 32);
    screenbuffer_2      = al_create_bitmap(MAX_RES_X + 32, MAX_RES_Y + 32);
    screenbuffer        = screenbuffer_1;
    screenbuffer_next   = screenbuffer_2;

	// Retrieve actual video format. This will be used to compute color values.
	g_screenbuffer_format = al_get_bitmap_format(screenbuffer_1);

	Screenbuffer_AcquireLock();

	Blit_CreateVideoBuffers();
	Data_CreateVideoBuffers();
	SkinFx_CreateVideoBuffers();
	if (g_env.state == MEKA_STATE_GUI)
		GUI_SetupNewVideoMode();
}

static int Video_ChangeVideoMode(t_video_driver* driver, int w, int h, bool fullscreen, int refresh_rate, bool fatal)
{
    // Attempt to avoid unnecessary resolution change (on blitter change)
    static struct
    {
        t_video_driver* driver;
        int w, h;
		int fullscreen;
        int refresh_rate;
    } previous_mode = { NULL, -1, -1, -1, -1 };
    if (driver == previous_mode.driver && w == previous_mode.w && h == previous_mode.h && (int)fullscreen == previous_mode.fullscreen && refresh_rate == previous_mode.refresh_rate)
    {
        Video_GameMode_UpdateBounds();
		if (g_env.state == MEKA_STATE_GUI)
			gui_relayout_all();
        return (MEKA_ERR_OK);
    }

    previous_mode.driver = driver;
    previous_mode.w = w;
    previous_mode.h = h;
	previous_mode.fullscreen = fullscreen;
    previous_mode.refresh_rate = refresh_rate;

    Video_DestroyVideoBuffers();

    // Set new mode
	if (g_display != NULL)
	{
#ifdef ARCH_WIN32
		// Allegro is missing keyboard events when there's no display, so as a workaround we clear the key states.
		Inputs_KeyClearAllState();
#endif
		al_unregister_event_source(g_display_event_queue, al_get_display_event_source(g_display));
		al_destroy_display(g_display);
		g_display = NULL;
	}

	// Create new display
	int display_flags = driver->flags;
	if (fullscreen)
		display_flags |= ALLEGRO_FULLSCREEN;
	else
		display_flags |= ALLEGRO_WINDOWED;
	al_set_new_display_flags(display_flags);
	al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);
//	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
	al_set_new_display_refresh_rate(g_configuration.video_mode_gui_refresh_rate);
	g_display = al_create_display(w, h);

	if (!g_display)
    {
        if (fatal)
            Quit_Msg(Msg_Get(MSG_Error_Video_Mode), w, h);
        Msg(MSGT_USER, Msg_Get(MSG_Error_Video_Mode), w, h);
        return (MEKA_ERR_FAIL);
    }

	al_register_event_source(g_display_event_queue, al_get_display_event_source(g_display));

    g_video.res_x = w;
    g_video.res_y = h;
    g_video.refresh_rate_requested = refresh_rate;
	Video_GameMode_UpdateBounds();

	// Window title
    al_set_window_title(g_display, Msg_Get(MSG_Window_Title));

	// Recreate all video buffers
	Video_CreateVideoBuffers();

    return (MEKA_ERR_OK);
}

void    Video_GameMode_UpdateBounds()
{
    // Compute game area position to be centered on the screen
	Blit_Fullscreen_UpdateBounds();
}

void	Video_GameMode_ScreenPosToEmulatedPos(int screen_x, int screen_y, int* pemu_x, int* pemu_y, bool clamp)
{
	if (clamp)
	{
		const int rx = LinearRemapClamp(screen_x, g_video.game_area_x1, g_video.game_area_x2, 0, g_driver->x_res);
		const int ry = LinearRemapClamp(screen_y, g_video.game_area_y1, g_video.game_area_y2, 0, g_driver->y_res);
		*pemu_x = rx;
		*pemu_y = ry;
	}
	else
	{
		const int rx = LinearRemap(screen_x, g_video.game_area_x1, g_video.game_area_x2, 0, g_driver->x_res);
		const int ry = LinearRemap(screen_y, g_video.game_area_y1, g_video.game_area_y2, 0, g_driver->y_res);
		*pemu_x = rx;
		*pemu_y = ry;
	}
}

void	Video_GameMode_EmulatedPosToScreenPos(int emu_x, int emu_y, int* pscreen_x, int* pscreen_y, bool clamp)
{
	if (clamp)
	{
		const int rx = LinearRemapClamp(emu_x, 0, g_driver->x_res, g_video.game_area_x1, g_video.game_area_x2);
		const int ry = LinearRemapClamp(emu_y, 0, g_driver->y_res, g_video.game_area_y1, g_video.game_area_y2);
		*pscreen_x = rx;
		*pscreen_y = ry;
	}
	else
	{
		const int rx = LinearRemap(emu_x, 0, g_driver->x_res, g_video.game_area_x1, g_video.game_area_x2);
		const int ry = LinearRemap(emu_y, 0, g_driver->y_res, g_video.game_area_y1, g_video.game_area_y2);
		*pscreen_x = rx;
		*pscreen_y = ry;
	}
}

void	Video_GameMode_GetScreenCenterPos(int* pscreen_x, int* pscreen_y)
{
	*pscreen_x = (g_video.game_area_x1 + g_video.game_area_x2) >> 1;
	*pscreen_y = (g_video.game_area_y1 + g_video.game_area_y2) >> 1;
}

void    Video_ClearScreenBackBuffer()
{
    // Note: actual clearing will be done in blit.c
    g_video.clear_requests = 3;
}

void	Video_EnumerateDisplayModes()
{
	std::vector<t_video_mode>& display_modes = g_video.display_modes;
	display_modes.clear();

	const int modes = al_get_num_display_modes();
	for (int i = 0; i != modes; i++)
	{
		ALLEGRO_DISPLAY_MODE al_display_mode;
		if (al_get_display_mode(i, &al_display_mode))
		{
			//Msg(MSGT_DEBUG, "Display Mode: %dx%d @ %d Hz, format %d", display_mode.width, display_mode.height, display_mode.refresh_rate, display_mode.format);
			display_modes.resize(display_modes.size()+1);

			t_video_mode* video_mode = &display_modes.back();
			video_mode->w = al_display_mode.width;
			video_mode->h = al_display_mode.height;
			//video_mode->color_format = al_display_mode.format;
			video_mode->refresh_rate = al_display_mode.refresh_rate;
		}
	}

	// filter out color_format duplicates because we ignore it for now
	for (size_t i = 0; i < display_modes.size(); i++)
	{
		t_video_mode* video_mode = &display_modes[i];
		for (size_t j = i + 1; j < display_modes.size(); j++)
		{
			t_video_mode* video_mode_2 = &display_modes[j];
			if (video_mode->w == video_mode_2->w && video_mode->h == video_mode_2->h)
			{
				 if (video_mode->refresh_rate == video_mode_2->refresh_rate)
				 {
					 display_modes.erase(display_modes.begin()+j);
					 j--;
				 }
			}
		}
	}

	// find mode closest to current setting
	g_video.display_mode_current_index = 0;
	int closest_index = -1;
	float closest_d2 = FLT_MAX;
	for (size_t i = 0; i < display_modes.size(); i++)
	{
		t_video_mode* video_mode = &display_modes[i];

		int dx = (video_mode->w - g_configuration.video_mode_gui_res_x);
		int dy = (video_mode->h - g_configuration.video_mode_gui_res_y);
		float d2 = dx*dx + dy*dy;

		if (closest_d2 > d2)
		{
			if (closest_index != -1)
				if (video_mode->refresh_rate != 0 && g_configuration.video_mode_gui_refresh_rate != 0 && video_mode->refresh_rate != g_configuration.video_mode_gui_refresh_rate)
					continue;
			closest_d2 = d2;
			closest_index = i;
		}
	}
	if (closest_index != -1)
		g_video.display_mode_current_index = closest_index;
}

void    Video_Setup_State()
{
    switch (g_env.state)
    {
    case MEKA_STATE_SHUTDOWN:
        {
            Video_DestroyVideoBuffers();
            if (g_display)
    			al_destroy_display(g_display);
			g_display = NULL;
            break;
        }
    case MEKA_STATE_GAME: // FullScreen mode ----------------------------
        {
			const int refresh_rate = g_configuration.video_mode_gui_refresh_rate;
			const int game_res_x = g_configuration.video_mode_gui_res_x;
			const int game_res_y = g_configuration.video_mode_gui_res_y;
			const bool game_fullscreen = g_configuration.video_fullscreen;

            if (Video_ChangeVideoMode(g_configuration.video_driver, game_res_x, game_res_y, game_fullscreen, refresh_rate, FALSE) != MEKA_ERR_OK)
            {
                g_env.state = MEKA_STATE_GUI;
                Video_Setup_State();
                Msg(MSGT_USER, "%s", Msg_Get(MSG_Error_Video_Mode_Back_To_GUI));
                return;
            }
            fs_out = al_get_backbuffer(g_display);
			Palette_Emulation_Reload();
			Video_ClearScreenBackBuffer();
        }
        break;
    case MEKA_STATE_GUI:
        {
			const int refresh_rate = g_configuration.video_mode_gui_refresh_rate;
			const int gui_res_x = g_configuration.video_mode_gui_res_x;
			const int gui_res_y = g_configuration.video_mode_gui_res_y;
			Video_ChangeVideoMode(g_configuration.video_driver, gui_res_x, gui_res_y, g_configuration.video_fullscreen, refresh_rate, TRUE);
			if (opt.GUI_Inited)
				gui_redraw_everything_now_once();
        }
        break;
    }
	Inputs_Peripheral_Change_Update();
}

void    Screen_Save_to_Next_Buffer()
{
	al_set_target_bitmap(screenbuffer_next);
	al_draw_bitmap(screenbuffer, 0, 0, 0);
}

void    Screen_Restore_from_Next_Buffer()
{
	al_set_target_bitmap(screenbuffer);
	al_draw_bitmap(screenbuffer_next, 0, 0, 0);
}

void	Screenbuffer_AcquireLock()
{
	assert(g_screenbuffer_locked_region == NULL && g_screenbuffer_locked_buffer == NULL);
	g_screenbuffer_locked_region = al_lock_bitmap(screenbuffer, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
    g_screenbuffer_locked_buffer = screenbuffer;
}

void	Screenbuffer_ReleaseLock()
{
	assert(g_screenbuffer_locked_region != NULL && g_screenbuffer_locked_buffer != NULL);
    assert(g_screenbuffer_locked_buffer == screenbuffer);
	al_unlock_bitmap(screenbuffer);
	g_screenbuffer_locked_region = NULL;
    g_screenbuffer_locked_buffer = NULL;
}

bool	Screenbuffer_IsLocked()
{
	return g_screenbuffer_locked_region != NULL;
}

void	Video_UpdateEvents()
{
	ALLEGRO_EVENT key_event;
	while (al_get_next_event(g_display_event_queue, &key_event))
	{
		switch (key_event.type)
		{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			if (g_env.state == MEKA_STATE_INIT || g_env.state == MEKA_STATE_SHUTDOWN)
				break;
			opt.Force_Quit = TRUE;
			break;
		case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
			//Msg(MSGT_USER, "ALLEGRO_EVENT_DISPLAY_SWITCH_IN");
			//if (g_env.state == MEKA_STATE_INIT || g_env.state == MEKA_STATE_SHUTDOWN)
			//	return;
			//// clear_to_color (screen, BORDER_COLOR);
			//Video_Clear();
			//Sound_Playback_Resume();
			Inputs_Sources_ClearOutOfFocus();
			break;
		case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
			//Msg(MSGT_USER, "ALLEGRO_EVENT_DISPLAY_SWITCH_OUT");
			//if (g_env.state == MEKA_STATE_INIT || g_env.state == MEKA_STATE_SHUTDOWN)
			//	break;
			//Sound_Playback_Mute();
			Inputs_Sources_ClearOutOfFocus();
			break;
		}
	}
}

// This is called when line == tsms.VDP_Line_End
void    Video_RefreshScreen()
{

	PROFILE_STEP("Video_RefreshScreen()");

	Screenbuffer_ReleaseLock();
	PROFILE_STEP("Screenbuffer_ReleaseLock()");

	Video_UpdateEvents();
	PROFILE_STEP("Video_UpdateEvents()");

	// 3-D glasses emulation cancel out one render out of two
	if (Glasses.Enabled && Glasses_Must_Skip_Frame())
		Screen_Restore_from_Next_Buffer();

    if (fskipper.Show_Current_Frame)
    {
		Capture_Update();

        if (g_machine_pause_requests > 0)
            Machine_Pause();

		gui_update_mouse();
        if (g_env.state == MEKA_STATE_GUI)
        {
            gui_update();
			PROFILE_STEP("gui_update()");

            // Check if we're switching GUI off now
            if (g_env.state != MEKA_STATE_GUI)
            {
				Screenbuffer_AcquireLock();
                return;
            }

            gui_draw();
			PROFILE_STEP("gui_draw()");

			Blit_GUI();
			PROFILE_STEP("Blit_GUI()");
			RenderAchievementOverlays();
        }

        if (g_env.state == MEKA_STATE_GAME)
        {
            // Show current FPS
            if (fskipper.FPS_Display)
            {
                char buf[16];
                sprintf(buf, "%.1f FPS", fskipper.FPS);
				int x, y;
                if (g_driver->id == DRV_GG) { x = 48; y = 24; } else { x = 8; y = 6; }
				al_set_target_bitmap(screenbuffer);
                Font_Print(FONTID_MEDIUM, buf, x, y, COLOR_WHITE); // In white
                //g_gui_status.timeleft = 0; // Force disabling the current message because it is slow to display
            }

            // Blit emulated screen in fullscreen mode
            Blit_Fullscreen();

        }
		
        // Palette update after redraw
        Palette_UpdateAfterRedraw();

        // Clear keypress queue
        Inputs_KeyPressQueue_Clear();

    } // of: if (fskipper.Show_Current_Frame)

    // Draw next image in other buffer
    if (g_machine_flags & MACHINE_PAUSED)
    {
        Screen_Restore_from_Next_Buffer();
    }
    else
    {
        // Swap buffers
        ALLEGRO_BITMAP *tmp = screenbuffer;
        screenbuffer = screenbuffer_next;
        screenbuffer_next = tmp;
        // Msg(MSGT_DEBUG, "Swap buffer. screenbuffer=%d", screenbuffer==screenbuffer_1?1:2);

        // In debugging mode, copy previously rendered buffer to new one
        // This is so the user always see the current rendering taking place over the previous one
        #ifdef MEKA_Z80_DEBUGGER
            if (Debugger.active)
                Screen_Restore_from_Next_Buffer();
        #endif
    }
	
	//Screenbuffer_AcquireLock();
	//RenderAchievementOverlays(); //Simply cannot render on top of allegro without introducing flicker.
	//Screenbuffer_ReleaseLock();

    // Ask frame-skipper whether next frame should be drawn or not
    fskipper.Show_Current_Frame = Frame_Skipper();
	PROFILE_STEP("Frame_Skipper()");


	Screenbuffer_AcquireLock();
	PROFILE_STEP("Screenbuffer_AcquireLock()");


}


ALLEGRO_BITMAP *convert_hbitmap_to_bitmap(HBITMAP bitmap);
ALLEGRO_BITMAP *get_bitmap_from_dib(int bpp, int w, int h, BYTE *pixels);
#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

void RenderAchievementOverlays() {
	//#RA:
	//So alegro is finished flipping screenbuffers?
	//So we can draw the overlays now, right?
	//WARNING: Ugly Hack

	HWND MekaWND;
	HDC  hDC; //Direct context (HAVE to release after use)

	//Really need a proper Win32 Window to draw the overlay on
	MekaWND = al_get_win_window_handle(g_display); //This works, but Too much flicker.
	//MekaWND = ConsoleHWND(); // This works, but redraw is awful.
	hDC = GetDC(MekaWND);

	static int nOldTime = GetTickCount(); //Time in ms I presume

	int nDelta = GetTickCount() - nOldTime;
	nOldTime = GetTickCount();


	RECT rcSize;
	//int display_width = al_get_display_width(g_display);
	//int display_height = al_get_display_height(g_display);

	//SetRect(&rcSize, 0, 0, display_width, display_height);
	GetClientRect(MekaWND, &rcSize);


	ControllerInput input; // This has to be passed to the overlay

	//Just taking input from standard keyboard because Meka controller input is super wierd
	//Note: Not eating allegro key inputs (FALSE)
	input.m_bUpPressed = Inputs_KeyPressed(ALLEGRO_KEY_UP, FALSE);
	input.m_bDownPressed = Inputs_KeyPressed(ALLEGRO_KEY_DOWN, FALSE);
	input.m_bLeftPressed = Inputs_KeyPressed(ALLEGRO_KEY_LEFT, FALSE);
	input.m_bRightPressed = Inputs_KeyPressed(ALLEGRO_KEY_RIGHT, FALSE);
	input.m_bCancelPressed = Inputs_KeyPressed(ALLEGRO_KEY_B, FALSE); //
	input.m_bConfirmPressed = Inputs_KeyPressed(ALLEGRO_KEY_A, FALSE); // I think these match the interface
	input.m_bQuitPressed = Inputs_KeyPressed(ALLEGRO_KEY_ENTER, FALSE);

	bool meka_paused = (g_machine_flags & MACHINE_PAUSED);
	bool meka_fullscreen = FALSE; // just going to set this


/* No 	*/
								//Need to prevent flicker with a custom double buffer on top of allegro's buffers.
								//This hack just became a massive pain
								  // Create an off-screen DC for double-buffering
								static HDC			hdcMem = NULL;
								static HBITMAP      hbmMem = NULL;
								static HANDLE		hbmOld = NULL; //Really need this?
								static HBRUSH		hbrBkGnd;
								static COLORREF		blk = GetSysColor(COLOR_WINDOW);// GetSysColor(COLOR_BACKGROUND);
																					//static COLORREF		wht = ((DWORD)0x00ffffff); //GetSysColor(COLOR_WINDOW);// GetSysColor(COLOR_BACKGROUND);


								static int display_width = -1; 
								static int display_height = -1; 


								
								static int bpp = 32; // Bits per pixel
								static int stride = -1;

								static BITMAPV5HEADER bmh;

								static BYTE *pBits = NULL; // needs to be initalised or debugger complains.




								static ALLEGRO_BITMAP *bitmap = NULL;
								//int c = al_get_bitmap_format(bitmap);
								//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);
								static ALLEGRO_LOCKED_REGION *lock = NULL;
								//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);


								if (!hdcMem) {


									display_width = al_get_display_width(g_display);
									display_height = al_get_display_height(g_display);

									stride = (display_width * (bpp / 8));

									memset(&bmh, 0, sizeof(BITMAPV5HEADER));
									bmh.bV5Size = sizeof(BITMAPV5HEADER);
									bmh.bV5Width = display_width;
									bmh.bV5Height = display_height;
									bmh.bV5Planes = 1;
									bmh.bV5BitCount = 32;
									bmh.bV5Compression = BI_RGB;
									bmh.bV5AlphaMask = 0xFF000000;
									bmh.bV5RedMask = 0x00FF0000;
									bmh.bV5GreenMask = 0x0000FF00;
									bmh.bV5BlueMask = 0x000000FF;


									// Create an off-screen DC for double-buffering
									hdcMem = CreateCompatibleDC(hDC);
									hbmMem = CreateDIBSection(hDC, (BITMAPINFO *)&bmh, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);

									int a = al_get_new_bitmap_format();
									int dpf = al_get_display_format(g_display);
									//int b = ALLEGRO_PIXEL_FORMAT_ARGB_8888;

									//al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
									//hbmMem = CreateCompatibleBitmap(hDC, display_width, display_height); //allegro had better not change the display size, etc (Meka doesn't seem to do this)
									//hbmMem = Bitmap(display_width, display_height, Format32bppPArgb);

									//hbrBkGnd = CreateSolidBrush((DWORD)0x00fe01fd); //magic magenta mask
									hbrBkGnd = CreateSolidBrush((DWORD)0x00ff00ff); //magic magenta mask
									hbmOld = SelectObject(hdcMem, hbmMem);
									SetBkColor(hdcMem, blk);
									al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
									bitmap = al_create_bitmap(display_width, display_height);




								}
								pBits[0] = 0x11;
								pBits[1] = 0x32;
								pBits[2] = 0x13;
								pBits[3] = 0xfd;
								//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_XRGB_8888, ALLEGRO_LOCK_WRITEONLY);

								//al_unlock_bitmap(bitmap);

								//Paint to offscreen direct context buffer with transparency
								//SetBkColor(hdcMem, GetSysColor(COLOR_WINDOW));
								//SetBkColor(hdcMem, blk);
								//FillRect(hdcMem, &rcSize, hbrBkGnd);
								//SetBkColor(hdcMem, GetSysColor(COLOR_WINDOW));
								//SetBkColor(hdcMem, blk);
								//SetBkMode(hdcMem, TRANSPARENT);

								{
									FillRect(hdcMem, &rcSize, hbrBkGnd);
									//RA_UpdateRenderOverlay(hDC, &input, ((float)nDelta / 1000.0f), &rcSize, meka_fullscreen, meka_paused);
									RA_UpdateRenderOverlay(hdcMem, &input, ((float)nDelta / 1000.0f), &rcSize, meka_fullscreen, meka_paused);
								}
								int aghfyf = 3;

	//								  al_destroy_bitmap(bitmap);

	/*

														   `  ```
													   ` `````--:-.`
													`..`-::::+oossso:``
												  ...:+/oyyssyhhyhhhyo//-``
												--.:oshhhdddhhhhhhhhhhyyyso+/:.
											  ```:+yshhdhdddddhhhhhhhdhhhhhhyso+`
											``./oyyyhhhhdddddhhhyyhhhdddhhdddhhys.
										   .--syhhhhhdhdddddhhhhyhhhdddddddddddddy.
										   `:shhhdhhddddhhhhhhhhyhhhhhdddddmdddddd/
										  :.osyhhhddhhddddhhhdddhhhhddddddmmdmmmmmh/
										 .-/oosyhhhhhhhhhyyyhdddddddmmdmmmmmdmmmmmdd:
										 `.+oosyyhhhhhhhhyyhddddmmmmmmmmmmmmmNmNmmmdy
										  -+ooosyhhhhhhddhhhddddmmmmmdddddmmNNmNmmmmd`
										 .:+oosyyhdhhddmdddddmdddmmmmmmhhddNNNmmNNNNNs`
										 /ssyhdmmmdddhhyyyyhdmddmmmddmmmhddmNNNNNNNNNNo
										  ---.yNNNNmmmdhyysshdddddhysydhshdmmNNNNNNNNNh +Nmhs:`
											   smmNmhdhhysssyyyyyhhyyhhyyhhmmmNNNNNmmmmmNNNNMMNy`
											   .syyhhhhsssyyyyyhhhhhhysyhmmddmmmmmdmmNNNNNMMMMMMN
											   -osssyyyyyyhhhhhhhhdhhhddddddddddddhodNNNNMMMMMMMM
											   -syyyyyyyhhhhhhhhdddddddhhhdhhhhhyohNNNNNMMMMMMMNm
											   -ssssshhhhhhhhhhhddddhhdddddhhhhoyNNNNNNMMMMMMNNNN
											   `shdmmdhhhhhhhhhdddddhhddddhhdsyNNNNNNMMMMMNNNNNNN
												.-.../yyhhhhhhhddddhhdddddddhNNNNNNNMMMNNNNNNNNNN
													   `.+hdhhhhhdddhdddmmNNMMNNNNNNMMNNNNNNNNNNN
														`syhhhhddddddmmmNNMMNNNNNMMMMNNNNNNNNNNNN
														 -+yhhhdddmmmNNNMMMNNNNNMMMNNNNNNNNNNNNNN
														   -oyhdmmmmy++hNNNNNNNMMMNNNNNNNNNNNNNNN
															./os/:-.   `omNNNMMMNNNNNNNNNNNNNNNNN
																	 `-ymNNNNMMNNNNNNNNNNNNNNNNNN
						"A funeral! You let Dougal do a funeral!!"
	*/

								pBits[0] = 0x11;
								pBits[1] = 0x32;
								pBits[2] = 0x13;
								pBits[3] = 0x00;
								uint8_t *dst;
								{
									int x,y;// , y;
									int pitch;
									//int col;
									//int b, g, r;
									//BYTE *src;
									//ALLEGRO_BITMAP *bitmap;
									//ALLEGRO_LOCKED_REGION *lock;
									//uint8_t *dst;
									int w = display_width;
									int h = display_height;
									al_set_target_bitmap(bitmap);

									for (int bit = 0; bit < w*h; bit++) {
										pBits[4 * bit + 0] = 0;
										pBits[4 * bit + 1] = 0;
										pBits[4 * bit + 2] = 255;
										pBits[4 * bit + 3] = 0;

									}
									for (x = 0; x < 50; x++) {
										for (y = 0; y < 50; y++) {

											al_put_pixel(x, y, al_map_rgba(0, 0, 255, 0));
										}
									}
									pitch = w * BYTES_PER_PIXEL(bpp);
									pitch = (pitch + 3) & ~3;  /* align on dword */

															   //al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
															   //bitmap = al_create_bitmap(w, h);

									lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);
									//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_RGB_888, ALLEGRO_LOCK_WRITEONLY);
									//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);

									dst = (uint8_t *)lock->data;
									
									for (y = 0; y < h/2; ++y)
									{

										//memcpy(dst, pBits + y * pitch, w * BYTES_PER_PIXEL(bpp));
										dst += lock->pitch;
									}
									//memcpy(dst , pBits, w*h*BYTES_PER_PIXEL(bpp));
									al_unlock_bitmap(bitmap);
									//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);
									//al_unlock_bitmap(bitmap);


								}
								ALLEGRO_COLOR p1 = al_get_pixel(bitmap, 0 * display_width, 0 * display_height);
								ALLEGRO_COLOR p2 = al_get_pixel(bitmap, 1 * display_width -1, 0 * display_height);
								ALLEGRO_COLOR p3 = al_get_pixel(bitmap, 0 * display_width, 1 * display_height -1);
								ALLEGRO_COLOR p4 = al_get_pixel(bitmap, 1 * display_width -1 , 1 * display_height -1);



								int a = 1;

								al_set_target_backbuffer(g_display);
								//
								// Blt the changes to the screen DC.
								//BitBlt(hDC, rcSize.left, rcSize.top, 	rcSize.right - rcSize.left, rcSize.bottom - rcSize.top, hdcMem,	0, 0, SRCCOPY);


								//al_convert_mask_to_alpha(bitmap, al_map_rgb(253, 1, 254));
								//al_convert_mask_to_alpha(bitmap, al_map_rgb(255, 0, 255));
								//ALLEGRO_BITMAP *bitmap2 = al_clone_bitmap(bitmap);


								int op; int src; int dt;
								al_get_blender(&op, &src, &dt); // get current state of blender
								al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

								//al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);

								al_draw_bitmap(bitmap, 0, 0, ALLEGRO_FLIP_VERTICAL);

								al_set_blender(op, src, dt); // put this back the way we found it or its white out time

								p1 = al_get_pixel(bitmap, 0 * display_width, 0 * display_height);
								//al_destroy_bitmap(bitmap2);
								//static ALLEGRO_BITMAP* al_hbmMem;
								//al_hbmMem = convert_hbitmap_to_bitmap(hbmMem);
								//al_hbmMem = get_bitmap_from_dib(bpp, display_width, display_height, pBits);
								//al_convert_mask_to_alpha(al_hbmMem, al_map_rgb(253, 1, 254));
								//al_draw_bitmap(al_hbmMem, 0, 0, ALLEGRO_FLIP_VERTICAL);

								//al_hbmMem = al_create_bitmap(display_width, display_height);
								//al_convert_mask_to_alpha(al_hbmMem, al_map_rgb(255, 255, 255));
								//al_draw_bitmap(al_hbmMem, 0, 0, 0);

								//ALLEGRO_BITMAP *al_hbmMem2 = al_clone_bitmap(al_hbmMem);
								//al_convert_mask_to_alpha(al_hbmMem, al_map_rgb(0, 255, 0));
								//al_draw_bitmap(al_hbmMem, 0, 0, 0);
								//al_draw_bitmap(al_hbmMem, 0, 0, ALLEGRO_FLIP_VERTICAL);
								//al_draw_tinted_bitmap(al_hbmMem, al_map_rgba_f(1,1,1,0.5), 0, 0, 0);
								//al_flip_display();
								//al_destroy_bitmap(al_hbmMem);
								//al_destroy_bitmap(al_hbmMem2);

								  //Screenbuffer_AcquireLock();
								  //BitBlt(hDC, 0, 0, display_width, display_height, hdcMem, 0, 0, SRCCOPY); // "A funeral! You let Dougal do a funeral!!"
								  //Screenbuffer_ReleaseLock();

/*End No */
	//al_flip_display();
	//al_wait_for_vsync();
/*	char meka_currDir[2048];
	
	InvalidateRect(MekaWND, &rcSize, FALSE);
	GetCurrentDirectory(2048, meka_currDir); // "where'd you get the multithreaded code, Ted?"
	RA_UpdateRenderOverlay(hDC, &input, ((float)nDelta / 1000.0f), &rcSize, meka_fullscreen, meka_paused);
	SetCurrentDirectory(meka_currDir); // "Cowboys Ted! They're a bunch of cowboys!"
	*/
	
	ReleaseDC(MekaWND, hDC);
	
	
}

t_video_driver*	VideoDriver_FindByName(const char* name)
{
	t_video_driver* driver = &g_video_drivers[0];
	while (driver->name)
	{
		if (stricmp(name, driver->name) == 0)
			return driver;
		driver++;
	}

	// Silently return default
	return g_video_driver_default;
}


//-----------------------------------------------------------------------------








#include <Windows.h>

#include <allegro5/allegro.h>

#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

/* get_bitmap_from_dib:
*  Creates an Allegro BITMAP from a Windows device-independent bitmap (DIB).
*/
static ALLEGRO_BITMAP *get_bitmap_from_dib(int bpp, int w, int h, BYTE *pixels)
{
	int x, y;
	int pitch;
	int col;
	int b, g, r;
	BYTE *src;
	ALLEGRO_BITMAP *bitmap;
	ALLEGRO_LOCKED_REGION *lock;
	uint8_t *dst;

	pitch = w * BYTES_PER_PIXEL(bpp);
	pitch = (pitch + 3) & ~3;  /* align on dword */

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
	bitmap = al_create_bitmap(w, h);

	lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_XRGB_8888, ALLEGRO_LOCK_WRITEONLY);
	//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_RGB_888, ALLEGRO_LOCK_WRITEONLY);
	//lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);


	dst = (uint8_t *)lock->data;

	for (y = 0; y < h; ++y)
	{
		memcpy(dst, pixels + y * pitch, w * BYTES_PER_PIXEL(bpp));
		dst += lock->pitch;
	}

	al_unlock_bitmap(bitmap);

	return bitmap;
}

/* get_dib_from_hbitmap:
*  Creates a Windows device-independent bitmap (DIB) from a Windows BITMAP.
*  You have to free the memory allocated by this function.
*/
static BYTE *get_dib_from_hbitmap(int bpp, HBITMAP hbitmap)
{
	BITMAPINFOHEADER bi;
	BITMAPINFO *binfo;
	HDC hdc;
	HPALETTE hpal, holdpal;
	int col;
	BITMAP bm;
	int pitch;
	BYTE *pixels;
	BYTE *ptr;
	int x, y;

	if (!hbitmap)
		return NULL;

	if (bpp == 15)
		bpp = 16;

	if (!GetObject(hbitmap, sizeof(bm), (LPSTR)& bm))
		return NULL;

	pitch = bm.bmWidth * BYTES_PER_PIXEL(bpp);
	pitch = (pitch + 3) & ~3;  /* align on dword */

	pixels = (BYTE *)al_malloc(bm.bmHeight * pitch);
	if (!pixels)
		return NULL;

	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biBitCount = bpp;
	bi.biPlanes = 1;
	bi.biWidth = bm.bmWidth;
	bi.biHeight = -abs(bm.bmHeight);
	bi.biClrUsed = 256;
	bi.biCompression = BI_RGB;

	binfo = (BITMAPINFO *)al_malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
	binfo->bmiHeader = bi;

	hdc = GetDC(NULL);

	GetDIBits(hdc, hbitmap, 0, bm.bmHeight, pixels, binfo, DIB_RGB_COLORS);

	ptr = pixels;

	al_free(binfo);


	ReleaseDC(NULL, hdc);

	return pixels;
}

/* convert_hbitmap_to_bitmap:
*  Converts a Windows BITMAP to an Allegro BITMAP.
*/
ALLEGRO_BITMAP *convert_hbitmap_to_bitmap(HBITMAP bitmap)
{
	BYTE *pixels;
	ALLEGRO_BITMAP *bmp;
	BITMAP bm;
	int bpp;

	if (!GetObject(bitmap, sizeof(bm), (LPSTR)& bm))
		return NULL;

	if (bm.bmBitsPixel == 8) {
		/* ask windows to save truecolor image, then convert to our format */
		bpp = 24;
	}
	else
		bpp = bm.bmBitsPixel;

	/* get the DIB first */
	pixels = get_dib_from_hbitmap(bpp, bitmap);

	/* now that we have the DIB, convert it to a BITMAP */
	bmp = get_bitmap_from_dib(bpp, bm.bmWidth, bm.bmHeight, pixels);

	al_free(pixels);

	return bmp;
}

ALLEGRO_BITMAP *convert_hdc_to_bitmap(HDC dc)
{
	ALLEGRO_BITMAP *bg;
	const int w = GetDeviceCaps(dc, HORZRES);
	const int h = GetDeviceCaps(dc, VERTRES);

	HDC hmemdc = CreateCompatibleDC(dc);
	HBITMAP hbmp = CreateCompatibleBitmap(dc, w, h);
	HBITMAP holdbmp = (HBITMAP)SelectObject(hmemdc, hbmp);

	StretchBlt(hmemdc, 0, 0, w, h, dc, 0, 0, w, h, SRCCOPY);
	SelectObject(hmemdc, holdbmp);

	bg = convert_hbitmap_to_bitmap(hbmp);

	DeleteObject(hbmp);
	DeleteDC(hmemdc);

	return bg;
}
