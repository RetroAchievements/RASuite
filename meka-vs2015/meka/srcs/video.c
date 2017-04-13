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

int overlay_render_method = OVERLAY_RENDER_ALLEGRO;// default. Set here but read in by config.c 
bool disable_RA_overlay = false;
int overlay_frame_skip = 0;
int overlay_alternate_render_blit = 0;
//int overlay_bg_splits = 0;


void RenderAchievementOverlays();
void RenderAchievementOverlays_ALLEGRO_OVERLAY();
void RenderAchievementOverlays_WIN_LAYER();
void UpdateOverlay(HDC, RECT);
LRESULT CALLBACK RAWndProc(HWND, UINT, WPARAM, LPARAM);

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

//RA - Making these global for easier usage;
HWND layeredWnd;
HWND MekaWND;

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
//	al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST); actually v_sync
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

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


		RenderAchievementOverlays(); // al_flip_display() occurs here now

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
	

    // Ask frame-skipper whether next frame should be drawn or not
    fskipper.Show_Current_Frame = Frame_Skipper();
	PROFILE_STEP("Frame_Skipper()");


	Screenbuffer_AcquireLock();
	PROFILE_STEP("Screenbuffer_AcquireLock()");


}


//#RA:
#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

void RenderAchievementOverlays() {
	
	if (disable_RA_overlay) return;

	if (g_env.state == MEKA_STATE_SHUTDOWN) return; // should be here

	if (overlay_render_method == OVERLAY_RENDER_WIN_LAYER) {
		al_flip_display();
		RenderAchievementOverlays_WIN_LAYER();
	}
	else { //render allegro by default
		RenderAchievementOverlays_ALLEGRO_OVERLAY();
		al_flip_display();
		//PROFILE_STEP("al_flip_display");
	}

}

