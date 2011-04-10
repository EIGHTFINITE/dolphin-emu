// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/


// IMPORTANT: UI etc should modify g_Config. Graphics code should read g_ActiveConfig.
// The reason for this is to get rid of race conditions etc when the configuration
// changes in the middle of a frame. This is done by copying g_Config to g_ActiveConfig
// at the start of every frame. Noone should ever change members of g_ActiveConfig 
// directly.

#ifndef _VIDEO_CONFIG_H_
#define _VIDEO_CONFIG_H_

#include "Common.h"
#include "VideoCommon.h"

#include <vector>
#include <string>

// Log in two categories, and save three other options in the same byte
#define CONF_LOG			1
#define CONF_PRIMLOG		2
#define CONF_SAVETEXTURES	4
#define CONF_SAVETARGETS	8
#define CONF_SAVESHADERS	16

enum MultisampleMode {
	MULTISAMPLE_OFF,
	MULTISAMPLE_2X,
	MULTISAMPLE_4X,
	MULTISAMPLE_8X,
	MULTISAMPLE_CSAA_8X,
	MULTISAMPLE_CSAA_8XQ,
	MULTISAMPLE_CSAA_16X,
	MULTISAMPLE_CSAA_16XQ,
};

enum AspectMode {
	ASPECT_AUTO = 0,
	ASPECT_FORCE_16_9 = 1,
	ASPECT_FORCE_4_3 = 2,
	ASPECT_STRETCH = 3,
};

class IniFile;

// NEVER inherit from this class.
struct VideoConfig
{

private:
	// According to new structure-design this member MUST BE private
	void GameIniLoad(const char *ini_file);

public:
	VideoConfig();
	// You can choose what "INI snapshot" you wish to load...
	// GameIni is loaded over MainIni file only if 'fileCheck' argument is passed with success
	void Load(const char *main_ini_file, bool fileCheck = false, const char *game_ini_file = "");

	void VerifyValidity();
	void Save(const char *ini_file);
	void GameIniSave(const char* default_ini, const char* game_ini);
	void UpdateProjectionHack();

	// General
	bool bVSync;

	bool bRunning;
	bool bWidescreenHack;
	int iAspectRatio;
	bool bCrop;   // Aspect ratio controls.
	bool bUseXFB;
	bool bUseRealXFB; // joined to radio button

	// OpenCL/OpenMP
	bool bEnableOpenCL;
	bool bOMPDecoder;

	// Enhancements
	int iMultisampleMode;
	int iEFBScale;
	bool bForceFiltering;
	int iMaxAnisotropy;
	std::string sPostProcessingShader;

	// Information
	bool bShowFPS;
	bool bShowInputDisplay;
	bool bOverlayStats;
	bool bOverlayProjStats;
	bool bTexFmtOverlayEnable;
	bool bTexFmtOverlayCenter;
	bool bShowEFBCopyRegions;
	
	// Render
	bool bWireFrame;
	bool bDisableLighting;
	bool bDisableTexturing;
	bool bDstAlphaPass;
	bool bDisableFog;
	
	// Utility
	bool bDumpTextures;
	bool bHiresTextures;	
	bool bDumpEFBTarget;
	bool bDumpFrames;
	bool bUseFFV1;
	bool bFreeLook;
	bool bAnaglyphStereo;
	int iAnaglyphStereoSeparation;
	int iAnaglyphFocalAngle;
	bool b3DVision;
	
	
	// Hacks
	bool bEFBAccessEnable;
	bool bDlistCachingEnable;

	// EFB Copy options
	bool bEFBCopyEnable; // Whether copies are enabled, period
	bool bEFBCopyRAMEnable; // Original RAM copies
	bool bEFBCopyVirtualEnable; // High-res lookaside copies

	bool bEFBEmulateFormatChanges;
	bool bOSDHotKey;
	int iPhackvalue[4];
	std::string sPhackvalue[2];
	float fAspectRatioHackW, fAspectRatioHackH;
	bool bZTPSpeedHack; // The Legend of Zelda: Twilight Princess
	bool bEnablePixelLighting;
	bool bEnablePerPixelDepth;

	int iLog; // CONF_ bits
	int iSaveTargetId;
	
	//currently unused:
	int iCompileDLsLevel;
	bool bShowShaderErrors;

	// D3D only config, mostly to be merged into the above
	int iAdapter;

	// UI Controls state
	struct
	{
		// IMPORTANT: each member inside this struct MUST HAVE same name corresponding to data member
		bool bVSync;
		bool bWidescreenHack;
		bool iAspectRatio;
		bool bCrop;
		bool bUseXFB;
		bool bUseRealXFB;
		bool bEnableOpenCL;
		bool iMultisampleMode;
		bool iEFBScale;
		bool bForceFiltering;
		bool iMaxAnisotropy;
		bool sPostProcessingShader;
		bool bShowFPS;
		bool bShowInputDisplay;
		bool bOverlayStats;
		bool bOverlayProjStats;
		bool bTexFmtOverlayEnable;
		bool bTexFmtOverlayCenter;
		bool bShowEFBCopyRegions;
		bool bWireFrame;
		bool bDisableLighting;
		bool bDisableTexturing;
		bool bDstAlphaPass;
		bool bDisableFog;
		bool bDumpTextures;
		bool bHiresTextures;
		bool bDumpEFBTarget;
		bool bDumpFrames;
		bool bUseFFV1;
		bool bFreeLook;
		bool bAnaglyphStereo;
		bool b3DVision;
		bool iAnaglyphStereoSeparation;
		bool iAnaglyphFocalAngle;
		bool bEFBAccessEnable;
		bool bOMPDecoder;
		bool bDlistCachingEnable;
		bool bEFBCopyEnable;
		bool bEFBCopyRAMEnable;
		bool bEFBCopyVirtualEnable;
		bool bEFBEmulateFormatChanges;
		bool bOSDHotKey;
		bool bZTPSpeedHack;
		bool bEnablePixelLighting;
		bool bEnablePerPixelDepth;
		bool iLog;
		bool iSaveTargetId;
		bool iCompileDLsLevel;
		bool bShowShaderErrors;
		bool iAdapter;
	} UI_State;

	// Static config per API
	struct
	{
		API_TYPE APIType;

		std::vector<std::string> Adapters; // for D3D9 and D3D11
		std::vector<std::string> AAModes;
		std::vector<std::string> PPShaders; // post-processing shaders

		bool bUseRGBATextures; // used for D3D11 in TextureCache
		bool bSupports3DVision;
		bool bSupportsDualSourceBlend; // only supported by D3D11 and OpenGL
		bool bSupportsFormatReinterpretation;
		bool bSupportsPixelLighting;
	} backend_info;
};

extern VideoConfig g_Config;
extern VideoConfig g_ActiveConfig;

// Called every frame.
void UpdateActiveConfig();

void ComputeDrawRectangle(int backbuffer_width, int backbuffer_height, bool flip, TargetRectangle *rc);

#endif  // _VIDEO_CONFIG_H_
