import os
import subprocess
import sys

SHADER_ROOT = os.path.join("Engine", "shaders")

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
	{
		"file": "Skybox.slang",
		"entry": "vertexMain",
		"stage": "vertex",
		"profile_hlsl": "sm_5_1",
		"out_name": "Skybox.vert"
	},
	{
		"file": "Skybox.slang",
		"entry": "fragmentMain",
		"stage": "fragment",
		"profile_hlsl": "sm_5_1",
		"out_name": "Skybox.frag"
	},
]

# DXC Shader List
# Edit this list to add/delete shaders for compile
DXC_SHADERS = [
	{
		"file": "WorkGraphs.hlsl",
		"entry": "broadcastNode",
		"profile": "lib_6_8",
		"out_name": "WorkGraphs.cso"
	}
]

def run_command(cmd, description, output_file=None):
	if output_file and os.path.exists(output_file):
		try:
			os.remove(output_file)
			print(f"[Info]: Deleted old file '{output_file}'")
		except OSError as e:
			print(f"[Warning]: Could not delete old file '{output_file}': {e}")

	try:
		subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
		print(f"[Success]: {description}")
		return True
	except subprocess.CalledProcessError as e:
		print(f"[Failed]:  {description}")
		print(f"[Error]: {e.stderr.strip()}")
		return False
	except FileNotFoundError:
		print(f"[Error]: Tool not found for command '{cmd[0]}'")
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
	print(f"[Info]: Starting Shader Compilation in: {SHADER_ROOT}\n")

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
			"slangc", infile,
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
			"slangc", infile,
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
			"dxc", infile,
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