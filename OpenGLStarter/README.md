# Future Me — งานศพของฉัน

Final Project วิชา **Computer Graphics** ของ **67050176 ณิชาพร อุทธยาน** และ **67050219 ธนัชนันท์ บุญเกษม** สร้างฉาก 3 มิติด้วย **C++17 และ Modern OpenGL 3.3** ในหน้าต่างขนาด **800 × 600**

## แนวคิดและองค์ประกอบ

ตีความหัวข้อ **FUTURE ME** ผ่านฉาก “งานศพของฉัน” โดยมีแมวสามตัวร่วมอยู่ในฉาก เพราะเราทั้งสองคนชอบแมวมาก อนาคตที่เป็นงานสำคัญครั้งสุดท้ายในชีวิตอยากให้แมวที่รักมาอยูู่ส่ง 

ใช้แสงเทียนสีอุ่นเป็นแหล่งกำเนิดแสงหลัก ประกอบด้วยพื้นแพลตฟอร์ม โลงศพพร้อมฐานและดอกไม้ พวงหรีด โต๊ะหมู่บูชา อาสนะพระ โต๊ะหน้าโลง ธูป เทียน และแมวทั้ง 3 ตัว

โมเดลจัดฉากและเตรียมใน Blender แล้วส่งออกเป็น OBJ พร้อมข้อมูล UV, normals และวัสดุ ส่วนภาพที่แสดงขณะรันคำนวณด้วย OpenGL และ GLSL shaders

## เริ่มใช้งาน

### เครื่องมือที่ต้องมี

- การ์ดจอและไดรเวอร์ที่รองรับ OpenGL 3.3 Core
- Visual Studio 2026 หรือ Build Tools พร้อม workload **Desktop development with C++**; มี preset สำหรับ Visual Studio 2022 ด้วย
- CMake **4.2 ขึ้นไปสำหรับ preset Visual Studio 2026**
- Git และ vcpkg สำหรับจัดการ GLEW, GLFW, GLM และ stb ตาม [vcpkg.json](vcpkg.json)
- หากใช้ VS Code: ส่วนขยาย **C/C++** และ **CMake Tools** ของ Microsoft

หากยังไม่มี vcpkg ให้ติดตั้งใน PowerShell:

```powershell
git clone https://github.com/microsoft/vcpkg "$env:USERPROFILE\vcpkg"
& "$env:USERPROFILE\vcpkg\bootstrap-vcpkg.bat"
```

CMake จะค้นหา vcpkg ที่ตำแหน่งนี้ให้เอง หากติดตั้งไว้ที่อื่นให้ตั้ง `VCPKG_ROOT` เป็นโฟลเดอร์ vcpkg ก่อน configure โดย dependencies จะติดตั้งระหว่าง configure ครั้งแรก

### Build และ run ด้วย VS Code

1. เปิดโฟลเดอร์ **OpenGLStarter** ซึ่งมี `CMakeLists.txt` โดยตรง
2. ใช้ **CMake: Select Configure Preset** เลือก **Windows MSVC (Visual Studio 2026)** หรือรุ่น 2022 ให้ตรงกับเครื่อง
3. ใช้ **CMake: Configure** แล้วกด **F7** เพื่อ build
4. เลือก target `OpenGLStarter` และใช้ **CMake: Run Without Debugging** หรือ **F5** เพื่อเริ่ม debug ด้วย configuration ของ Windows

> [!NOTE]
> `.vscode/settings.json` ระบุ `cmake.cmakePath` ของ Visual Studio Community 2026 ไว้สำหรับเครื่องที่ใช้งานอยู่ หากเครื่องอื่นติดตั้งต่างตำแหน่ง ให้ปรับค่านี้เป็น CMake ของเครื่องนั้น

### Build และ run ด้วย PowerShell

รันคำสั่งต่อไปนี้จากโฟลเดอร์ `OpenGLStarter` โดยต้องเรียก `cmake` ได้จาก Terminal:

```powershell
cmake --preset win-msvc-2026
cmake --build --preset build-win-2026-debug
& ".\build\win-msvc-2026\Debug\OpenGLStarter.exe"
```

สำหรับ Visual Studio 2022 ใช้ `win-msvc-2022`, `build-win-2022-debug` และ executable ใน `build/win-msvc-2022/Debug/` แทน ส่วน Release ใช้ build preset `build-win-2026-release` และ executable ในโฟลเดอร์ `Release/`

