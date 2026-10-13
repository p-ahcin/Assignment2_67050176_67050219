# Computer Graphics Final Project — Future Me

โปรเจกต์งานคู่รหัสนักศึกษา **67050176** และ **67050219** สำหรับวิชา Computer Graphics

เริ่มจาก [OpenGLStarter ของอาจารย์](https://github.com/momotheoxy/OpenGLStarter) ซึ่งใช้ C++17, OpenGL 3.3, GLEW, GLFW, GLM, CMake และ vcpkg

## เป้าหมายของงาน

สร้างฉาก 3 มิติหัวข้อ **FUTURE ME** ด้วย vertices/โมเดลใน Modern OpenGL โดยมี texture และ lighting model สามารถใช้โมเดลและ texture ภายนอกได้

## เปิดและ build

1. เปิดโฟลเดอร์นี้ใน VS Code โดยตรง (โฟลเดอร์ที่มี `CMakeLists.txt`)
2. ติดตั้งส่วนขยาย **C/C++** และ **CMake Tools** ของ Microsoft
3. เลือก **CMake: Select Configure Preset** แล้วเลือก preset ที่ตรงกับเครื่อง เช่น **Windows MSVC (Visual Studio 2026)**
4. กด **F7** เพื่อ build และ **F5** เพื่อ run

การตั้งค่า compiler, CMake และ vcpkg สำหรับ Windows/macOS/Linux ดูได้ใน [README ของ OpenGLStarter](https://github.com/momotheoxy/OpenGLStarter#readme) โฟลเดอร์ `build/` เป็นไฟล์ที่สร้างจากเครื่องแต่ละคนและไม่ต้อง commit

## โครงสร้างที่จะใช้ต่อ

- `src/main.cpp` : โค้ดตั้งต้นสำหรับสร้างหน้าต่างและวาดฉาก
- `src/Libs/` : คลาส Window, Shader และ Mesh จาก OpenGLStarter
- `Shaders/` : vertex shader และ fragment shader
- `Models/` : โมเดล OBJ ที่ใช้
- `Textures/` : texture ที่ใช้

เมื่อเพิ่มโมเดลหรือ texture ให้ commit ไฟล์ที่โปรแกรมต้องใช้ และเก็บแหล่งที่มาของทรัพยากร/ภาพแรงบันดาลใจไว้สำหรับการอ้างอิงตอนส่งงาน

## เตรียมโมเดลด้วย Blender

เลือกโมเดลที่สามารถแปลงเป็น mesh และ triangulate faces ได้ แล้ว Export เป็น Wavefront `.obj` โดยเลือกตัวเลือก Geometry ตามที่อาจารย์สอน:

- UV Coordinates
- Curves as NURBS
- Triangulated Mesh
- Apply Modifiers
- Apply Transform

ตรวจให้มี **Normals** ในไฟล์ที่ Export ด้วย เพราะ lighting model ใน Lab12 ต้องใช้ normal vector ตัวอ่าน OBJ ใน Lab10 ใช้ข้อมูล `v`, `vt`, `vn` และ `f`; ถ้าโมเดลมี curve/NURBS ให้แปลงส่วนที่จะวาดเป็น mesh ก่อน เพราะตัวอ่านนี้ไม่ได้วาดคำสั่ง NURBS โดยตรง หลัง Export ให้ทดลองโหลด `.obj` ในโปรแกรมจริงและตรวจ texture/ทิศทางของโมเดล
