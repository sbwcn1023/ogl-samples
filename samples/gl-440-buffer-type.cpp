//**********************************
// OpenGL Buffer Type
// 10/05/2010
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
	char const * SAMPLE_NAME("OpenGL Buffer Type");
	char const * VERT_SHADER_SOURCE("gl-440/buffer-type.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/buffer-type.frag");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(4);

	class i10i10i10i2
	{
	public:
		i10i10i10i2(float x, float y, float z) :
			x(int(511.0f * x)),
			y(int(511.0f * y)),
			z(int(511.0f * z)),
			w(1)
		{}

		operator glm::vec3()
		{
			return glm::vec3(
				float(x) / 511.0f,
				float(y) / 511.0f,
				float(z) / 511.0f);
		}

	private:
		int x : 10;
		int y : 10;
		int z : 10;
		int w : 2;
	};

namespace detail
{
	glm::uint16 float2half(glm::uint32 const & f)
	{
		// 10 bits    =>                         EE EEEFFFFF
		// 11 bits    =>                        EEE EEFFFFFF
		// Half bits  =>                   SEEEEEFF FFFFFFFF
		// Float bits => SEEEEEEE EFFFFFFF FFFFFFFF FFFFFFFF

		// 0x00007c00 => 00000000 00000000 01111100 00000000
		// 0x000003ff => 00000000 00000000 00000011 11111111
		// 0x38000000 => 00111000 00000000 00000000 00000000 
		// 0x7f800000 => 01111111 10000000 00000000 00000000 
		// 0x00008000 => 00000000 00000000 10000000 00000000 
		return 
			((f >> 16) & 0x8000) | // sign
			((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | // exponential
			((f >> 13) & 0x03ff); // Mantissa
	}

	glm::uint16 float2packed11(glm::uint32 const & f)
	{
		// 10 bits    =>                         EE EEEFFFFF
		// 11 bits    =>                        EEE EEFFFFFF
		// Half bits  =>                   SEEEEEFF FFFFFFFF
		// Float bits => SEEEEEEE EFFFFFFF FFFFFFFF FFFFFFFF

		// 0x000007c0 => 00000000 00000000 00000111 11000000
		// 0x00007c00 => 00000000 00000000 01111100 00000000
		// 0x000003ff => 00000000 00000000 00000011 11111111
		// 0x38000000 => 00111000 00000000 00000000 00000000 
		// 0x7f800000 => 01111111 10000000 00000000 00000000 
		// 0x00008000 => 00000000 00000000 10000000 00000000 
		return 
			((((f & 0x7f800000) - 0x38000000) >> 17) & 0x07c0) | // exponential
			((f >> 17) & 0x003f); // Mantissa
	}

	glm::uint16 float2packed10(glm::uint32 const & f)
	{
		// 10 bits    =>                         EE EEEFFFFF
		// 11 bits    =>                        EEE EEFFFFFF
		// Half bits  =>                   SEEEEEFF FFFFFFFF
		// Float bits => SEEEEEEE EFFFFFFF FFFFFFFF FFFFFFFF

		// 0x0000001F => 00000000 00000000 00000000 00011111
		// 0x0000003F => 00000000 00000000 00000000 00111111
		// 0x000003E0 => 00000000 00000000 00000011 11100000
		// 0x000007C0 => 00000000 00000000 00000111 11000000
		// 0x00007C00 => 00000000 00000000 01111100 00000000
		// 0x000003FF => 00000000 00000000 00000011 11111111
		// 0x38000000 => 00111000 00000000 00000000 00000000 
		// 0x7f800000 => 01111111 10000000 00000000 00000000 
		// 0x00008000 => 00000000 00000000 10000000 00000000 
		return 
			((((f & 0x7f800000) - 0x38000000) >> 18) & 0x03E0) | // exponential
			((f >> 18) & 0x001f); // Mantissa
	}

	glm::uint32 half2float(glm::uint16 const & h)
	{
		return ((h & 0x8000) << 16) | ((( h & 0x7c00) + 0x1C000) << 13) | ((h & 0x03FF) << 13);
	}

	int floatTo11bit(float x)
	{
		if(x == 0.0f)
			return 0;
		else if(glm::isnan(x))
			return ~0;
		else if(glm::isinf(x))
			return 0x1f << 6;
	}

	int floatTo10bit(float x)
	{
		if(x == 0.0f)
			return 0;
		else if(glm::isnan(x))
			return ~0;
		else if(glm::isinf(x))
			return 0x1f << 5;
	}

	glm::uint32 f11_f11_f10(float x, float y, float z)
	{
		return floatTo11bit(x) |  (floatTo11bit(y) << 11) | (floatTo11bit(z) << 22);
	}
}//namespace detail

	class f11f11f10
	{
	public:
		f11f11f10(float x, float y, float z) :
			x(detail::floatTo11bit(x)),
			y(detail::floatTo11bit(y)),
			z(detail::floatTo10bit(z))
		{}
/*
		operator glm::vec3()
		{
			return glm::vec3(
				float(x) / 511.0f,
				float(y) / 511.0f,
				float(z) / 511.0f);
		}
*/
	private:
		int x : 11;
		int y : 11;
		int z : 10;
	};

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSizeF16 = VertexCount * sizeof(glm::half) * 2;
	glm::half const PositionDataF16[VertexCount * 2] =
	{
		glm::half(-1.0f), glm::half(-1.0f),
		glm::half( 1.0f), glm::half(-1.0f),
		glm::half( 1.0f), glm::half( 1.0f),
		glm::half( 1.0f), glm::half( 1.0f),
		glm::half(-1.0f), glm::half( 1.0f),
		glm::half(-1.0f), glm::half(-1.0f)
	};

	GLsizeiptr const PositionSizeF32 = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionDataF32[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	GLsizeiptr const PositionSizeI8 = VertexCount * sizeof(glm::i8vec2);
	glm::i8vec2 const PositionDataI8[VertexCount] =
	{
		glm::i8vec2(-1,-1),
		glm::i8vec2( 1,-1),
		glm::i8vec2( 1, 1),
		glm::i8vec2( 1, 1),
		glm::i8vec2(-1, 1),
		glm::i8vec2(-1,-1)
	};

	GLsizeiptr const PositionSizeRGB10A2 = VertexCount * sizeof(i10i10i10i2);
	i10i10i10i2 const PositionDataRGB10A2[VertexCount] =
	{
		i10i10i10i2(-1.0f,-1.0f, 0.0f),
		i10i10i10i2( 1.0f,-1.0f, 0.0f),
		i10i10i10i2( 1.0f, 1.0f, 0.0f),
		i10i10i10i2( 1.0f, 1.0f, 0.0f),
		i10i10i10i2(-1.0f, 1.0f, 0.0f),
		i10i10i10i2(-1.0f,-1.0f, 0.0f)
	};

	GLsizeiptr const PositionSizeI32 = VertexCount * sizeof(glm::i32vec2);
	glm::i32vec2 const PositionDataI32[VertexCount] =
	{
		glm::i32vec2(-1,-1),
		glm::i32vec2( 1,-1),
		glm::i32vec2( 1, 1),
		glm::i32vec2( 1, 1),
		glm::i32vec2(-1, 1),
		glm::i32vec2(-1,-1)
	};

	GLsizeiptr const PositionSizeRG11FB10F = VertexCount * sizeof(f11f11f10);
	glm::uint32 const PositionDataRG11FB10F[VertexCount] =
	{
		glm::uint32(detail::f11_f11_f10(-1.0f,-1.0f, 0.0f)),
		glm::uint32(detail::f11_f11_f10( 1.0f,-1.0f, 0.0f)),
		glm::uint32(detail::f11_f11_f10( 1.0f, 1.0f, 0.0f)),
		glm::uint32(detail::f11_f11_f10( 1.0f, 1.0f, 0.0f)),
		glm::uint32(detail::f11_f11_f10(-1.0f, 1.0f, 0.0f)),
		glm::uint32(detail::f11_f11_f10(-1.0f,-1.0f, 0.0f))
	};

	namespace vertex_format
	{
		enum type
		{
			F32,
			I8,
			I32,
			RGB10A2,
			F16,
			RG11B10F,
			MAX
		};
	}

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}

	namespace viewport
	{
		enum type
		{
			VIEWPORT0,
			VIEWPORT1,
			VIEWPORT2,
			VIEWPORT3,
			VIEWPORT4,
			VIEWPORT5,
			MAX
		};
	}

	GLuint PipelineName(0);
	GLuint ProgramName(0);
	GLint UniformMVP(0);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> VertexArrayName(vertex_format::MAX);

	struct view
	{
		view(){}

		view(
			glm::vec4 const & Viewport, 
			vertex_format::type const & VertexFormat) :
			Viewport(Viewport),
			VertexFormat(VertexFormat)
		{}

		glm::vec4 Viewport;
		vertex_format::type VertexFormat;
	};

	std::vector<view> Viewport(viewport::MAX);
	glm::mat4* UniformPointer(NULL);
}//namespace

bool initDebugOutput()
{
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return glf::checkError("initDebugOutput");
}

bool initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		glf::compiler Compiler;
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, 
			glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, 
			"--version 440 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, 
			glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE,
			"--version 440 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glLinkProgram(ProgramName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);

		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
	}

	if(Validated)
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

	return Validated && glf::checkError("initProgram");
}

