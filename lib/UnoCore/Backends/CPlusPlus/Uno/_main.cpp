// @(MSG_ORIGIN)
// @(MSG_EDIT_WARNING)

#include <uBase/Memory.h>
#include <XliPlatform/Display.h>
#include <XliPlatform/GL.h>
#include <XliPlatform/GLContext.h>
#include <XliPlatform/MessageBox.h>
#include <XliPlatform/PlatformLib.h>
#include <XliPlatform/Window.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <thread>
@{Uno.Diagnostics.Clock:IncludeDirective}
@{Uno.Exception:IncludeDirective}
@{Uno.Platform.CoreApp:IncludeDirective}
@{Uno.Runtime.Implementation.Internal.Bootstrapper:IncludeDirective}
@{Uno.Runtime.Implementation.Internal.BootstrapHints:IncludeDirective}
@{Uno.Runtime.Implementation.PlatformWindowImpl:IncludeDirective}
@{Uno.Application:IncludeDirective}
@{string:IncludeDirective}

#if WIN32
#include <Uno/WinAPIHelper.h>
#include <Shellapi.h>
#elif LINUX || OSX
#include <XliPlatform/PlatformSpecific/SDL2.h>
#endif

#ifdef DEBUG_DUMPS
#include <stdio.h> // needed for sprintf
#endif

/**
    \addtogroup Main
    @{
*/
Xli::Window* _XliWindowPtr;
Xli::GLContext* _XliGLContextPtr;
uSStrong<uArray*> _CommandLineArgs = 0;

struct uMainLoop : Xli::WindowEventHandler
{
    uMainLoop()
    {
        Xli::PlatformLib::Init();
        
        Xli::DisplaySettings settings;
        int maxFps = Xli::Display::GetCount() > 0 &&
                        Xli::Display::GetCurrentSettings(0, settings) &&
                        settings.RefreshRate > 0
                    ? settings.RefreshRate
                    : 60;

        uBase::Auto<Xli::Window> wnd = Xli::Window::Create(uBase::Vector2i(375, 667), "@(Project.Title)", Xli::WindowFlagsResizeable);

        const char* hidden = getenv("UNO_WINDOW_HIDDEN");
        if (hidden && !strcmp(hidden, "1"))
        {
#if WIN32
            ShowWindow((HWND)wnd->GetNativeHandle(), SW_HIDE);
#elif LINUX || OSX
            SDL_HideWindow(Xli::PlatformSpecific::SDL2::GetWindowHandle(wnd));
#endif
        }

        uBase::Auto<Xli::GLContext> gl = Xli::GLContext::Create(wnd, Xli::GLContextAttributes::Default());

        // Store global references to wnd and gl
        _XliWindowPtr = wnd;
        _XliGLContextPtr = gl;

#ifdef U_GL_DESKTOP
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        gl->SetSwapInterval(0);
        gl->SwapBuffers();

        wnd->SetEventHandler(this);
        Xli::Window::ProcessMessages();
        @{Uno.Platform.CoreApp.Start():Call()};

        while (!wnd->IsClosed())
        {
            double startTime = @{Uno.Diagnostics.Clock.GetSeconds():Call()};
    
            Xli::Window::ProcessMessages();
            OnUpdate(wnd);
    
            if (wnd->IsVisible())
                OnDraw(wnd);
    
            if (maxFps > 0)
            {
                double targetTime = 1.0 / (double)maxFps;
                double renderTime = @{Uno.Diagnostics.Clock.GetSeconds():Call()} - startTime;
                int msTimeout = (int)((targetTime - renderTime) * 1000.0 + 0.5);
    
                if (msTimeout > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(msTimeout));
            }
        }
    }
    
    virtual void OnUpdate(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Runtime.Implementation.Internal.Bootstrapper.OnUpdate():Call()};
    }

    virtual void OnDraw(Xli::Window* wnd)
    {
        {
            uAutoReleasePool pool;

            if (@{Uno.Application.Current:Get()} && @{Uno.Application.Current:Get().NeedsRedraw:Get()})
            {
                @{Uno.Runtime.Implementation.Internal.Bootstrapper.OnDraw():Call()};
                _XliGLContextPtr->SwapBuffers();
            }
        }

#ifdef DEBUG_DUMPS
        static int frame = 0;
        char path[32];
        sprintf(path, "draw%d.dot", frame++);
        uDumpAllStrongRefs(path);
#endif
    }

    virtual bool OnKeyDown(Xli::Window* wnd, Xli::Key key)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnKeyDown(Uno.Runtime.Implementation.PlatformWindowHandle,Uno.Platform.Key):Call(wnd, key)})
            return true;

#ifdef WIN32
        if (key == Xli::KeyF11)
        {
            wnd->SetFullscreen(!wnd->IsFullscreen());
            return true;
        }
