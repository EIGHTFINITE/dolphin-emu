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

#ifndef _BANNER_LOADER_H_
#define _BANNER_LOADER_H_

#include <vector>
#include <string>

#include "Filesystem.h"

namespace DiscIO
{
class IBannerLoader
{
	public:

		IBannerLoader()
		{}


		virtual ~IBannerLoader()
		{}


		virtual bool IsValid() = 0;

		virtual bool GetBanner(u32* _pBannerImage) = 0;

		virtual std::vector<std::string> GetNames() = 0;
		virtual std::string GetCompany() = 0;
		virtual std::vector<std::string> GetDescriptions() = 0;
};

IBannerLoader* CreateBannerLoader(DiscIO::IFileSystem& _rFileSystem, DiscIO::IVolume *pVolume);
} // namespace

#endif

