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

#pragma once

#include "GLUtil.h"

#include "VertexShaderCache.h"
#include "PixelShaderCache.h"
#include "PixelShaderGen.h"
#include "VertexShaderGen.h"

#include "LinearDiskCache.h"
#include "ConfigManager.h"

namespace OGL
{

const int NUM_UNIFORMS = 27;
extern const char *UniformNames[NUM_UNIFORMS];

class ProgramShaderCache
{
public:
	typedef std::pair<u32, u32> ShaderUID;

	struct PCacheEntry
	{
		GLuint prog_id;
		static GLenum prog_format;
		u8 *binary;
		GLint binary_size;
		GLuint vsid, psid;
		ShaderUID uid;
		GLint UniformLocations[NUM_UNIFORMS];

		PCacheEntry() : prog_id(0), binary(NULL), binary_size(0), vsid(0), psid(0)  { }

		~PCacheEntry()
		{
			FreeProgram();
		}

		void Create(const GLuint pix_id, const GLuint vert_id)
		{
			psid = pix_id;
			vsid = vert_id;
			uid = std::make_pair(psid, vsid);
			prog_id = glCreateProgram();
		}

		void Destroy()
		{
			glDeleteProgram(prog_id);
			prog_id = 0;
		}

		void UpdateSize()
		{
			if (binary_size == 0)
				glGetProgramiv(prog_id, GL_PROGRAM_BINARY_LENGTH, &binary_size);
		}

		// No idea how necessary this is
		static GLenum SetProgramFormat()
		{
			GLint Supported;
			glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &Supported);

			GLint *Formats = new GLint[Supported];
			glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, Formats);
			// We don't really care about format
			GLenum prog_format = (GLenum)Formats[0];
			delete[] Formats;
			return prog_format;
		}

		u8 *GetProgram()
		{
			UpdateSize();
			FreeProgram();
			binary = new u8[binary_size];
			glGetProgramBinary(prog_id, binary_size, NULL, &prog_format, binary);
			return binary;
		}

		void FreeProgram()
		{
			delete [] binary;
			binary = NULL;
		}

		GLint Size()
		{
			UpdateSize();
			return binary_size;
		}
	};

	static PCacheEntry GetShaderProgram(void);
	static void SetBothShaders(GLuint PS, GLuint VS);
	static GLuint GetCurrentProgram(void);

	static void SetMultiPSConstant4fv(unsigned int offset, const float *f, unsigned int count);
	static void SetMultiVSConstant4fv(unsigned int offset, const float *f, unsigned int count);

	static void Init(void);
	static void Shutdown(void);

private:
	class ProgramShaderCacheInserter : public LinearDiskCacheReader<ShaderUID, u8>
	{
	public:
		void Read(const ShaderUID &key, const u8 *value, u32 value_size)
		{
			// The two shaders might not even exist anymore
			// But it is fine, no need to worry about that
			PCacheEntry entry;
			entry.Create(key.first, key.second);

			glProgramBinary(entry.prog_id, entry.prog_format, value, value_size);

			GLint success;
			glGetProgramiv(entry.prog_id, GL_LINK_STATUS, &success);

			if (success)
			{
				pshaders[key] = entry;
				glUseProgram(entry.prog_id);
				SetProgramVariables(entry);
			}
		}
	};

	typedef std::map<ShaderUID, PCacheEntry> PCache;

	static PCache pshaders;
	static GLuint CurrentFShader, CurrentVShader, CurrentProgram;
	static ShaderUID CurrentShaderProgram;

	static GLuint s_ps_vs_ubo;
	static GLintptr s_vs_data_offset;
	static void SetProgramVariables(PCacheEntry &entry);
};

}  // namespace OGL
