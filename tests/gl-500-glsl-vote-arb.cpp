///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-500/glsl-vote.vert");
	char const * FRAG_SHADER_SOURCE("gl-500/glsl-vote.frag");
	char const * TEXTURE_DIFFUSE("kueken7_rgba8_srgb.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const VertexData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	namespace buffer
	{
		enum type
		{
			TRANSFORM,
			VERTEX,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_500_glsl_vote : public test
{
public:
	gl_500_glsl_vote(int argc, char* argv[]) :
		test(argc, argv, "gl-500-glsl-vote-arb", test::CORE, 4, 4),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0),
		UniformPointer(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	glm::mat4* UniformPointer;

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_SHADER_STORAGE_BUFFER, VertexSize, VertexData, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		this->UniformPointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER,
			0, sizeof(glm::mat4) * 2, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

		return true;
	}

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;

			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 440 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 440 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.check_program(ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initTexture()
	{
		gli::texture2d Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), Format.Internal, GLsizei(Texture[0].extent().x), GLsizei(Texture[0].extent().y));
		for(gli::texture2d::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D, GLint(Level),
				0, 0,
				GLsizei(Texture[Level].extent().x), GLsizei(Texture[Level].extent().y),
				Format.External, Format.Type,
				Texture[Level].data());
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_shader_group_vote");

		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
			glm::mat4 View = this->view();
			glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			glm::mat4 MVP = Projection * View * Model;
			glm::mat4 MV = View * Model;
		
			*(this->UniformPointer + 0) = MVP;
			*(this->UniformPointer + 1) = MV;
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, BufferName[buffer::VERTEX]);
		glBindTextures(semantic::sampler::DIFFUSE, 1, &TextureName);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_POINTS, 0, VertexCount, 1);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_glsl_vote Test(argc, argv);
	Error += Test();

	return Error;
}



