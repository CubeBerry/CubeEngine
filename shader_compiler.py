import os
import subprocess
import sys

PROJECT_ROOT = os.getcwd()
SHADER_ROOT = os.path.join("Engine", "shaders")

TOOLS_DIR_NAME = "Tools"

DXC_DIR_NAME = "dxc_2025_07_14"
DXC_PATH = os.path.join(PROJECT_ROOT, TOOLS_DIR_NAME, DXC_DIR_NAME, "bin", "x64", "dxc.exe")

SLANG_DIR_NAME = "slang-2025.24.1-windows-x86_64"
SLANG_PATH = os.path.join(PROJECT_ROOT, TOOLS_DIR_NAME, SLANG_DIR_NAME, "bin", "slangc.exe")

DIRS = {
	"slang": "slang",
	"hlsl": "hlsl",
	"glsl": "glsl",
	"spirv": "spirv",
	"cso": "cso"
}

# Slang Shader List
# Edit this list to add/delete shaders for compile
SLANG_SHADERS = [
	# 2D
	{ "file": "2D.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "2D.vert" },
	{ "file": "2D.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "2D.frag" },
	# 3D
	{ "file": "3D.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "3D.vert" },
	{ "file": "3D.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "3D.frag" },
	# For Mesh Shader
	{ "file": "3D.slang", "entry": "meshMain", "stage": "mesh", "profile_hlsl": "sm_6_5", "out_name": "3D.mesh" }, # Also need to convert to DXIL
	{ "file": "3D.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_6_5", "out_name": "3DMesh.frag" }, # Also need to convert to DXIL
	# Shadow Map Pass
	{ "file": "ShadowMapPass.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "ShadowMapPass.vert" },
	{ "file": "ShadowMapPass.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "ShadowMapPass.frag" },
	{ "file": "ConvolutionBlur.slang", "entry": "computeMain", "stage": "compute", "profile_hlsl": "sm_5_1", "out_name": "ConvolutionBlur.compute" },
	# Geometry Pass, Use totally same vertex and mesh shader but different pixel shader
	{ "file": "GBuffer.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "GBuffer.frag" },
	{ "file": "GBuffer.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_6_5", "out_name": "GBufferMesh.frag" }, # Also need to convert to DXIL
	# Naive Lighting Pass
	{ "file": "NaiveLightingPass.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "NaiveLightingPass.vert" },
	{ "file": "NaiveLightingPass.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "NaiveLightingPass.frag" },
	# Global Lighting Pass
	{ "file": "GlobalLightingPass.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "GlobalLightingPass.vert" },
	{ "file": "GlobalLightingPass.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "GlobalLightingPass.frag" },
	# Local Lighting Pass
	{ "file": "LocalLightingPass.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "LocalLightingPass.vert" },
	{ "file": "LocalLightingPass.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "LocalLightingPass.frag" },
	# Normal Vector Visualization
	{ "file": "Normal3D.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "Normal3D.vert" },
	{ "file": "Normal3D.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "Normal3D.frag" },
	# IBL
	{ "file": "Cubemap.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "Cubemap.vert" },
	{ "file": "Equirectangular.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "Equirectangular.frag" },
	{ "file": "Irradiance.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "Irradiance.frag" },
	{ "file": "Prefilter.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "Prefilter.frag" },
	{ "file": "BRDF.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "BRDF.vert" },
	{ "file": "BRDF.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "BRDF.frag" },
	# Skybox
	{ "file": "Skybox.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "Skybox.vert" },
	{ "file": "Skybox.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "Skybox.frag" },
	# Tone Mapping (Post-Process)
	{ "file": "ToneMapping.slang", "entry": "vertexMain", "stage": "vertex", "profile_hlsl": "sm_5_1", "out_name": "ToneMapping.vert" },
	{ "file": "ToneMapping.slang", "entry": "fragmentMain", "stage": "fragment", "profile_hlsl": "sm_5_1", "out_name": "ToneMapping.frag" },
	# Compute
	{ "file": "Compute.slang", "entry": "computeMain", "stage": "compute", "profile_hlsl": "sm_5_1", "out_name": "Compute.compute" }
]

# DXC Shader List
# Edit this list to add/delete shaders for compile
DXC_SHADERS = [
	# Mesh Shaders
	{ "file": "3D.mesh.hlsl", "entry": "meshMain", "profile": "ms_6_5", "out_name": "3D.mesh.cso" },
	{ "file": "3DMesh.frag.hlsl", "entry": "fragmentMain", "profile": "ps_6_5", "out_name": "3DMesh.frag.cso" },
	# Geometry Pass, Use totally same vertex and mesh shader but different pixel shader
	{ "file": "GBufferMesh.frag.hlsl", "entry": "fragmentMain", "profile": "ps_6_5", "out_name": "GBufferMesh.frag.cso" },
	# Work Graphs
	{ "file": "WorkGraphs.hlsl", "entry": "broadcastNode", "profile": "lib_6_8", "out_name": "WorkGraphs.cso" },
	# Work Graphs Frustum Culling
	{ "file": "WorkGraphsFrustumCulling.lib.hlsl", "entry": "CullNode", "profile": "lib_6_9", "out_name": "WorkGraphsFrustumCulling.lib.cso" },
	{ "file": "WorkGraphsFrustumCulling.frag.hlsl", "entry": "pixelMain", "profile": "ps_6_6", "out_name": "WorkGraphsFrustumCulling.frag.cso" }
]

def run_command(cmd, description, output_file=None):
	if output_file and os.path.exists(output_file):
		try:
			os.remove(output_file)
			print(f"[Info]: Deleted old file '{output_file}'")
		except OSError as e:
			print(f"[Warning]: Could not delete old file '{output_file}': {e}")

	try:
		result = subprocess.run(cmd, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, errors='replace')

		output_msg = result.stdout.strip()
		error_msg = result.stderr.strip()
		full_log = ""
		if output_msg: full_log += output_msg + "\n"
		if error_msg: full_log += error_msg

		if result.returncode == 0:
			print(f"[Success]: {description}")
			if full_log: print(f"[Warning]: \n{full_log}")
			return True
		else:
			print(f"[Failed]: {description}")
			print(f"[Failed Command]: {' '.join(cmd)}")
			print(f"[Error]: \n{full_log}")
	except FileNotFoundError:
		print(f"[Error]: Tool not found for command '{cmd[0]}'")
		return False
	except OSError as e:
		print(f"[Error]: OS Error: {e}")
		return False

def check_dirs():
	if not os.path.exists(SHADER_ROOT):
		print(f"[Error]: Root shader directory '{SHADER_ROOT}' not found.")
		return False

	for key, folder in DIRS.items():
		path = os.path.join(SHADER_ROOT, folder)
		if not os.path.exists(path):
			print(f"[Info]: Creating directory: {path}")
			os.makedirs(path)
	return True

def main():
	print(f"Starting Shader Compilation in: {SHADER_ROOT}\n")
	print(f"Project Root: {PROJECT_ROOT}")
	print(f"Tools Directory: {os.path.join(PROJECT_ROOT, TOOLS_DIR_NAME)}")
	print(f"Slang Path: {SLANG_PATH}")
	print(f"DXC Path: {DXC_PATH}\n")

	if not os.path.exists(SLANG_PATH):
		print(f"[Error]: 'slangc.exe' not found.")
		print(f"Expected path: {SLANG_PATH}")
		sys.exit(1)

	# Requires dxc supports Shader Model 6.9 & Mesh Nodes Support Preview
	# https://github.com/microsoft/DirectXShaderCompiler/releases/tag/v1.8.2405-mesh-nodes-preview
	# Above dxc version does not work with mesh shader and work graphs shader. Use latest stable version for now
	if not os.path.exists(DXC_PATH):
		print(f"[Error]: 'dxc.exe' not found.")
		print(f"Expected path: {DXC_PATH}")
		sys.exit(1)

	if not check_dirs():
		sys.exit(1)

	os.chdir(SHADER_ROOT)

	success_count = 0
	fail_count = 0

	# Slang
	print("--- [Step 1] Compiling Slang Shaders ---")
	for shader in SLANG_SHADERS:
		infile = os.path.join(DIRS["slang"], shader["file"])

		# To SPIR-V (.spv)
		outfile_spv = os.path.join(DIRS["spirv"], f"{shader['out_name']}.spv")
		cmd_spv = [
			SLANG_PATH, infile,
			"-profile", "glsl_460",
			"-entry", shader["entry"],
			"-stage", shader["stage"],
			"-target", "spirv",
			"-o", outfile_spv
		]
		if run_command(cmd_spv, f"SPV: {infile} -> {outfile_spv}", outfile_spv): success_count += 1
		else: fail_count += 1

		# To HLSL (.hlsl)
		outfile_hlsl = os.path.join(DIRS["hlsl"], f"{shader['out_name']}.hlsl")
		cmd_hlsl = [
			SLANG_PATH, infile,
			"-profile", shader["profile_hlsl"],
			"-entry", shader["entry"],
			"-stage", shader["stage"],
			"-target", "hlsl",
			"-o", outfile_hlsl,
			"-D__hlsl__"
		]
		if run_command(cmd_hlsl, f"HLSL: {infile} -> {outfile_hlsl}", outfile_hlsl): success_count += 1
		else: fail_count += 1

	print("\n--- [Step 2] Compiling HLSL to DXIL ---")
	# DXC HLSL To DXIL (.hlsl to .cso)
	for shader in DXC_SHADERS:
		infile = os.path.join(DIRS["hlsl"], shader["file"])
		outfile = os.path.join(DIRS["cso"], shader["out_name"])
		cmd_dxc = [
			DXC_PATH, infile,
			"-T", shader["profile"],
			"-E", shader["entry"],
			"-Fo", outfile
		]
		if run_command(cmd_dxc, f"DXC: {infile} -> {outfile}", outfile): success_count += 1
		else: fail_count += 1

	print("\n" + "="*40)
	print(f"All Jobs Finished.")
	print(f"Success: {success_count}")
	print(f"Failed: {fail_count}")
	print("="*40)

	if fail_count > 0: sys.exit(1)

if __name__ == "__main__": main()