
#ifndef CORONA_HEADERS_H_
#define CORONA_HEADERS_H_

#include <crtdbg.h>
#include <math.h>
#include <xmmintrin.h> //necessary for sse intrinsics

#include <stdio.h>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "resource.h"

#include "corona_defines.h"

//Shared Objects (These shouldn't need to be debugged or messed with)
#include "types.h"
#include "timer.h"
#include "registry_class.h"
#include "stack.h"
#include "messages.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3x3.h"
#include "matrix4x4.h"

#include "pcx.h"

//Model Formats
#include "md2.h"
#include "vgd.h"

#include "options.h"
#include "graphics.h"
#include "vertex.h"
#include "camera.h"
#include "light.h"
#include "renderer.h"
#include "mesh.h"
#include "mesh_loader.h"


#include "corona_winmain.h"
#include "corona_engine.h"
#include "dialog.h"

#endif