มี preset สำหรับ macOS และ Linux อยู่ใน [CMakePresets.json](CMakePresets.json) แต่ขั้นตอนด้านบนเน้น Windows ดูการเตรียมเครื่องระบบอื่นได้จาก [OpenGLStarter ของอาจารย์](https://github.com/momotheoxy/OpenGLStarter#readme)

โปรแกรมอ้างตำแหน่ง `Models`, `Textures` และ `Shaders` จาก header ที่ CMake สร้างสำหรับแต่ละเครื่อง เมื่อย้ายหรือ clone โปรเจกต์ไปที่ใหม่ ให้ configure และ build ใหม่ การคัดลอกเฉพาะ executable ไปเครื่องอื่นยังไม่ใช่ชุดโปรแกรมแบบพกพา

## การควบคุมกล้อง

| การควบคุม | ผลลัพธ์ |
|---|---|
| เลื่อนเมาส์ในหน้าต่าง | หมุนมุมมองซ้าย–ขวาและก้ม–เงย โดยไม่ต้องกดปุ่มเมาส์ |
| `W` / `S` | เดินหน้า / ถอยหลังตามทิศที่มอง |
| `A` / `D` | เลื่อนไปทางซ้าย / ขวา |
| `Space` | กลับตำแหน่งและทิศทางกล้องเริ่มต้นที่มองกลางฉาก |
| ปุ่มปิดหน้าต่าง | ออกจากโปรแกรม |

ความเร็วกล้องปัจจุบันคือ 50 หน่วยต่อวินาที และความไวเมาส์ 0.1 ตั้งค่าใน [src/main.cpp](src/main.cpp)

กล้องเริ่มที่ `(-5.28, 110, 330)` มองไปยัง `(-5.28, 16, 71.90)` ใช้มุมมอง perspective 25 องศา

## โครงสร้างไฟล์

โค้ดและทรัพยากรสำหรับรันอยู่ในโฟลเดอร์ `OpenGLStarter/` ของ repository นี้ เส้นทางใน README อ้างจากโฟลเดอร์นี้:

```text
Assignment2_67050176_67050219/
└── OpenGLStarter/
    ├── README.md
    ├── src/
    │   ├── main.cpp                 # โหลดฉาก กล้อง แสง และลำดับการวาด
    │   ├── ProjectPaths.h.in        # แม่แบบตำแหน่ง assets สำหรับ CMake
    │   └── Libs/                    # Mesh, Shader และ Window
    ├── Shaders/
    │   ├── shader.vert              # vertex shader หลัก
    │   ├── shader.frag              # texture แสง และเงาของฉาก
    │   ├── depthShader.vert         # vertex shader สำหรับรอบเงา
    │   ├── depthShader.frag         # บันทึกระยะจากแสงลง depth cubemap
    │   └── bgShader.*, lightShader.* # ตัวอย่างเดิมที่ไม่ได้เรียกใช้ปัจจุบัน
    ├── Models/
    │   ├── <ชื่อกลุ่ม>/             # OBJ และ MTL ของแต่ละองค์ประกอบ
    │   └── scene_manifest.json      # ข้อมูล export ไม่ได้อ่านตอนรัน
    ├── Textures/<ชื่อกลุ่ม>/        # ภาพประกอบของแต่ละองค์ประกอบ
    ├── .vscode/                    # การตั้งค่า VS Code และ debugger
    ├── .gitignore
    ├── CMakeLists.txt
    ├── CMakePresets.json
    └── vcpkg.json
```

แผนผังแสดงไฟล์หลักที่ใช้พัฒนาและรันโปรเจกต์ ส่วน `build/` สร้างขึ้นเมื่อ configure/build และถูกละเว้นจาก Git ไฟล์ Blender ที่ใช้เตรียมฉากเก็บแยกจาก repository; ผู้ที่ clone สามารถ build และรันจาก OBJ/MTL, texture และ shaders ที่ให้มาได้โดยไม่ต้องมี Blender

### การโหลดโมเดลและ texture

`CreateScene()` ระบุ 14 กลุ่ม แล้ว `LoadSceneGroup()` ค้นหาและเรียงไฟล์ OBJ ในแต่ละโฟลเดอร์ ก่อนส่งให้ `LoadScenePart()` อ่าน MTL ชื่อเดียวกัน ชุดปัจจุบันมี **178 คู่ OBJ/MTL และ 50 PNG** โดยบางภาพเป็นข้อมูลเสริมที่ shader ยังไม่ได้ใช้

| กลุ่ม | องค์ประกอบ |
|---|---|
| `platform` | พื้นรวมของฉาก |
| `coffin2` | โลงศพและส่วนประกอบ |
| `Flower1`, `Flower2`, `wreath` | ดอกไม้และพวงหรีด |
| `altar_table`, `monk_pltform` | โต๊ะหมู่บูชาและแท่นอาสนะพระ |
| `table`, `incense`, `candle1`, `candle2` | โต๊ะหน้าโลง ธูป และเทียน |
| `cat_american`, `cat_black`, `cat_fred` | แมวสามตัว |

- หนึ่ง OBJ ใช้หนึ่งวัสดุ ตัวช่วยอ่านเฉพาะ `Kd` (สี), `map_Kd` (ภาพสี) และ `map_d` (alpha) โดย alpha ต้องมาจากภาพเดียวกับภาพสี
- ส่วนไม่มีภาพใช้ `Textures/platform/plain_white.png` ร่วมกับสี `Kd` ปัจจุบันพื้นเป็นสีเทา `Kd = (0.8, 0.8, 0.8)` และยังไม่มีลายภาพบนพื้น
- ใช้ `textureCache` เพื่อไม่อัปโหลดภาพเดียวกันเข้า GPU ซ้ำ
- OBJ มีตำแหน่งจากฉากอยู่แล้ว จึงใช้ model matrix identity การแปลงแกนคือ Blender `(x, y, z)` → OpenGL `(x, z, -y)`
- ตัวโหลดกลับ UV แกน V เฉพาะสามกลุ่มแมวให้ตรงกับภาพที่ stb อ่าน ไม่ควรกลับภาพ texture ซ้ำ
- เพิ่ม OBJ/MTL ในกลุ่มเดิมแล้วเปิดโปรแกรมใหม่เพื่อโหลดได้ หากเพิ่มกลุ่มใหม่ ต้องเพิ่มชื่อใน `CreateScene()` ด้วย

## แสงเทียนและเงา

ใช้ **Point Light 4 จุด** ได้แก่เทียนหน้าโลงสองเล่มและเทียนบนโต๊ะหมู่บูชาสองเล่ม แสงกระจายรอบจุดกำเนิดและลดความสว่างตามระยะทาง พื้นและโมเดลทั้งหมดใช้ shader แสงเดียวกัน

| แหล่งกำเนิดแสง | ตำแหน่ง OpenGL `(x, y, z)` |
|---|---|
| `candle1` บนโต๊ะหน้าโลง | `(8.111, 11.24, 47.729)` |
| `candle2` บนโต๊ะหน้าโลง | `(-5.276, 11.24, 47.729)` |
| เทียนโต๊ะหมู่บูชาเล่มแรก | `(46.415, 20.15, 49.196)` |
| เทียนโต๊ะหมู่บูชาเล่มที่สอง | `(46.415, 20.15, 52.902)` |

ค่าปัจจุบันอยู่ช่วงต้นของ [src/main.cpp](src/main.cpp):

| ตัวแปร | ค่า / หน้าที่ |
|---|---|
| `candlePositions` | ตำแหน่งเปลวทั้งสี่ในพิกัด OpenGL |
| `candleColor` | สีอุ่น RGB `(1.0, 0.65, 0.32)` |
| `candleStrengths` | ความแรงแต่ละดวง `(5, 5, 4, 4)` |
| `ambientStrength` | แสงแวดล้อม `0.12` ช่วยให้เห็นส่วนที่ไกลเทียน |
| `attenuationLinear` / `attenuationQuadratic` | การลดแสงตามระยะ `0.07` / `0.017` |
| `SHADOW_SIZE` / `shadowFarPlane` | แผนที่เงา `1024 × 1024` ต่อหน้า / ระยะไกล `180` |

[shader.frag](Shaders/shader.frag) รวม ambient, diffuse และ specular แบบ Blinn–Phong กับเงาของแต่ละดวง ส่วน `CreateCandleShadows()` วาด depth cubemap หกทิศทางต่อเทียน รวม 24 มุม ก่อนเริ่ม loop เพราะฉากนิ่ง การเลื่อนกล้องจึงใช้แผนที่เงาเดิมได้ หากเพิ่มการขยับโมเดลหรือแสงระหว่างรัน ต้องคำนวณเงาใหม่

ใช้ alpha discard ในรอบสีและรอบเงา และเฉลี่ยเงาเจ็ด samples เพื่อลดขอบหยัก ภาพ normal, roughness และ AO ที่เก็บใน Textures ยังไม่ได้ใช้ในสูตรแสงปัจจุบัน ระบบนี้ยังไม่มีไฟกะพริบหรือ animation เปลวเทียน

## เมื่อพบปัญหา

- **ไม่พบเมนู CMake หรือ preset:** เปิดโฟลเดอร์ OpenGLStarter โดยตรง ตรวจว่าส่วนขยาย CMake Tools เปิดใช้งาน แล้วดู `cmake.cmakePath`
- **ไม่พบ vcpkg toolchain:** ตรวจตำแหน่งติดตั้งหรือ `VCPKG_ROOT` แล้ว configure ใหม่
- **โหลดโมเดล/texture ไม่ได้:** อ่านเส้นทางที่แจ้งใน Terminal ตรวจไฟล์ OBJ/MTL ชื่อคู่กันและเส้นทาง `map_Kd` / `map_d` ซึ่งอ้างจากตำแหน่ง MTL
- **ภาพมืดบริเวณไกลเทียน:** ตรวจค่าความแรงไฟ attenuation และ ambient ใน main.cpp
- **พื้นไม่มีลาย:** ชุดปัจจุบันตั้งใจโหลดสีวัสดุร่วมกับภาพขาว หากจะเพิ่มลาย ให้เพิ่มภาพและระบุ `map_Kd` ใน MTL ของพื้น

## ผู้จัดทำและแหล่งอ้างอิง

| รหัสนักศึกษา | ชื่อ–นามสกุล |
|---|---|
| 67050176 | ณิชาพร อุทธยาน |
| 67050219 | ธนัชนันท์ บุญเกษม |

### โค้ดและเครื่องมือ

- [OpenGLStarter — momotheoxy](https://github.com/momotheoxy/OpenGLStarter): โครงงานตั้งต้นของอาจารย์ และแนวทางจาก Lab10–Lab13 สำหรับ OBJ, texture, แสง กล้อง และ depth pass
- [GLFW](https://github.com/glfw/glfw), [GLEW](https://glew.sourceforge.net/), [GLM](https://github.com/g-truc/glm), [stb_image](https://github.com/nothings/stb): libraries ที่ใช้ในโค้ด
- [CMake](https://cmake.org/) และ [vcpkg](https://github.com/microsoft/vcpkg): configure/build และจัดการ dependencies
- [Blender](https://www.blender.org/): เตรียมโมเดลและส่งออก OBJ/MTL

### โมเดล ภาพ และแรงบันดาลใจ

นำโมเดลภายนอกมาจัดฉาก ปรับขนาดและรายละเอียดใน Blender แล้วส่งออกเป็น OBJ/MTL พร้อม texture สำหรับ OpenGL แหล่งดาวน์โหลดที่ผู้จัดทำยืนยันมีดังนี้:

| องค์ประกอบ | โมเดลและแหล่งดาวน์โหลด | การนำมาใช้ |
|---|---|---|
| ฉากงานศพ | [Thai funeral — 3D Warehouse](https://3dwarehouse.sketchup.com/model/a9b1499b-c792-4c1d-adc0-c24abc45ef78/Thai-funeral) | ฉากและองค์ประกอบงานศพ เตรียมแยกเป็นกลุ่มสำหรับโหลดในโปรแกรม |
| แมวดำ (`cat_black`) | [An Animated Cat — Sketchfab](https://sketchfab.com/3d-models/an-animated-cat-aec25699660043a29595f9572149d1e8) | ใช้เป็นโมเดลท่านิ่งในฉาก |
| แมววิเชียรมาศ Fred (`cat_fred`) | [Fred - The Cat — Sketchfab](https://sketchfab.com/3d-models/fred-the-cat-d27ca4d3fdd44fc49b5bb3efd19c9a06) | ใช้โมเดลและ texture ของ Fred ในฉาก |
| แมวอเมริกันช็อตแฮร์ (`cat_american`) | [Cat in Motion 3d model free — iRahulRajput](https://sketchfab.com/3d-models/cat-in-motion-3d-model-free-baa1120483c844e6bce9744f3f868c63) | ใช้เป็นโมเดลท่านิ่งในฉาก |


Texture สีที่โปรแกรมใช้มาจากชุดโมเดลที่เตรียมไว้ บางส่วนใช้สีวัสดุแทนภาพ ส่วนภาพแรงบันดาลใจเพิ่มเติมยังไม่ได้ระบุแหล่งแยกจากรายการโมเดลข้างต้น

