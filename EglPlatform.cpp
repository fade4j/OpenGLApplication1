#include "EglPlatform.h"
#include "WinPlatform.h"

EGLint GetEvent(NativeWindowType Window, InputEvent * Event)
{
    InputEvent  inEvent;
    EGLint      result = 0;
    STATUS      status = eglSTATUS_OK;

    status = WinGetEvent(Window, &inEvent);
    if (status == eglSTATUS_OK)
    {
        result = 1;
        switch(inEvent.type)
        {
        case EVENT_KEYBOARD:
            Event->type = EVENT_KEYBOARD;
            Event->data.keyboard.scancode = (Keys)inEvent.data.keyboard.scancode;
            Event->data.keyboard.pressed  = inEvent.data.keyboard.pressed;
            Event->data.keyboard.key      = inEvent.data.keyboard.key;
            break;

        case EVENT_BUTTON:
            Event->type = EVENT_BUTTON;
            Event->data.button.left     = inEvent.data.button.left;
            Event->data.button.right    = inEvent.data.button.right;
            Event->data.button.middle   = inEvent.data.button.middle;
            Event->data.button.x        = inEvent.data.button.x;
            Event->data.button.y        = inEvent.data.button.y;
            break;

        case EVENT_POINTER:
            Event->type = EVENT_POINTER;
            Event->data.pointer.x = inEvent.data.pointer.x;
            Event->data.pointer.y = inEvent.data.pointer.y;
            break;

        case EVENT_CLOSE:
            Event->type = EVENT_CLOSE;
            break;

        case EVENT_WINDOW_UPDATE:
            Event->type = EVENT_WINDOW_UPDATE;
            break;

        default:
            return 0;
        }
    }

    return result;
}

void FinishEGL(EglObj *pEgl)
{
    /* Only process a valid EGL pointer. */
    if (pEgl != NULL)
    {
        /* Only process a valid EGLDisplay pointer. */
        if (pEgl->eglDisplay != EGL_NO_DISPLAY)
        {
            /* Unbind everything from the EGL context. */
            eglMakeCurrent(pEgl->eglDisplay, NULL, NULL, NULL);

            if (pEgl->eglContext != EGL_NO_CONTEXT)
            {
                /* Destroy the EGL context. */
                eglDestroyContext(pEgl->eglDisplay, pEgl->eglContext);
                pEgl->eglContext = EGL_NO_CONTEXT;
            }

            if (pEgl->eglSurface != EGL_NO_SURFACE)
            {
                /* Destroy the EGL surface. */
                eglDestroySurface(pEgl->eglDisplay, pEgl->eglSurface);
                pEgl->eglSurface = EGL_NO_SURFACE;
            }

            /* Terminate the EGL display. */
            eglTerminate(pEgl->eglDisplay);
            pEgl->eglDisplay = EGL_NO_DISPLAY;

            /* Release thread data. */
            eglReleaseThread();
        }

        if (pEgl->window != 0)
        {
            /* Hide the window. */
            WinHideWindow(pEgl->window);

            /* Destroy the window. */
            WinDestroyWindow(pEgl->window);
            pEgl->window = 0;
        }

        if (pEgl->display != NULL)
        {
            /* Destroy the display. */
            WinDestroyDisplay(pEgl->display);
            pEgl->display = NULL;
        }
    }
}

