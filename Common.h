#pragma once

// #define TGFEXT       // TGF2, MMF2, MMF2 Dev
   #define MMFEXT       // MMF2, MMF2 Dev
// #define PROEXT       // MMF2 Dev only


#include    "Edif.h"
#include	"Resource.h"
#include <windows.h>
#include "atlstr.h"
#include <exiv2/exiv2.hpp>

// edPtr : Used at edittime and saved in the MFA/CCN/EXE files

struct EDITDATA
{
	// Header - required
	extHeader		eHeader;

	// Object's data
//	short			swidth;
//	short			sheight;

};

#include "Extension.h"
