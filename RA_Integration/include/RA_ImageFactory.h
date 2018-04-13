#pragma once

#include <wincodec.h>
#include <WTypes.h>

#include "RA_Defs.h"

//////////////////////////////////////////////////////////////////////////
//	Image Factory

struct IWICBitmapSource;
struct IWICImagingFactory;

struct UserImageFactoryVars
{
	CComPtr<IWICBitmapSource> m_pOriginalBitmapSource{ null };
	CComPtr<IWICImagingFactory> m_pIWICFactory{ null };
};


BOOL InitializeUserImageFactory( HINSTANCE hInst );
HBITMAP LoadOrFetchBadge( const std::string& sBadgeURI, const RASize& nSZ = RA_BADGE_PX );
HBITMAP LoadOrFetchUserPic( const std::string& sUser, const RASize& nSZ = RA_USERPIC_PX );
HBITMAP LoadLocalPNG( const std::string& sPath, const RASize& nSZ );

extern UserImageFactoryVars g_UserImageFactoryInst;

extern void DrawImage( HDC hDC, HBITMAP hBitmap, int nX, int nY, int nW, int nH );
extern void DrawImageTiled( HDC hDC, HBITMAP hBitmap, RECT& rcSource, RECT& rcDest );
extern void DrawImageStretched( HDC hDC, HBITMAP hBitmap, RECT& rcSource, RECT& rcDest );
