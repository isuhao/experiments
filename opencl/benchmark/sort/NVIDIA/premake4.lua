	
	hasCL = findOpenCL_NVIDIA()
	hasDX11 = findDirectX11()
	
	if (hasCL) then

		project "OpenCL_radixsort_benchmark_NVIDIA"

		initOpenCL_NVIDIA()

		if (hasDX11) then
			initDirectX11()
		end
		
		language "C++"
				
		kind "ConsoleApp"
		targetdir "../../../../bin"
		includedirs {"..",projectRootDir .. "bullet2"}
		
		links {
			"OpenCL","LinearMath"
		}
		
		files {
			"../../../basic_initialize/btOpenCLUtils.cpp",
			"../../../basic_initialize/btOpenCLUtils.h",
			"../../../broadphase_benchmark/btFillCL.cpp",
			"../../../broadphase_benchmark/btPrefixScanCL.cpp",
			"../../../broadphase_benchmark/btRadixSort32CL.cpp",
			"../test_large_problem_sorting.cpp"
		}
		
	end