#endif

        return false;
    }

    virtual bool OnKeyUp(Xli::Window* wnd, Xli::Key key)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnKeyUp(Uno.Runtime.Implementation.PlatformWindowHandle,Uno.Platform.Key):Call(wnd, key)})
            return true;

        return false;
    }

    virtual bool OnTextInput(Xli::Window* wnd, const uBase::String& text)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnTextInput(Uno.Runtime.Implementation.PlatformWindowHandle,string):Call(wnd, uString::Utf8(text.Ptr()))})
            return true;

        return false;
    }

    virtual bool OnMouseDown(Xli::Window* wnd, uBase::Vector2i pos, Xli::MouseButton button)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnMouseDown(Uno.Runtime.Implementation.PlatformWindowHandle,int,int,Uno.Platform.MouseButton):Call(wnd, pos.X, pos.Y, button)})
            return true;

        return false;
    }

    virtual bool OnMouseUp(Xli::Window* wnd, uBase::Vector2i pos, Xli::MouseButton button)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnMouseUp(Uno.Runtime.Implementation.PlatformWindowHandle,int,int,Uno.Platform.MouseButton):Call(wnd, pos.X, pos.Y, button)})
            return true;

        return false;
    }

    virtual bool OnMouseMove(Xli::Window* wnd, uBase::Vector2i pos)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnMouseMove(Uno.Runtime.Implementation.PlatformWindowHandle,int,int):Call(wnd, pos.X, pos.Y)})
            return true;

        return false;
    }

    virtual bool OnMouseWheel(Xli::Window* wnd, uBase::Vector2i delta)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnMouseWheel(Uno.Runtime.Implementation.PlatformWindowHandle,float,float,int):Call(wnd, (float)delta.X, (float)delta.Y, 1)})
            return true;

        return false;
    }

    virtual bool OnTouchDown(Xli::Window* wnd, uBase::Vector2 pos, int id)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnTouchDown(Uno.Runtime.Implementation.PlatformWindowHandle,float,float,int):Call(wnd, pos.X, pos.Y, id)})
            return true;

        return false;
    }

    virtual bool OnTouchMove(Xli::Window* wnd, uBase::Vector2 pos, int id)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnTouchMove(Uno.Runtime.Implementation.PlatformWindowHandle,float,float,int):Call(wnd, pos.X, pos.Y, id)})
            return true;

        return false;
    }

    virtual bool OnTouchUp(Xli::Window* wnd, uBase::Vector2 pos, int id)
    {
        uAutoReleasePool pool;

        if (@{Uno.Runtime.Implementation.Internal.Bootstrapper.OnTouchUp(Uno.Runtime.Implementation.PlatformWindowHandle,float,float,int):Call(wnd, pos.X, pos.Y, id)})
            return true;

        return false;
    }

    virtual void OnNativeHandleChanged(Xli::Window* wnd)
    {
        _XliGLContextPtr->MakeCurrent(wnd);
    }

    virtual void OnSizeChanged(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Runtime.Implementation.Internal.Bootstrapper.OnWindowSizeChanged(Uno.Runtime.Implementation.PlatformWindowHandle):Call(wnd)};
#if WIN32 || OSX
        if (wnd->GetMouseButtonState(Xli::MouseButtonLeft))
        {
            OnUpdate(wnd);
            OnDraw(wnd);
        }
#endif
    }

    virtual bool OnClosing(Xli::Window* wnd, bool& cancel)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.EnterBackground():Call()};
        return false;
    }

    virtual void OnClosed(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.Terminate():Call()};
    }

    virtual void OnAppLowMemory(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.OnReceivedLowMemoryWarning():Call()};
    }

    virtual void OnAppTerminating(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.Terminate():Call()};
    }

    virtual void OnAppDidEnterForeground(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.EnterInteractive():Call()};
    }

    virtual void OnAppDidEnterBackground(Xli::Window* wnd)
    {
        uAutoReleasePool pool;
        @{Uno.Platform.CoreApp.EnterBackground():Call()};
    }
};

static void uUnhandledException(const char* msg, const char* function = NULL, int line = 0)
{
    std::stringstream ss;
    ss << msg;
    if (function)
        ss << "\n\nFunction: " << function << "\nLine: " << line;

    std::string str = ss.str();
    uLog(uLogLevelFatal, "Unhandled Exception: %s", str.c_str());
    Xli::MessageBox::Show(NULL, str.c_str(), "Unhandled Exception", Xli::DialogButtonsOK, Xli::DialogHintsError);
}

int main(int argc, char** argv)
{
    uRuntime uno;
    uAutoReleasePool pool;

    try
    {
#if WIN32
        int numArgs;
        LPWSTR* cmdArgs = CommandLineToArgvW(GetCommandLineW(), &numArgs);
        _CommandLineArgs = uArray::New(@{string[]:TypeOf}, numArgs);

        for (int i = 0; i < numArgs; i++)
            _CommandLineArgs->Strong<uString*>(i) = uString::Utf16((const char16_t*) cmdArgs[i]);
#else
        _CommandLineArgs = uArray::New(@{string[]:TypeOf}, argc);

        for (int i = 0; i < argc; i++)
            _CommandLineArgs->Strong<uString*>(i) = uString::Utf8(argv[i]);
#endif
        uMainLoop();
    }
    catch (const uThrowable& t)
    {
        uCString cstr(t.Exception->ToString());
        uUnhandledException(cstr.Ptr, t.Function, t.Line);
        return 1;
    }
    catch (const uBase::Exception& e)
    {
        uUnhandledException(e.GetMessage().Ptr(), e._func, e._line);
        return 1;
    }
    catch (const std::system_error& e)
    {
        std::stringstream ss;
        ss << "std::system_error " << e.code();
        uUnhandledException(ss.str().c_str());
        return 1;
    }
    catch (const std::exception& e)
    {
        uUnhandledException(e.what());
        return 1;
    }

    return 0;
}

#if WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

@(TypeObjects.Declaration:JoinSorted())
void uInitRtti(uType*(*factories[])());

void uInitRtti()
{
    static uType*(*factories[])() =
    {
        @(TypeObjects.FunctionPointer:JoinSorted('\n        ', '', ','))
        NULL
    };

    uInitRtti(factories);
}

@(Main.Include:JoinSorted('\n', '#include <', '>'))

void uStartApp()
{
    @(Main.Body:Indent(' ', 4))
}

/** @} */
