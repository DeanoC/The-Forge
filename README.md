# ![The Forge Logo](Screenshots/TheForge-on-white.jpg)

This is a derivative of ConfettiFX The Forge, this version has a pure C interface and just the renderer portion and uses CMake and my own al2o3 framework for some parts (TODO reduce dependency).

## This version is currently based on upstream Forge Release 1.45

The Forge is a cross-platform rendering framework supporting
- PC 
  * Windows 10 
     * with DirectX 12 / Vulkan 1.1
     * with DirectX Ray Tracing API
     * DirectX 11 Fallback Layer for Windows 7 support (not extensively tested)
  * TODO Linux Ubuntu 18.04 LTS with Vulkan 1.1 and RTX Ray Tracing API
- Android Pie with Vulkan 1.1
- macOS / iOS / iPad OS with Metal 2.2

Particularly, the graphics layer of The Forge supports cross-platform
- Descriptor management
- Multi-threaded and asynchronous resource loading
- Shader reflection
- Multi-threaded command buffer generation

### Basic Usage
Clone https://github.com/DeanoC/The-Forge-AL2O3-Simple.git and load the CMakeLists.txt file into CLion or VS2019 (or use any other CMake system) and build. First build will take a while as it downloads and compiles everthing.

### Resources
Currently the CAPI is configured to look in the following directory for various resources. This isn't ideal and would like to make it configurable.
* binshaders/                                FSR_BinShaders
* srcshaders/                                FSR_SrcShaders
* textures/                                  FSR_Textures
* meshes/                                    FSR_Meshes
* fonts/                                     FSR_Builtin_Fonts
* gpuconfigs/                                FSR_GpuConfig
* misc/                                      FSR_OtherFiles

### Info

Please find a link and credits for all open-source packages used at the https://github.com/ConfettiFX/The-Forge/README.md.

The Forge proper has a discord channel, I hang out there so if you have questions about this fork, check for DeanoC at

Discord: TheForge now offer support through a discord channel. Sign up here: 
<a href="https://discord.gg/hJS54bz" target="_blank"><img src="Screenshots/Discord.png" 
 width="20" height="20" border="0" /> The Forge Discord Channel

