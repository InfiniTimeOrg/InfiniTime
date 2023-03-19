# External resources
Since InfiniTime 1.11 apps and watchfaces can benefit from the external flash memory to store images and fonts. 
This external memory is a lot bigger (4MB) than the internal memory that contains the firmware (512KB).

This page describes how the resources are integrated in InfiniTime from a developer perspective. [This page](gettingStarted/updating-software.md) explains how to install and update the external resources using companion apps.

## Resources generation

Resources are generated at build time via the [CMake target `Generate  Resources`](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/src/resources/CMakeLists.txt#L19). 
It runs 3 Python scripts that respectively convert the fonts to binary format, convert the images to binary format and package everything in a .zip file.

The resulting file `infinitime-resources-x.y.z.zip` contains the images and fonts converted in binary `.bin` files and a JSON file `resources.json`. 

Companion apps use this file to upload the files to the watch. 

```
{
    "resources": [
        {
            "filename": "lv_font_dots_40.bin",
            "path": "/fonts/lv_font_dots_40.bin"
        }
    ],
    "obsolete_files": [
        {
            "path": "/example-of-obsolete-file.bin",
            "since": "1.11.0"
        }
    ]
}
```

The resource JSON file describes an array of resources and an array of obsolete files : 

- `resources` : a resource is a file that must be flashed to the watch
  - `filename`: name of the resources in the zip file.
  - `path` : file path and name where the file must be flashed in the watch FS.

- `obsolete_files` : files that are not needed anymore in the memory of the watch that can be deleted during the update procedure.
  - `path` : path of the file in the watch FS
  - `since` : version of InfiniTime that made this file obsolete.

## Resources update procedure

The update procedure is based on the [BLE FS API](BLEFS.md). The companion app simply write the binary files to the watch FS using information from the file `resources.json`.

## Working with external resources in the code

Load a picture from the external resources:

```
lv_obj_t* logo = lv_img_create(lv_scr_act(), nullptr);
lv_img_set_src(logo, "F:/images/logo.bin");
```

Load a font from the external resources: you first need to check that the file actually exists. LVGL will crash when trying to open a font that doesn't exist.

```
lv_font_t* font_teko = nullptr;
if (filesystem.FileOpen(&f, "/fonts/font.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_teko = lv_font_load("F:/fonts/font.bin");
}

if(font != nullptr) {
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
}

```

