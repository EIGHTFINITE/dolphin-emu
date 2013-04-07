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


#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "VideoCommon.h"
#include "FramebufferManager.h"
#include "GLUtil.h"

// glew < 1.8 doesn't support pinned memory
#ifndef GLEW_AMD_pinned_memory
#define GLEW_AMD_pinned_memory glewIsSupported("GL_AMD_pinned_memory")
#define GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD 0x9160
#endif

namespace OGL
{
enum StreamType {
	DETECT_MASK = 0x3F,
	STREAM_DETECT = (1 << 0),
	MAP_AND_ORPHAN = (1 << 1),
	MAP_AND_SYNC = (1 << 2),
	MAP_AND_RISK = (1 << 3),
	PINNED_MEMORY = (1 << 4),
	BUFFERSUBDATA = (1 << 5)
};

class StreamBuffer {

public:
	StreamBuffer(u32 type, size_t size, StreamType uploadType = DETECT_MASK);
	~StreamBuffer();
	
	void Alloc(size_t size, u32 stride = 0);
	size_t Upload(u8 *data, size_t size);
	
	u32 getBuffer() { return m_buffer; }
	
private:
	void Init();
	void Shutdown();
	
	StreamType m_uploadtype;
	u32 m_buffer;
	u32 m_buffertype;
	size_t m_size;
	u8 *pointer;
	size_t m_iterator;
	size_t m_used_iterator;
	size_t m_free_iterator;
	GLsync *fences;
};

}

#endif // STREAMBUFFER_H
