#include <metahook.h>
#include <glew.h>
#include "cvardef.h"
#include "plugins.h"
#include "exportfuncs.h"
#include "entity_types.h"
#include "parsemsg.h"
#include "gl_capture.h"
#include <SteamAPIBridge.h>

cl_enginefunc_t gEngfuncs;
engine_studio_api_t IEngineStudio;
r_studio_interface_t **gpStudioInterface;

char g_szServerName[256] = {0};

namespace
{
	bool g_bLoggedSteamUserUnavailable = false;
	bool g_bLoggedSteamScreenshotsWriteUnavailable = false;

	void Con_Printf_Once(bool& bAlreadyPrinted, const char* pszMessage)
	{
		if (bAlreadyPrinted)
			return;

		bAlreadyPrinted = true;
		gEngfuncs.Con_Printf(pszMessage);
	}
}

static hook_t* g_pPresentHook = NULL;

class CSnapshotManager
{
public:
	static void __cdecl OnSnapshotCallback(void*, uint32_t screenshot, int32_t result);
	void TryRegisterCallback();
	void UnregisterCallback();
	SteamBridgeContextHandle Context() const { return m_Context; }

private:
	bool m_bCallbackRegistered = false;
	SteamBridgeContextHandle m_Context = nullptr;
};

CSnapshotManager g_SnapshotManager;

void CSnapshotManager::TryRegisterCallback()
{
	if (m_bCallbackRegistered)
		return;

	if (!m_Context)
		m_Context = SteamBridge_CreateContext();
	if ((SteamBridge_GetCapabilities(m_Context) & (SB_CAP_SCREENSHOTS | SB_CAP_USER)) != (SB_CAP_SCREENSHOTS | SB_CAP_USER))
		return;

	m_bCallbackRegistered = SteamBridge_SubscribeScreenshots(m_Context, OnSnapshotCallback, this) == SB_OK;
}

void CSnapshotManager::UnregisterCallback()
{
	SteamBridge_DestroyContext(m_Context);
	m_Context = nullptr;
	m_bCallbackRegistered = false;
}

void __cdecl CSnapshotManager::OnSnapshotCallback(void* opaque, uint32_t screenshot, int32_t result)
{
	auto context = static_cast<CSnapshotManager*>(opaque)->Context();

	if (result == SB_STEAM_RESULT_OK)
	{
		SteamBridge_SetScreenshotLocation(context, screenshot, g_szServerName);

		uint64_t steamId = 0;
		if (SteamBridge_GetSteamID(context, &steamId) == SB_OK)
		{
			SteamBridge_TagScreenshotUser(context, screenshot, steamId);
		}
		else
		{
			Con_Printf_Once(g_bLoggedSteamUserUnavailable, "[SteamScreenshots] Cannot tag user because Steam user interface is unavailable.\n");
		}

		gEngfuncs.Con_Printf("[SteamScreenshots] Snapshot saved.\n");
	}
	else if (result == SB_STEAM_RESULT_IO_FAILURE)
	{
		gEngfuncs.Con_Printf("[SteamScreenshots] Cannot save snapshot. Got an IO error.\n");
	}
	else
	{
		gEngfuncs.Con_Printf("[SteamScreenshots] Cannot save snapshot. Got an unknown error.\n");
	}
}

void HUD_Shutdown(void)
{
	ShutdownSteamBridge();

	GL_ShutdownCapture();

	gExportfuncs.HUD_Shutdown();
}

void ShutdownSteamBridge()
{
	g_SnapshotManager.UnregisterCallback();
}

void ScreenshotCallback(void* pBuf, size_t cbBufSize, int width, int height)
{
	uint32_t screenshot = 0;
	if (SteamBridge_WriteScreenshot(g_SnapshotManager.Context(), pBuf, static_cast<uint32_t>(cbBufSize), width, height, &screenshot) != SB_OK)
	{
		Con_Printf_Once(g_bLoggedSteamScreenshotsWriteUnavailable, "[SteamScreenshots] Cannot write screenshot because Steam screenshots interface is unavailable.\n");
	}
}

void __cdecl SDL_GL_SwapWindow(void* window)
{
	GL_CapturePendingBeforeSwap(ScreenshotCallback);

	// With Renderer enabled, its final blit runs before this engine flip.
	gPrivateFuncs.SDL_GL_SwapWindow(window);
}

void __cdecl Sys_VID_FlipScreen(void)
{
	// With Renderer enabled, its final blit runs before this engine flip.
	GL_CapturePendingBeforeSwap(ScreenshotCallback);

	gPrivateFuncs.Sys_VID_FlipScreen();
}