void RenderAchievementOverlays_WIN_LAYER() {
	//#RA:
	//So alegro is finished flipping screenbuffers?
	//So we can draw the overlays now, right?
	//WARNING: Ugly Hack
	MekaWND = al_get_win_window_handle(g_display);

	RECT rect;
	GetClientRect(MekaWND, &rect);

	char meka_currDir[2048];
	GetCurrentDirectory(2048, meka_currDir); // "where'd you get the multithreaded code, Ted?"

	// Initialize layered window
	if (layeredWnd == NULL)
	{
		//Set up window class
		WNDCLASSEX wndEx;
		memset(&wndEx, 0, sizeof(wndEx));
		wndEx.cbSize = sizeof(wndEx);
		wndEx.lpszClassName = "RA_WND_CLASS";
		wndEx.lpfnWndProc = RAWndProc;
		wndEx.hInstance = (HINSTANCE)GetWindowLong(MekaWND, GWL_HINSTANCE);
		int result = RegisterClassEx(&wndEx);

		// Create Window. WS_POPUP style is important so window displays without any borders or toolbar;
		layeredWnd = CreateWindowEx(
			(WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED),
			wndEx.lpszClassName, 
			"RAWnd", 
			(WS_POPUP),
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom, 
			MekaWND, NULL, wndEx.hInstance, NULL);

		//SetParent(MekaWND, layeredWnd);
		
		ShowWindow(layeredWnd, SW_SHOWNOACTIVATE);
	}
	else
	{
		// Set up buffer and back buffer
		HDC hdc = GetDC(MekaWND);

		static HDC hdcMem = NULL;
		static HBITMAP hBmp = NULL;
		static HBITMAP hBmpOld = NULL;
		if (!hdcMem) {
			hdcMem = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmp);
		}

		// Blits the MekaWND to the back buffer.
		BitBlt(hdcMem, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

		// Update RA stuff
		UpdateOverlay(hdcMem, rect);

		// Actually draw to the back buffer
		// Not familiar with BLENDFUNCTION, may not be needed.
		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_OVER;
		POINT ptSrc = { 0, 0 };
		SIZE sizeWnd = { rect.right, rect.bottom };
		UpdateLayeredWindow(layeredWnd, hdc, NULL, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

		// Get position of the client rect. (NOT the window rect)
		ClientToScreen(MekaWND, reinterpret_cast<POINT*>(&rect.left));
		ClientToScreen(MekaWND, reinterpret_cast<POINT*>(&rect.right));

		// Move layered window over MekaWND.
		SetWindowPos(layeredWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(MekaWND, layeredWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); // Don't think this line is necessary on most OS, but just safety net.

		//SelectObject(hdcMem, hBmpOld);
		//DeleteObject(hBmp);
		//DeleteDC(hdcMem);
		ReleaseDC(MekaWND, hdc);
	}

	SetCurrentDirectory(meka_currDir); // "Cowboys Ted! They're a bunch of cowboys!"
}	


LRESULT CALLBACK RAWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SIZE:
		return SIZE_RESTORED;
	case WM_NCHITTEST:
		return HTNOWHERE;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}


void RenderAchievementOverlays_ALLEGRO_OVERLAY() {

	static int display_width = -1;
	static int display_height = -1;
	static int bpp = 32; // Bits per pixel. Don't change this. The hack won't work without it
	static int stride = -1;

	HWND MekaWND = al_get_win_window_handle(g_display);
	HDC hDC = GetDC(MekaWND);
	RECT rcSize;
	GetClientRect(MekaWND, &rcSize);

	static HDC			hdcMem = NULL;
	static HBITMAP      hbmMem = NULL;
	static HANDLE		hbmOld = NULL;
	static HBRUSH		hbrBkGnd;

	static BITMAPV5HEADER bmh;
	static BYTE *pBits = NULL; // WINAPI pixel array. Needs to be initalised or debugger complains.
	static ALLEGRO_BITMAP *bitmap = NULL;
	static ALLEGRO_LOCKED_REGION *lock = NULL;

	//The color value which is set to transparent when we blit into allegro. All elements of the overlay which are this color will be made transparent.
	//Note: Currently set to modified magic magenta. 
	static DWORD mask = 0x00fe01fd;
	static DWORD win_mask = (mask << 16) & 0x00ff0000   //BYTE order shenanigans
		| (mask >> 16) & 0x000000ff | mask & 0x0000ff00;

	if (!hdcMem) {  //WARNING: Not currently checking or destroying this anywhere so if Meka goes to ACTUAL Alt+Enter Fullscreen mode we are hosed

		display_width = al_get_display_width(g_display);
		display_height = al_get_display_height(g_display);
		stride = (display_width * (bpp / 8));

		memset(&bmh, 0, sizeof(BITMAPV5HEADER));		bmh.bV5Size = sizeof(BITMAPV5HEADER);
		bmh.bV5Width = display_width;					bmh.bV5Height = display_height;
		bmh.bV5Planes = 1;								bmh.bV5BitCount = bpp;
		bmh.bV5Compression = BI_RGB;
		/*bmh.bV5AlphaMask = 0xFF000000;		bmh.bV5RedMask = 0x00FF0000;		bmh.bV5GreenMask = 0x0000FF00;		bmh.bV5BlueMask = 0x000000FF;*/ // Probably don't need these really

																																						// Create an off-screen WINAPI bitmap for RA_dll to draw to
		hdcMem = CreateCompatibleDC(hDC);
		hbmMem = CreateDIBSection(hDC, (BITMAPINFO *)&bmh, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
		hbrBkGnd = CreateSolidBrush((DWORD)win_mask); //magic magenta mask
		hbmOld = SelectObject(hdcMem, hbmMem);

		//Create ALLEGRO bitmap we must transfer the windows bit to and then draw in allegro
		//This format must be ARGB for this to work (Not unsetting here but meka seems to use XRGB internally anyway)
		al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
		bitmap = al_create_bitmap(display_width, display_height);

	} //End bitmaps creation code (Only run once at the moment)


    //First let RA Draw the pixels to the WINAPI buffer
	FillRect(hdcMem, &rcSize, hbrBkGnd);
	//RA_UpdateRenderOverlay(hdcMem, &input, ((float)nDelta / 1000.0f), &rcSize, meka_fullscreen, meka_paused);
	UpdateOverlay(hdcMem, rcSize);


	/* Now we do something very stupid and hacky
	....................................................
	......................```...........................
	...............```...:////-`........................
	...............-:+o++syyhyy+-.`.....................
	..........-../syyhdhyhdhhhhhyys+/:-`................
	.........`./sshddddddhhhhhhhhhhhhyso/...............
	......``-+yyyhhhdddddhhyyhhhddhhdddhys-.............
	......-/yhhhhhdhddddhhhyhhhdhdddddddddh.............
	.....`/shhddhhddhhhhhhhhyhhhhdddmmdmddm+............
	....:-osyhhddhhhdhhhhddddhhdddddmddmmmmdo...........
	...../oosyhhhhhhhyyhddddddmmmmmmmmmmmmmmd-..........
	.....+ooosyhhhhhhhhhdddmmmmmddddmmNNNNmmmo..........
	...`:+oosyhdhhddddddddmdmmmmmdhddNNNmNNNNd-.........
	...`osyydmmddddhyyyhdmdmmmddmmhhdmNNNNNNNNd`.`......
	....````hNNNmmddhysshddhhhyshhshdmmNNNNNNNm:oNNdy/`.
	.........sdmdhhhyssyyyyyhhyyhyyhdmmmmNNmmmNNNNNMMMNo
	........./ssyhhysyyyyyhhhhhhyhdmddddmmddymNNNMMMMMMM
	.........+yyyyyyyhhhhhhhdddddhhhdddddhssmNNNMMMMMMNm
	.........+ssyyhhhhhhhhhddddhhddddhhdssmNNNNMMMMMNNNN
	.........:ydmmdhhhhhhhhddddhhdddhdhsdMNNNNMMMMNNNNNN
	...........```/syhhhhhhdddhhddddddmMNNNNNMMNNNNNNNNN
	.................:hdhhhhdddddmmNNMMNNNNNMMNNNNNNNNNN
	................./shhhhdddmmNNNNMMNNNNMMMNNNNNNNNNNN
	..................`/syhddmmdhmNNNNNNMMMNNNNNNNNNNNNN
	....................:ohs++:`.`+mNNNMMMNNNNNNNNNNNNNN
	............................`+hNNNMMMNNNNNNNNNNNNNNN
	"A funeral! You let Dougal do a funeral!!"
	*/
	{
		//		int x, y;// , y;
		int pitch;
		int w = display_width;
		int h = display_height;
		uint8_t *dst;
		static DWORD *pPixels = (DWORD*) &(*pBits); //Represent BYTE array as array of 4 byte RGBA DWORD array

													//The alpha value of all the bits had better be zero after WINAPI is done or ... nothing will happen here.
		for (int pixel = 0; pixel < w*h; pixel++) {
			if (pPixels[pixel] != mask)   //there is probably some gosu way of doing this in one SIMD operation but I don't know.
				pPixels[pixel] += 0xff000000;
			//Is the system little endian or big endian? If colors are being inverted you know what is wrong now.
		}

		pitch = w * BYTES_PER_PIXEL(bpp);
		pitch = (pitch + 3) & ~3;  /* align on dword */

								   //must use ARGB format here. For the life of me I don't understand why this isn't RGBA. or ABGR. I think windows is internally returning 0xAARRGGBB corresponding to BGR. Never mind it works 
		lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);
		dst = (uint8_t *)lock->data;

		//Copy the WINAPI bitmap bits into the ALLEGRO bitmap's buffer.
		memcpy(dst, pBits, w*h*BYTES_PER_PIXEL(bpp));
		al_unlock_bitmap(bitmap);

	}							// "You have used three inches of sticky tape, God bless you"



								//[02:26] <+SiegeLord> Allegro assumes pre - multiplied alpha channel
								//[02:27] <+SiegeLord> While what you have uses the non - pre multiplied alpha channel
								//So we need to switch blender types for drawing transparent bitmaps
	int old_op; int old_src; int old_dst;
	al_get_blender(&old_op, &old_src, &old_dst); // get current state of blender
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	//finally we draw to the display backbuffer (remember al_flip_display() still needs to be called
	al_draw_bitmap(bitmap, 0, 0, ALLEGRO_FLIP_VERTICAL);
	al_set_blender(old_op, old_src, old_dst); // put this back the way we found it or its white out time

	ReleaseDC(MekaWND, hDC);
}


void UpdateOverlay(HDC hdc, RECT rect)
{


	static int nOldTime = GetTickCount(); //Time in ms I presume

	int nDelta;
	nDelta = GetTickCount() - nOldTime;
	nOldTime = GetTickCount();

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

	bool meka_paused; meka_paused = (g_machine_flags & MACHINE_PAUSED);
	bool meka_fullscreen; meka_fullscreen = FALSE; // just going to set this

	RA_UpdateRenderOverlay(hdc, &input, ((float)nDelta / 1000.0f), &rect, meka_fullscreen, meka_paused);
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

