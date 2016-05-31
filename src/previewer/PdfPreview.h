/* Copyright 2015 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

#define SZ_PDF_PREVIEW_CLSID    L"{3D3B1846-CC43-42AE-BFF9-D914083C2BA3}"
#ifdef BUILD_XPS_PREVIEW
#define SZ_XPS_PREVIEW_CLSID    L"{D427A82C-6545-4FBE-8E87-030EDB3BE46D}"
#endif
#ifdef BUILD_DJVU_PREVIEW
#define SZ_DJVU_PREVIEW_CLSID   L"{6689D0D4-1E9C-400A-8BCA-FA6C56B2C3B5}"
#endif
#ifdef BUILD_EPUB_PREVIEW
#define SZ_EPUB_PREVIEW_CLSID   L"{80C4E4B1-2B0F-40D5-95AF-BE7B57FEA4F9}"
#endif
#ifdef BUILD_FB2_PREVIEW
#define SZ_FB2_PREVIEW_CLSID    L"{D5878036-E863-403E-A62C-7B9C7453336A}"
#endif
#ifdef BUILD_MOBI_PREVIEW
#define SZ_MOBI_PREVIEW_CLSID   L"{42CA907E-BDF5-4A75-994A-E1AEC8A10954}"
#endif
#if defined(BUILD_CBZ_PREVIEW) || defined(BUILD_CBR_PREVIEW) || defined(BUILD_CB7_PREVIEW) || defined(BUILD_CBT_PREVIEW)
#define SZ_CBX_PREVIEW_CLSID    L"{C29D3E2B-8FF6-4033-A4E8-54221D859D74}"
#endif
#ifdef BUILD_TGA_PREVIEW
#define SZ_TGA_PREVIEW_CLSID    L"{CB1D63A6-FE5E-4DED-BEA5-3F6AF1A70D08}"
#endif
