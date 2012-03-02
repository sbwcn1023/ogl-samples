//**********************************
// OpenGL Texture Conversion
// 26/02/2012 - 26/02/2012
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Texture Conversion";
	std::string const VERT_SHADER_SOURCE(glf::DATA_DIRECTORY + "gl-420/texture-conversion.vert");
	std::string const FRAG_SHADER_SOURCE[2] = 
	{
		glf::DATA_DIRECTORY + "gl-420/texture-conversion-normalized.frag", 
		glf::DATA_DIRECTORY + "gl-420/texture-conversion-uint.frag"
	};
	std::string const TEXTURE_DIFFUSE(glf::DATA_DIRECTORY + "kueken256-rgba8.dds");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace texture
	{
		enum type
		{
			RGBA8, // GL_RGBA8
			RGBA8UI, // GL_RGBA8UI
			RGBA32F, // GL_RGBA32F
			RGBA8_SNORM, 
			MAX
		};
	}//namespace texture

	namespace program
	{
		enum type
		{
			NORM, 
			UINT, 
			MAX
		};
	}//namespace program

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	GLenum TextureInternalFormat[texture::MAX] = 
	{
		GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
		GL_RGBA8UI, 
		GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,
		GL_RGBA8_SNORM
	};

	GLenum TextureFormat[texture::MAX] = 
	{
		GL_BGRA, 
		GL_BGRA_INTEGER, 
		GL_BGRA,
		GL_BGRA
	};

	GLuint VertexArrayName(0);
	GLuint PipelineName[program::MAX] = {0, 0};
	GLuint ProgramName[program::MAX] = {0, 0};

	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint TextureName[texture::MAX] = {0, 0};

	glm::ivec4 Viewport[texture::MAX] = 
	{
		glm::ivec4(  0,   0, 320, 240),
		glm::ivec4(320,   0, 320, 240),
		glm::ivec4(320, 240, 320, 240),
		glm::ivec4(  0, 240, 320, 240)
	};

}//namespace

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return Validated;
}

bool initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(program::MAX, PipelineName);

	for(std::size_t i = 0; (i < program::MAX) && Validated; ++i)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE);
		GLuint FragShaderNameNorm = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE[program::NORM]);
		GLuint FragShaderNameUint = glf::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE[program::UINT]);

		ProgramName[program::NORM] = glCreateProgram();
		glProgramParameteri(ProgramName[program::NORM], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::NORM], VertShaderName);
		glAttachShader(ProgramName[program::NORM], FragShaderNameNorm);
		glLinkProgram(ProgramName[program::NORM]);
		Validated = Validated && glf::checkProgram(ProgramName[program::NORM]);

		ProgramName[program::UINT] = glCreateProgram();
		glProgramParameteri(ProgramName[program::UINT], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::UINT], VertShaderName);
		glAttachShader(ProgramName[program::UINT], FragShaderNameUint);
		glLinkProgram(ProgramName[program::UINT]);
		Validated = Validated && glf::checkProgram(ProgramName[program::UINT]);

		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderNameNorm);
		glDeleteShader(FragShaderNameUint);
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName[program::NORM], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::NORM]);
		glUseProgramStages(PipelineName[program::UINT], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::UINT]);
	}

	return Validated;
}

bool initBuffer()
{
	bool Validated(true);

	glGenBuffers(buffer::MAX, BufferName);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint UniformBufferOffset(0);

	glGetIntegerv(
		GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&UniformBufferOffset);

	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return Validated;
}

bool initTexture()
{
	gli::texture2D Texture = gli::load(TEXTURE_DIFFUSE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(texture::MAX, TextureName);
	glActiveTexture(GL_TEXTURE0);

	for(std::size_t i = 0; i < texture::MAX; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, TextureName[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), TextureInternalFormat[i], 
			GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				TextureFormat[i], 
				GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return true;
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
    glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBindVertexArray(0);

	return true;
}

bool begin()
{
	bool Validated = glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated;
}

bool end()
{
	glDeleteBuffers(buffer::MAX, BufferName);
	for(std::size_t i = 0; i < program::MAX; ++i)
		glDeleteProgram(ProgramName[i]);
	glDeleteTextures(texture::MAX, TextureName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return true;
}

void display()
{
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	// Update of the uniform buffer
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 Model = glm::mat4(1.0f);
		
		*Pointer = Projection * View * Model;

		// Make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	glBindVertexArray(VertexArrayName);
	glActiveTexture(GL_TEXTURE0);

	glBindProgramPipeline(PipelineName[program::UINT]);
	{
		glViewportIndexedfv(0, &glm::vec4(Viewport[texture::RGBA8UI])[0]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8UI]);

		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);
	}

	glBindProgramPipeline(PipelineName[program::NORM]);
	{
		glViewportIndexedfv(0, &glm::vec4(Viewport[texture::RGBA32F])[0]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA32F]);

		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		glViewportIndexedfv(0, &glm::vec4(Viewport[texture::RGBA8])[0]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8]);

		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		glViewportIndexedfv(0, &glm::vec4(Viewport[texture::RGBA8_SNORM])[0]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8_SNORM]);

		glDrawElementsInstancedBaseVertexBaseInstance(
			GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);
	}

	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
		::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);
}