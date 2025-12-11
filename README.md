# 🌌 **Goku on King Kai’s Planet — A Stylized Deferred Rendering Engine**
*A real-time rendering pipeline featuring spherical gravity, aura particles, LUT color grading, and anime-style post-processing.*

---

## 🐉 **Overview**

**Goku on King Kai’s Planet** is a world built and written in C++ and OpenGL.

This project recreates **King Kai’s iconic mini-planet** and implements:

- A full **deferred shading pipeline**
- A modular **post-processing engine**
- **Stylized shader filters** (toon shading, edge detection, pixelation, etc.)
- **3D LUT color grading** for cinematic anime visuals
- A **aura particle system** inspired by Dragon Ball power-ups
- **Spherical gravity** so Goku can walk all the way around the planet

It combines graphics programming, physics, and stylized aesthetics into a cohesive real-time engine.

---

## 🌀 **Major Features**

---
### 🕳️ **Depth Buffers**

Depth buffers ensure correct occlusion and ordering of 3D objects.  

- Depth testing guarantees nearer fragments overwrite farther ones  

---

### 🧱 **G-Buffers (Geometry Buffers)**

The engine implements a multi-attachment G-buffer for deferred shading:

- `gPosition` → world-space positions  
- `gNormal` → world-space normals  
- `gDiffuse` → albedo + roughness information  
- `gSpec` → specular reflectance  
- `depthTexture` → precise depth data  

During the geometry pass, each pixel writes material and geometric attributes into these buffers.  
This enables lighting to occur entirely in screen space, decoupling geometry from shading and allowing multiple dynamic lights at low cost.


### 🔷 **Deferred Rendering (G-Buffer Architecture)**

The engine uses a classical **deferred shading pipeline** with multiple render targets:

| Buffer        | Description              |
|---------------|--------------------------|
| `gPosition`   | World-space position     |
| `gNormal`     | World-space normals      |
| `gDiffuse`    | Albedo + roughness       |
| `gSpec`       | Specular response        |
| `depthTexture`| High-precision depth     |

Lighting is computed **per pixel** via a fullscreen quad, enabling:

- Large numbers of dynamic lights  
- Clean separation between geometry + shading  
- Efficient rendering of complex meshes (like Goku)

---

### 🌳 **Procedural L-System Trees**

Generate trees across King Kai’s planet using L-systems:

- Recursive branch growth via string rewriting  
- Turtle graphics interpreter converts rules into branch CTMs  
- Randomized angle variance for natural variation  
- Multiple trees scattered across the planet surface

---

### 🌌 **King Kai’s Skybox**

- A high-resolution cubemap recreates the soft pink-blue sky of King Kai’s world
- Blends beautifully with stylized LUT modes

---

### 🔥 **Goku’s Aura Particle System**

A real-time particle system simulates Goku when he learns Kaio-Ken:

- Particle motion around the player  
- Per-particle velocity, color, size, and lifetime  
- Follows Goku around the whole planet  
- Stacks visually with toon shading + LUT filters

---

### 🌍 **Spherical Gravity (Mario Galaxy–Style Movement)**

Inspired by Super Mario Galaxies Traversal system, we replicated it for Goku:

- Player orientation aligns to surface normals  
- Tangential local movement  
- Radial velocity to keep him grounded  
- Camera follow system wraps around the planet smoothly  

---
### 🤖 **Rigid Body Translations**

The engine uses a simplified rigid body model for player movement and object motion:

- Each object stores velocity, mass, and positional state  
- Collision correction keeps Goku above the planetary surface  
- Rigid bodies interact seamlessly with the rendering pipeline  

---

### 🗿 **OBJ Loader**

- Uses **TinyOBJLoader** to import Goku’s 3D model.

---

### 🎨 **Stylized Post-Processing Pipeline**

After lighting, the scene is written into a post-processing framebuffer:

A fullscreen shader applies real-time stylized effects:

- **Toon shading**
- **Edge detection**
- **Pixelation**
- **Grayscale**
- **Invert**
- **Vignette**
- **Depth visualization**
- **Stackable filters**

---

### 🎛 **3D LUT Color Grading (Anime Color Modes)**

We used standard `.cube` LUT files for color grading, but wrote a custom parser to load the LUT data and convert it into a 3D texture usable by our post-processing shader.

Included LUTs:

- **Ancient Orange** — An orange tint.  
- **Lush Green** — Boosts the strengths of the Greens.
- **Orange & Blue** — The best looking LUT helps bring out all the colors in the scene.


---

## 🎮 **Controls**

### Movement
| Key | Action |
|-----|---------|
| Up / Down | Move Goku |
| Left / Right | Rotate Goku / Rotate Camera (No Strafing)|

### Post-Processing Toggles
| Key | Effect |
|-----|--------|
| **T** | Toon shading |
| **E** | Edge detection |
| **G** | Grayscale |
| **I** | Invert colors |
| **V** | Vignette |
| **P** | Pixelation |
| **Z** | Depth visualization |
| **L** | Cycle LUT modes |
| **C** | Clear all effects |

---