void InstallSDL2Hook()
{
	if (g_pPresentHook)
		return;

	auto engine = g_pMetaHookAPI->GetEngineModule();
	if (engine && g_pMetaHookAPI->ModuleHasImportEx(engine, "SDL2.dll", "SDL_GL_SwapWindow"))
	{
		g_pPresentHook = g_pMetaHookAPI->IATHook(engine, "SDL2.dll", "SDL_GL_SwapWindow", SDL_GL_SwapWindow, (void**)&gPrivateFuncs.SDL_GL_SwapWindow);
	}

	if (!g_pPresentHook)
		gEngfuncs.Con_Printf("[SteamScreenshots] SDL hook unavailable; keeping the engine snapshot command.\n");
}

void InstallFlipScreenHook()
{
	if (g_pPresentHook)
		return;

	if (gPrivateFuncs.VID_FlipScreen)
		return;

	if (g_pInterface->MetaHookAPIVersion < 109)
	{
		gEngfuncs.Con_Printf("[SteamScreenshots] Gamedata API unavailable; keeping the engine snapshot command.\n");
		return;
	}

	PVOID flipScreenSlot = NULL;
	auto status = g_pMetaHookAPI->ResolveGameSymbol(g_pMetaHookAPI->GetEngineBase(),
		"VID_FlipScreen", MH_GAMESYMBOL_KIND_GLOBAL, &flipScreenSlot);
	if (status != MH_GAMESYMBOL_OK)
	{
		gEngfuncs.Con_Printf("[SteamScreenshots] VID_FlipScreen unavailable (%s); keeping the engine snapshot command.\n",
			g_pMetaHookAPI->GetGameSymbolStatusString(status));
		return;
	}

	// The gamedata GLOBAL is the engine's own flip function-pointer slot: GL_EndRendering
	// calls through it and Sys_InitGame writes it once at startup, so swapping the pointer
	// redirects every flip without patching code.
	if (flipScreenSlot)
	{
		gPrivateFuncs.VID_FlipScreen = (void(__cdecl**)(void))flipScreenSlot;
		gPrivateFuncs.Sys_VID_FlipScreen = *gPrivateFuncs.VID_FlipScreen;
		(*gPrivateFuncs.VID_FlipScreen) = Sys_VID_FlipScreen;
	}

	if (!gPrivateFuncs.VID_FlipScreen)
		gEngfuncs.Con_Printf("[SteamScreenshots] VID_FlipScreen hook unavailable; keeping the engine snapshot command.\n");
}

void UninstallPresentHook()
{
	if (g_pPresentHook)
	{
		g_pMetaHookAPI->UnHook(g_pPresentHook);
		g_pPresentHook = NULL;
		gPrivateFuncs.SDL_GL_SwapWindow = NULL;
	}

	if (gPrivateFuncs.VID_FlipScreen)
	{
		(*gPrivateFuncs.VID_FlipScreen) = gPrivateFuncs.Sys_VID_FlipScreen;
		gPrivateFuncs.VID_FlipScreen = NULL;
		gPrivateFuncs.Sys_VID_FlipScreen = NULL;
	}
}

bool IsPresentHookInstalled()
{
	return g_pPresentHook != NULL || gPrivateFuncs.VID_FlipScreen != NULL;
}

void VID_Snapshot_f(void)
{
	GL_RequestCapture();
}

void HUD_Frame(double time)
{
	gExportfuncs.HUD_Frame(time);

	g_SnapshotManager.TryRegisterCallback();

	auto levelname = gEngfuncs.pfnGetLevelName();

	if (!levelname || !levelname[0])
	{
		g_szServerName[0] = 0;
	}

	GL_QueryAsyncCapture(ScreenshotCallback);
}

pfnUserMsgHook m_pfnServerName = NULL;

int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char *szServerName = READ_STRING();

	strncpy(g_szServerName, szServerName, sizeof(g_szServerName) - 1);
	g_szServerName[sizeof(g_szServerName) - 1] = 0;

	if (m_pfnServerName)
		return m_pfnServerName(pszName, iSize, pbuf);

	return 0;
}

void IN_ActivateMouse(void)
{
	gExportfuncs.IN_ActivateMouse();

	static bool init = false;

	if (!init)
	{
		InstallSDL2Hook();
		InstallFlipScreenHook();

		if (IsPresentHookInstalled() && GL_InitCapture())
		{
			//cmd "snapshot" is registered after HUD_Init
			g_pMetaHookAPI->HookCmd("snapshot", VID_Snapshot_f);
		}

		m_pfnServerName = HOOK_MESSAGE(ServerName);

		g_SnapshotManager.TryRegisterCallback();

		init = true;
	}
}
