#include "StdAfx.h"

#pragma unmanaged
#include <dinput.h>
#include <windowsx.h>
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"
//#include "SharedUtil.hpp"

MyGUI::Gui* gui = NULL;
MyGUI::DirectXPlatform* guiPlatform = NULL;

myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
myIDirect3D9*       gl_pmyIDirect3D9;

IDirect3D9* WINAPI ThisDirect3DCreate9(UINT SDKVersion);
BOOL WINAPI ThisGetCursorPos(LPPOINT lpPoint);

/*int WINAPI DllMain ( HINSTANCE hModule, DWORD dwReason, PVOID pvNothing )
{
    // Eventually destroy our GUI upon detach
	if ( dwReason == DLL_PROCESS_ATTACH && !gui )
	{
		DWORD dwOldProtect;
		LPVOID pD3DCreate = (LPVOID)0x6D64DC;
		VirtualProtect(pD3DCreate, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		*(DWORD*)pD3DCreate = (DWORD)ThisDirect3DCreate9;
		VirtualProtect(pD3DCreate, 4, dwOldProtect, &dwOldProtect);

		LPVOID pGFW = (LPVOID)0x6D634C;
		VirtualProtect(pGFW, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		*(DWORD*)pGFW = (DWORD)ThisGetCursorPos;
		VirtualProtect(pGFW, 4, dwOldProtect, &dwOldProtect);
	}

    /*if ( dwReason == DLL_PROCESS_DETACH && gui )
    {
        delete g_pGUI;
    }* /

    return TRUE;
}*/
//
void GUI_Init()
{
	return;
	LPVOID pD3DCreate = (LPVOID)0x6D64DC;
	*(DWORD*)pD3DCreate = (DWORD)ThisDirect3DCreate9;

	LPVOID pGFW = (LPVOID)0x6D634C;
	*(DWORD*)pGFW = (DWORD)ThisGetCursorPos;
}

POINT oldPoint;

BOOL InputGoesToGUI();

BOOL WINAPI ThisGetCursorPos(LPPOINT lpPoint)
{
	if (InputGoesToGUI())
	{
		//memcpy(lpPoint, &oldPoint, sizeof(oldPoint));
		//return -1;

		*(DWORD*)0x633A374 = 0;
		memset((void*)0x435555, 0x90, 5);
		//*(BYTE*)0x4293F0 = 0xC3;
	}
	else
	{
		*(BYTE*)0x435555 = 0xA3;
		*(DWORD*)0x435556 = 0x0633A374;
	}

	BOOL result = GetCursorPos(lpPoint);

	memcpy(&oldPoint, lpPoint, sizeof(oldPoint));

	return result;
}

IDirect3D9* WINAPI ThisDirect3DCreate9(UINT SDKVersion)
{
	// Request pointer from Original Dll. 
	IDirect3D9 *pIDirect3D9_orig = Direct3DCreate9(SDKVersion);

	// Create my IDirect3D8 object and store pointer to original object there.
	// note: the object will delete itself once Ref count is zero (similar to COM objects)
	gl_pmyIDirect3D9 = new myIDirect3D9(pIDirect3D9_orig);

	// Return pointer to hooking Object instead of "real one"
	return (gl_pmyIDirect3D9);
}

//extern "C" _declspec(dllexport)
WNDPROC wndProc;
bool guiGetsInput = false;

bool GetGUIInput() {
	return guiGetsInput;
}

void SetGUIInput(bool input)
{
	guiGetsInput = input;

	gui->setVisiblePointer(input);
}

BOOL InputGoesToGUI() {
	return /*TRUE || */guiGetsInput;
}

DWORD TranslateScanCodeToGUIKey ( DWORD dwCharacter )
{
	switch ( dwCharacter )
	{
	case VK_HOME:       return DIK_HOME;
	case VK_END:        return DIK_END;
	case VK_RETURN:     return DIK_RETURN;
	case VK_TAB:        return DIK_TAB;
	case VK_BACK:       return DIK_BACK;
	case VK_LEFT:       return DIK_LEFTARROW;
	case VK_RIGHT:      return DIK_RIGHTARROW;
	case VK_UP:         return DIK_UPARROW;
	case VK_DOWN:       return DIK_DOWNARROW;
	case VK_LSHIFT:     return DIK_LSHIFT;
	case VK_RSHIFT:     return DIK_RSHIFT;
	case VK_SHIFT:      return DIK_LSHIFT;
	case VK_CONTROL:    return DIK_LCONTROL;
	case VK_DELETE:     return DIK_DELETE;
	case 0x56:          return DIK_V;           // V
	case 0x43:          return DIK_C;           // C
	case 0x58:          return DIK_X;           // X
	case 0x41:          return DIK_A;           // A
	default:            return 0;
	}
}