int SetupEGL(int X,
             int Y,
             int Width,
             int Height,
             const EGLint * ConfigurationAttributes,
             const EGLint * SurfaceAttributes,
             const EGLint * ContextAttributes,
             EglObj * pEgl)
{
    STATUS status = eglSTATUS_OK;

    /* Valid configurations. */
    EGLint matchingConfigs;

    /* Make sure we have a valid pEgl pointer. */
    if (pEgl == NULL)
    {
        return 0;
    }

    status = WinCreateWindowClass("tutorial1Class");
    if (status != eglSTATUS_OK)
    {
        return 0;
    }

    if (pEgl->display == NULL)
    {
        WinGetNativeDisplay(&pEgl->display, 0);

        /* Test for error. */
        /* EGL display can be NULL (=EGL_DEFAULT_DISPLAY) in QNX. Hence, don't test for error. */
#ifndef ANDROID_JNI
        if (pEgl->display == NULL)
        {
            return 0;
        }
#endif
    }

    /* Create the window. */
    if (pEgl->window == 0)
    {
        WinCreateNativeWindow("tutorial1Class",
                              "tutorial1",
                              NativeDisplayType(pEgl->display),
                              X,
                              Y,
                              Width,
                              Height,
                              &pEgl->window);

        /* Test for error. */
        if (pEgl->window == 0)
        {
            return 0;
        }
    }

    /* Get the EGL display. */
    pEgl->eglDisplay = eglGetDisplay(pEgl->display);

    if (pEgl->eglDisplay == EGL_NO_DISPLAY)
    {
        return 0;
    }

    /* Initialize the EGL and test for error. */
    if (!eglInitialize(pEgl->eglDisplay,
                       &pEgl->eglMajor,
                       &pEgl->eglMinor))
    {
        return 0;
    }

    /* Choose a configuration and test for error. */
    if (pEgl->eglConfig == NULL)
    {
        /* Default configuration. */
        EGLint configuration[] =
        {
            EGL_RED_SIZE,           8,
            EGL_GREEN_SIZE,         8,
            EGL_BLUE_SIZE,          8,
            EGL_DEPTH_SIZE,         24,
            EGL_SAMPLES,            0,
            EGL_RENDERABLE_TYPE,    EGL_DONT_CARE,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_NONE,
        };

        int defaultConfig = 0;

        /* Test for the default configuration. */
        if (ConfigurationAttributes == EGL_CONFIG_RGB888_D24)
        {
            defaultConfig = 1;
        }

        /* Test for RGB565 color. */
        if ((ConfigurationAttributes == EGL_CONFIG_RGB565_D16)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_D24)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_D24_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_AA)
        )
        {
            defaultConfig    = 1;
            configuration[1] = 5;
            configuration[3] = 6;
            configuration[5] = 5;
        }

        /* Test for no depth. */
        if ((ConfigurationAttributes == EGL_CONFIG_RGB565)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888_AA)
        )
        {
            defaultConfig    = 1;
            configuration[7] = 0;
        }

        /* Test for D16 depth. */
        if ((ConfigurationAttributes == EGL_CONFIG_RGB565_D16)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888_D16)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888_D16_AA)
        )
        {
            defaultConfig    = 1;
            configuration[7] = 16;
        }

        /* Test for Anti-Aliasing. */
        if ((ConfigurationAttributes == EGL_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB565_D24_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888_D16_AA)
        ||  (ConfigurationAttributes == EGL_CONFIG_RGB888_D24_AA)
        )
        {
            defaultConfig    = 1;
            configuration[9] = 4;
        }

        /* Test for OpenVG RGB565 color. */
        if (ConfigurationAttributes == EGL_CONFIG_RGB565_VG)
        {
            defaultConfig     = 1;
            configuration[ 1] = 5;
            configuration[ 3] = 6;
            configuration[ 5] = 5;
            configuration[ 7] = EGL_DONT_CARE;
            configuration[ 9] = EGL_DONT_CARE;
            configuration[11] = EGL_OPENVG_BIT;

            /* Bind OpenVG API. */
            if (!pEgl->eglBindAPI(EGL_OPENVG_API))
            {
                return 0;
            }
        }

        /* Test for OpenVG RGB565 color. */
        if (ConfigurationAttributes == EGL_CONFIG_RGB888_VG)
        {
            defaultConfig     = 1;
            configuration[ 7] = EGL_DONT_CARE;
            configuration[ 9] = EGL_DONT_CARE;
            configuration[11] = EGL_OPENVG_BIT;

            /* Bind OpenVG API. */
            if (!pEgl->eglBindAPI(EGL_OPENVG_API))
            {
                return 0;
            }
        }

        if (!eglChooseConfig(pEgl->eglDisplay,
                            defaultConfig
                                ? configuration
                                : ConfigurationAttributes,
                            &pEgl->eglConfig, 1,
                            &matchingConfigs))
        {
            return 0;
        }

        /* Make sure we got at least 1 configuration. */
        if (matchingConfigs < 1)
        {
            return 0;
        }
    }

    /* Create the EGL surface. */
    if (pEgl->eglSurface == EGL_NO_SURFACE)
    {
        pEgl->eglSurface = eglCreateWindowSurface(pEgl->eglDisplay,
                                                  pEgl->eglConfig,
                                                  pEgl->window,
                                                  SurfaceAttributes);

        /* Test for error. */
        if (pEgl->eglSurface == EGL_NO_SURFACE)
        {
            return 0;
        }
    }

    /* Create the EGL context. */
    if (pEgl->eglContext == EGL_NO_CONTEXT)
    {
        static const EGLint contextES20[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE,
        };

        pEgl->eglContext = eglCreateContext(pEgl->eglDisplay,
                                            pEgl->eglConfig,
                                            EGL_NO_CONTEXT,
                                            (ContextAttributes == EGL_CONTEXT_ES20)
                                                ? contextES20
                                                : ContextAttributes);

        /* Test for error. */
        if (pEgl->eglContext == EGL_NO_CONTEXT)
        {
            return 0;
        }
    }

    /* Bind the surface and context to the display and test for error. */
    /* Skip eglMakeCurrent if eglContext is set to EGL_DONT_CARE       */
    if (pEgl->eglContext != (EGLContext)EGL_DONT_CARE)
    {
        if (!eglMakeCurrent(pEgl->eglDisplay,
                            pEgl->eglSurface,
                            pEgl->eglSurface,
                            pEgl->eglContext))
        {                   
            return 0;
        }
    }

    /* Success. */
    return 1;
}

int SwapEGL(EglObj * pEgl)
{
    /* Make sure we have a valid pEgl pointer. */
    if (pEgl == NULL)
    {
        return 0;
    }

    /* Call EGL to swap the buffers. */
    return eglSwapBuffers(pEgl->eglDisplay, pEgl->eglSurface);
}