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

#ifndef _VERTEXMANAGER_H_
#define _VERTEXMANAGER_H_

#include "CPMemory.h"

#include "VertexManagerBase.h"

namespace OGL
{
	class GLVertexFormat : public NativeVertexFormat
	{
		PortableVertexDeclaration vtx_decl;

	public:
		GLVertexFormat();
		~GLVertexFormat();

		virtual void Initialize(const PortableVertexDeclaration &_vtx_decl);
		virtual void SetupVertexPointers();
		
		GLuint VAO;
	};

// Handles the OpenGL details of drawing lots of vertices quickly.
// Other functionality is moving out.
class VertexManager : public ::VertexManager
{
public:
	VertexManager();
	~VertexManager();
	NativeVertexFormat* CreateNativeVertexFormat();
	void CreateDeviceObjects();
	void DestroyDeviceObjects();
	
	// NativeVertexFormat use this
	GLuint m_vertex_buffers;
	GLuint m_index_buffers; 
	GLuint m_last_vao;
private:
	void Draw(u32 stride);
	void vFlush();
	void PrepareDrawBuffers(u32 stride);
	NativeVertexFormat *m_CurrentVertexFmt;
};

}

#endif  // _VERTEXMANAGER_H_
