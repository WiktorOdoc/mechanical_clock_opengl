# OpenGL Mechanical Clock

This project is a 3D mechanical clock simulation created using OpenGL and GLSL.  
It allows real-time interaction, camera movement, and time manipulation.
The project uses custom GLSL shaders located in the `shaders/` folder.  

---

<img width="1088" height="972" alt="clock" src="https://github.com/user-attachments/assets/51f82a2d-5701-4b7e-9fee-8919395e647b" />

---

## Controls

### Camera movement:
- **W A S D** – move around the scene  
- **Space** – move up  
- **Left ALT / Left CTRL** – move down  

### Camera rotation:
- **E / Q** – rotate right / left  
- **2 / X** – rotate up / down  

### Model rotation:
- **Arrow keys** – rotate the clock model  



## Time Control

- **=** – speed up time  
- **-** – slow down time  
- **Backspace** – reset time speed to normal  
- **N** – set clock to current system time  



## Libraries Used

This project uses the following libraries:

- **OpenGL** – graphics rendering  
- **GLFW** – window creation and input handling  
- **GLEW** – OpenGL extensions loading  
- **GLM** – mathematics library for graphics  
- **Assimp** – model loading  

> These libraries are NOT included in the repository, please install them manually.


## Models and Textures Sources

### Models:
- Cylinder: https://www.cgtrader.com/items/5734106/download-page  
- Tooth: https://sketchfab.com/3d-models/prisma-triangular-b4310db268de46f7ae1138ca60243c70  
  *(gears were assembled in code using the above models)*  
- Pendulum: https://sketchfab.com/3d-models/hogwarts-clocktower-pendulum-31e0450444594a17b26b7a6d82a4b869  
- Cube: https://sketchfab.com/3d-models/simple-cube-in-obj-fbx-format-d155c6dd450d4d48b9a17b35e1550c2f  
- Arrow: https://sketchfab.com/3d-models/jjba-stand-arrow-fixed-d8b66fa07f134ea2b00efba4275c0627  
- Sun: https://sketchfab.com/3d-models/sun-9ef1c68fbb944147bcfcc891d3912645  
- Wall: https://sketchfab.com/3d-models/stylized-nordic-wooden-wall-section-03-253651b57d4e48b18fdd4e8444d3c051  
- Mask: https://sketchfab.com/3d-models/jojo-ishikamen-stone-mask-b23b52dfca2545db92e5a57877df0118  
- Plush: https://sketchfab.com/3d-models/frieren-plushie-209c79c641164b38a81e145b6af3f890  

---

### Additional textures:
- Metal1, Metal2, Metal3:  
  https://seamless-pixels.blogspot.com/2012/09/free-seamless-metal-textures_28.html  

- Wood:  
  https://t4.ftcdn.net/jpg/04/33/77/23/360_F_433772331_iPo5eqvn3bqhKB3vBpRPyGVKvUaDs9CQ.jpg  

- Clock face:  
  https://thumbs.dreamstime.com/b/antique-clock-face-5828362.jpg  

- Sky:  
  https://media.istockphoto.com/id/938695332/photo/fluffy-clouds-hd-seamless-tiles-pattern-01.jpg  

---

## Build Instructions

1. Install required libraries:
   - OpenGL
   - GLFW
   - GLEW
   - GLM
   - Assimp

2. Open the `.sln` file in Visual Studio

3. Build and run the project
