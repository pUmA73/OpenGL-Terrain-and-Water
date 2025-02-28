# OpenGL-Terrain-and-Water
OpenGL Terrain generator that uses heightmaps + water rendering

# Concept
The project is a 3D graphics application that constructs and renders terrain using heightmaps. Additionally, Frame Buffer Objects are used to generate the necessary textures (reflection and refraction) for rendering water.

# Terrain Implementation
Initially, a grid of 20x20 (400) control patches is used for the terrain, each with four corner points. These patches are sent to the tessellation control shader (TCS) to manage the tessellation level for each one. The tessellation level is dynamically adjusted based on the distance from the camera to control the level of detail. Closer patches are rendered with higher detail, while distant ones use fewer subdivisions.

The next step in the pipeline is the tessellation evaluation shader (TES). Intermediate points are generated through tessellation in the tessellation primitive generator (which does not require explicit shader code but uses TCS output and TES input). TES calculates the final position of the vertices generated through tessellation. This process involves interpolating control point locations, calculating the normal for each control patch, and displacing the generated point along the normal using values extracted from the heightmap. TES also computes texture coordinates using bilinear interpolation between the four patch corner points.

Texturing occurs in the fragment shader and is applied based on each point's height—higher elevations receive different representative textures.

# Water Implementation
To render water, both reflection and refraction textures are needed. These textures are generated through two render passes (executed before rendering the terrain and water) using different frame buffers instead of the default screen buffer. This method allows saving refraction and reflection textures and using them to texture the water surface.

To generate these two textures, a clipping plane is used to dictate which part of the terrain geometry is visible. For the reflection texture, the camera is inverted (moved below the water surface), and the clipping plane removes the geometry below the water level. For the refraction texture, the camera remains above the water, and the clipping plane removes the geometry above the water level.

To simulate water movement, a dudv texture is used, along with an offset updated in each frame to modify the coordinates used by the texture sampler.

The Fresnel effect is also implemented for the water surface. According to this effect, the smaller the angle between the camera and the water surface, the stronger the reflection, while a larger angle results in stronger refraction.

# Screenshots
![image](https://github.com/user-attachments/assets/e4722117-b791-47d4-8676-6680f4d1511f)

![image](https://github.com/user-attachments/assets/567139bc-b30a-436a-b441-9923b8908a69)

Images of the same spot, one from above and one from a shallow angle, Fresnel effect can be observed

![image](https://github.com/user-attachments/assets/247792e2-fd52-4927-9f47-dffc2db16acb)

![image](https://github.com/user-attachments/assets/984f3064-af83-4319-b438-dab4a7fe8b0a)

Terrain generated using different heightmap

![image](https://github.com/user-attachments/assets/c2dca3be-ac19-4d13-9cf1-bec78424e888)

Left to implement: Soft edges for the water surface

Note: Textures for the terrain and heightmaps have not been included in this repository.



