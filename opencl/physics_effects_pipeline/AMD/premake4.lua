
	project "OpenGL_physics_effects_AMD"


		language "C++"
				
		kind "ConsoleApp"
		targetdir "../../../bin"


		initOpenGL()
		initGlew()

		
		includedirs {
		"../../../rendering/rendertest",
		"../../primitives",
		"../../../bullet2",
		"../../../physics_effects"
		}
		
			
		links {
		"physics_effects_base_level",
		"physics_effects_low_level",
		"physics_effects_util"}
		
		files {

			"../main.cpp",
			"../physics_func.cpp",
			"../physics_func.h",
			"../btPgsSolver.cpp",
			"../btPgsSolver.h",
			"../btFakeRigidBody.cpp",
			"../btFakeRigidBody.h",
			"../../../rendering/rendertest/GLInstancingRenderer.cpp",
			"../../../rendering/rendertest/GLInstancingRenderer.h",
			"../../../rendering/rendertest/Win32OpenGLRenderManager.cpp",
			"../../../rendering/rendertest/Win32OpenGLRenderManager.h",	
			"../../../bullet2/LinearMath/btConvexHullComputer.cpp",
			"../../../bullet2/LinearMath/btConvexHullComputer.h",
			"../../../bullet2/LinearMath/btSerializer.cpp",
			"../../../bullet2/LinearMath/btSerializer.h",
			"../../../bullet2/LinearMath/btAlignedAllocator.cpp",
			"../../../bullet2/LinearMath/btQuickprof.cpp",
			"../../../bullet2/LinearMath/btQuickprof.h"
	}
		