void ToggleGlobalGUI(bool on);

BOOL ProcessGUIMessage ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// If we have the focus, we handle the message
	if ( InputGoesToGUI () )
	{
		switch ( uMsg )
		{
		/*case WM_MOUSEWHEEL:
			if ( GET_WHEEL_DELTA_WPARAM ( wParam ) > 0 )  
				gui->injectPress
			else
				pGUI->ProcessMouseInput ( CGUI_MI_MOUSEWHEEL, 0, NULL );
			return true;
		*/
		case WM_MOUSEMOVE:
			gui->injectMouseMove(LOWORD ( lParam ), HIWORD ( lParam ), 0);
			return true;

		case WM_LBUTTONDOWN:
			gui->injectMousePress(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Left);
			return true;

		case WM_RBUTTONDOWN:
			gui->injectMousePress(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Right);
			return true;

		case WM_MBUTTONDOWN:
			gui->injectMousePress(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Middle);
			return true;

		case WM_LBUTTONUP:
			gui->injectMouseRelease(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Left);
			return true;

		case WM_RBUTTONUP:
			gui->injectMouseRelease(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Right);
			return true;

		case WM_MBUTTONUP:
			gui->injectMouseRelease(LOWORD ( lParam ), HIWORD ( lParam ), MyGUI::MouseButton::Middle);
			return true;
		case WM_KEYDOWN:
			{
				if (wParam == VK_HOME)
				{
					ToggleGlobalGUI(false);
					return true;
				}

				DWORD dwTemp = TranslateScanCodeToGUIKey ( wParam );
				if ( dwTemp > 0 )
				{
					gui->injectKeyPress(MyGUI::KeyCode::Enum(dwTemp));
					return true;
				}

				return true;
			}

		case WM_KEYUP:
			{
				DWORD dwTemp = TranslateScanCodeToGUIKey ( wParam );
				if ( dwTemp > 0 )
				{
					gui->injectKeyRelease(MyGUI::KeyCode::Enum(dwTemp));
					return false;
				}

				return false;
			}

		case WM_CHAR:
			//pGUI->ProcessCharacter ( wParam );
			gui->injectKeyPress(MyGUI::KeyCode::None, wParam);
			return true;
		}
	}
	else
	{
		if (uMsg == WM_KEYDOWN)
		{
			if (wParam == VK_HOME)
			{
				ToggleGlobalGUI(true);
				return true;
			}
		}
	}

	// The event wasn't handled
	return false;
}

LRESULT CALLBACK ProcessMessage(HWND hwnd, UINT uMsg, 
								WPARAM wParam, 
								LPARAM lParam)
{
	if (!ProcessGUIMessage(hwnd, uMsg, wParam, lParam)) {
		return CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc ( hwnd, uMsg, wParam, lParam );
}

void InitGUIWindow(HWND window)
{
	wndProc = SubclassWindow(window, ProcessMessage);
}

void GUIInited();

void InitGUIInterface ( IDirect3DDevice9* pDevice )
{
	if (gui) return;

    guiPlatform = new MyGUI::DirectXPlatform();
	guiPlatform->initialise(pDevice);

	guiPlatform->getRenderManagerPtr()->setViewSize(1280, 720);
	guiPlatform->getDataManagerPtr()->addResourceLocation("main\\gui", true);

	gui = new MyGUI::Gui();
	gui->initialise();
	//gui->showPointer();
	gui->setVisiblePointer(false);

	/*MyGUI::ButtonPtr button = gui->createWidget<MyGUI::Button>("Button", 10, 10, 300, 26, MyGUI::Align::Default, "Main");
	button->setCaption("exit");*/

	GUIInited();
}