bool initBuffer()
{
	// Generate a buffer object
	glGenBuffers(buffer::MAX, &BufferName[0]);

	// Allocate and copy buffers memory
	std::vector<glm::byte> Data(PositionSizeF32 + PositionSizeI8 + PositionSizeI32 + PositionSizeRGB10A2 + PositionSizeF16 + PositionSizeRG11FB10F);
	
	std::size_t CurrentOffset = 0;
	memcpy(&Data[0] + CurrentOffset, PositionDataF32, PositionSizeF32);
	CurrentOffset += PositionSizeF32;
	memcpy(&Data[0] + CurrentOffset, PositionDataI8, PositionSizeI8);
	CurrentOffset += PositionSizeI8;
	memcpy(&Data[0] + CurrentOffset, PositionDataI32, PositionSizeI32);
	CurrentOffset += PositionSizeI32;
	memcpy(&Data[0] + CurrentOffset, PositionDataRGB10A2, PositionSizeRGB10A2);
	CurrentOffset += PositionSizeRGB10A2;
	memcpy(&Data[0] + CurrentOffset, PositionDataF16, PositionSizeF16);
	CurrentOffset += PositionSizeF16;
	memcpy(&Data[0] + CurrentOffset, PositionDataRG11FB10F, PositionSizeRG11FB10F);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
	glBufferStorage(GL_ARRAY_BUFFER, GLsizeiptr(Data.size()), &Data[0], 0);

	GLint UniformBufferOffset(0);

	glGetIntegerv(
		GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&UniformBufferOffset);

	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return glf::checkError("initBuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(vertex_format::MAX, &VertexArrayName[0]);
	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);

	std::size_t CurrentOffset = 0;
	glBindVertexArray(VertexArrayName[vertex_format::F32]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	
	CurrentOffset += PositionSizeF32;
	glBindVertexArray(VertexArrayName[vertex_format::I8]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_BYTE, GL_FALSE, sizeof(glm::u8vec2), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);

	CurrentOffset += PositionSizeI8;
	glBindVertexArray(VertexArrayName[vertex_format::I32]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_INT, GL_FALSE, sizeof(glm::i32vec2), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);

	CurrentOffset += PositionSizeI32;
	glBindVertexArray(VertexArrayName[vertex_format::RGB10A2]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(i10i10i10i2), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);

	CurrentOffset += PositionSizeRGB10A2;
	glBindVertexArray(VertexArrayName[vertex_format::F16]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_HALF_FLOAT, GL_FALSE, sizeof(glm::hvec2), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);

	CurrentOffset += PositionSizeRG11FB10F;
	glBindVertexArray(VertexArrayName[vertex_format::RG11B10F]);
	glVertexAttribPointer(glf::semantic::attr::POSITION, 3, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_FALSE, sizeof(glm::uint), GLF_BUFFER_OFFSET(CurrentOffset));
	glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	glm::vec2 ViewportSize(Window.Size.x * 0.33f, Window.Size.y * 0.50f);

	Viewport[viewport::VIEWPORT0] = view(glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::F16);
	Viewport[viewport::VIEWPORT1] = view(glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::I32);
	Viewport[viewport::VIEWPORT2] = view(glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::RGB10A2);
	Viewport[viewport::VIEWPORT3] = view(glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::I8);
	Viewport[viewport::VIEWPORT4] = view(glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::F32);
	Viewport[viewport::VIEWPORT5] = view(glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::RG11B10F);

	bool Validated = true;
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	UniformPointer = (glm::mat4*)glMapBufferRange(
		GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

	return Validated && glf::checkError("begin");
}

bool end()
{
	if(!UniformPointer)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		UniformPointer = NULL;
	}

	glDeleteBuffers(buffer::MAX, &BufferName[0]);
	glDeleteVertexArrays(buffer::MAX, &VertexArrayName[0]);
	glDeleteProgramPipelines(1, &PipelineName);
	glDeleteProgram(ProgramName);

	return glf::checkError("end");
}

void display()
{
	{
		// Compute the MVP (Model View Projection matrix)
		float Aspect = (Window.Size.x * 0.33f) / (Window.Size.y * 0.50f);
		glm::mat4 Projection = glm::perspective(45.0f, Aspect, 0.1f, 100.0f);
		glm::mat4 ViewTranslateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslateZ, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 View = ViewRotateY;
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		*UniformPointer = MVP;
	}

	glFlushMappedBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4));

	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glBindProgramPipeline(PipelineName);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	for(std::size_t Index = 0; Index < viewport::MAX; ++Index)
	{
		glViewportIndexedf(0, 
			Viewport[Index].Viewport.x, 
			Viewport[Index].Viewport.y, 
			Viewport[Index].Viewport.z, 
			Viewport[Index].Viewport.w);

		glBindVertexArray(VertexArrayName[Viewport[Index].VertexFormat]);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		GLF_CONTEXT_CORE_PROFILE_BIT, 
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}

