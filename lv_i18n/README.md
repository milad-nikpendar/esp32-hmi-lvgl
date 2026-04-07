# 🌍 Internationalization (i18n) Support  
[![i18n](https://img.shields.io/badge/i18n-enabled-red?logo=google-translate&logoColor=white)](https://www.npmjs.com/package/i18n)
[![LVGL](https://img.shields.io/badge/LVGL-9.x-blueviolet?logo=lvgl&logoColor=white)](https://github.com/lvgl/lvgl)
[![EEZ Studio](https://img.shields.io/badge/EEZ_Studio-supported-orange?logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIwIiBoZWlnaHQ9IjEyMCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSIjZmZmIj48cmVjdCB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIGZpbGw9IiNmZjYiLz48L3N2Zz4=&logoColor=white)](https://github.com/eez-open/studio)
[![Author](https://img.shields.io/badge/Author-milad--nikpendar-blue)](https://github.com/milad-nikpendar)

This module adds full multilingual capabilities to ESP32‑HMI‑LVGL, enabling dynamic runtime language switching and seamless integration with LVGL and EEZ Studio.

---

## 🔧 Enabling i18n  
To activate the translation engine, enable the macro in `lvgl_functions.h`:

```c
#define USE_I18N
```

---

## 📁 Creating Translation Files  
Inside the `translations` directory:

1. Create one `.yml` file per language.  
   Three default files exist:  
   ```
   en.yml  
   fa.yml  
   ar.yml
   ```

2. Each file must contain translation **keys** and their **values** for that language.

3. All translation files must contain the **same set of keys**.  
   Missing keys will cause errors during the build step.

**Example (en.yml):**
```yaml
hello: "Hello"
welcome: "Welcome"
```

**Example (fa.yml):**
```yaml
hello: "سلام"
welcome: "خوش آمدید"
```

---

## 🛠️ Building Translation Output  
After editing translation files, run:

```
lv_i18n_build.bat
```

On first use, the script may require some npm packages.  
If they are missing, the command prompt will guide you through installation.

---

## 📂 Generated Files  
If the build completes successfully, two files will be generated inside the `ui` folder:

```
lv_i18n.h  
lv_i18n.c
```

These files contain the compiled translation tables used by LVGL.

---

## 🧩 Integration with EEZ Studio Output  
In `screens.c`, add:

```c
#include "lv_i18n.h"
```

This ensures all UI text is routed through the translation engine.

---

# 🚀 Using i18n in Your Application

## 1. Initialization  
The translation system is initialized automatically inside:

```c
hmi_lvgl_init();
```

Internally, it calls:

```c
translations_init();
```

---

## 2. Switching Languages

### 🔄 Rotate to the next language  
```c
translations_set_locale();
```

### 🌐 Set a specific language  
Example: switch to English:

```c
lv_i18n_set_locale("en");
```

Other examples:

```c
lv_i18n_set_locale("fa");
lv_i18n_set_locale("ar");
```

---
## ✍️ Author

**Milad Nikpendar**  
GitHub: [milad-nikpendar/esp32-hmi-lvgl](https://github.com/milad-nikpendar/esp32-hmi-lvgl)  
Email: milad82nikpendar@gmail.com  
